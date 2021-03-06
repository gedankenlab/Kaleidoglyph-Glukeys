// -*- c++ -*-

// TODO: decide the file names
#include "glukeys/Glukeys.h"

#include <Arduino.h>

#include <kaleidoglyph/Controller.h>
#include <kaleidoglyph/Key.h>
#include <kaleidoglyph/KeyAddr.h>
#include <kaleidoglyph/KeyArray.h>
#include <kaleidoglyph/KeyEvent.h>
#include <kaleidoglyph/KeyState.h>
#include <kaleidoglyph/Plugin.h>
#include <kaleidoglyph/cKey.h>


namespace kaleidoglyph {
namespace glukeys {

// Event handler
EventHandlerResult Plugin::onKeyEvent(KeyEvent& event) {

  // Ignore all `injected` events
  if (event.state.isInjected()) {
    return EventHandlerResult::proceed;
  }

  if (! plugin_active_) {
    if (isGlukeysKey(event.key)) {
      event.key = lookupGlukey(event.key);
    }
    return EventHandlerResult::proceed;
  }

  if (event.state.toggledOn()) {

    // This key can't be `pending` if it's toggling on (unless another plugin is
    // injecting this event), so if its `temp` bit is set, that means it's
    // already in the `sticky` state, so this is the second press of the key,
    // and we should therefore lock it. And if it is an virtual key event that
    // isn't marked `injected`, we end up putting the key in the `clear` state,
    // which is the best choice.
    if (isTemp(event.addr)) {
      // `sticky` => `locked`
      clearTemp(event.addr);
      // If this is a layer-shift key, unset `layer_shift_addr_`, which is only needed
      // when a `sticky` key is being released to prevent it from interfering with
      // subsequent keys.
      if (event.addr == layer_shift_addr_) {
        layer_shift_addr_ = cKeyAddr::invalid;
      }
      return EventHandlerResult::abort;
    }

    // We have already eliminated the possibility of this key being in either the
    // `pending` or `sticky` state, that means if its `sticky` bit is set, the key is
    // `locked`. This is the third press, so we should clear it (and we don't need to
    // clear `temp`, because we know it's already clear).
    if (isGlue(event.addr)) {
      // `locked` => `clear`
      clearGlue(event.addr);
      return EventHandlerResult::abort;
    }

    // If this is the escape-glukey
    if (event.key == cGlukey::cancel) {
#ifdef KALEIDOGLYPH_GLUKEYS_WITH_META
      // If the meta-glukey was active, clear it:
      if (meta_glukey_addr_.isValid()) {
        clearMetaGlukey();
        //controller_[meta_glukey_addr_] = cKey::clear;
        //meta_glukey_addr_ = cKeyAddr::invalid;
      }
#endif
      // Release all `sticky` & `locked` glukeys, and clear `pending` ones
      releaseGlukeys(true);
      return EventHandlerResult::abort;
    }

#ifdef KALEIDOGLYPH_GLUKEYS_WITH_META
    // If this is the meta-glukey
    if (event.key == cGlukey::meta) {
      if (meta_glukey_addr_.isValid()) {
        clearMetaGlukey();
      }
      setMetaGlukey(event.addr);
      setTemp(event.addr);
      return EventHandlerResult::proceed;
    }

    // If the meta-glukey is active, it gets released first, regardless of what key was
    // pressed. Next, the current key becomes a glukey.
    if (meta_glukey_addr_.isValid()) {
      if (isTemp(meta_glukey_addr_)) {
        if (isGlue(meta_glukey_addr_)) {
          // If the meta-glukey is `sticky`, release it
          clearMetaGlukey();
        } else {
          // It is being used chorded, so `pending` => `clear`
          clearTemp(meta_glukey_addr_);
        }
      }
      if (isGlukeysKey(event.key)) {
        event.key = lookupGlukey(event.key);
      }
      // `clear` => `locked`
      setGlue(event.addr);
      return EventHandlerResult::proceed;
    }
#endif

    // If there's already a release trigger set, that means previously-set `sticky`
    // glukeys need to be released before we proceed, or they would continue to be active
    // past the key that should have released them.
    if (release_trigger_.isValid()) {
      releaseGlukeys();
    }

    // Determine if the pressed key is a glukey
    const Key glukey = lookupGlukey(event.key);

    // If it's not a GlukeysKey...
    if (glukey == cKey::clear) {
      // This `event.key` is not a glukey. Check to see if this key should be set as the
      // trigger for release of `sticky` glukeys (and clearing of `pending` ones). Certain
      // types of keys (modifiers & layer changes) shouldn't trigger release, and we
      // should only set the trigger if there are any glukeys in the `pending` or `sticky`
      // states.
      if ((temp_key_count_ != 0) && isTriggerCandidate(event.key)) {
        release_trigger_ = event.addr;
        // Also, release any `sticky` layer-shift glukeys. The layer shift has already
        // been applied to this trigger key (`event.key` was looked up from the shifted-to
        // layer), and we don't want the layer shift to persist beyond that.
        if (layer_shift_addr_ != cKeyAddr::invalid) {
          KeyEvent event{layer_shift_addr_, cKeyState::injected_release};
          controller_.handleKeyEvent(event);
          layer_shift_addr_ = cKeyAddr::invalid;
        }
      }
      return EventHandlerResult::proceed;
    }
    // If it's a GlukeysKey, but its index value is out of bounds, abort
    if (glukey == cKey::blank) {
      return EventHandlerResult::abort;
    }
    // Change the `event.key` value to the one looked up in the `glukeys_[]` array of
    // `Key` objects (and let Controller restart the onKeyEvent() processing
    event.key = glukey;
    // `clear` => `pending`
    setTemp(event.addr);
    // Clear the release trigger whenever a glukey is pressed to prevent a bug caused by
    // rollover from the previous (unreleased) trigger key when a new glukey is
    // pressed. If we don't do this, the previous trigger key's release will also release
    // the newly-pressed glukey before its trigger is pressed.
    release_trigger_ = cKeyAddr::invalid;
    return EventHandlerResult::proceed;
  }

  if (event.state.toggledOff()) {
    // If the key is `pending`, make it `sticky`
    if (isTemp(event.addr)) {
      // `pending` => `sticky`
      setGlue(event.addr);
      // If this is a layer-shift key, record its address for later release:
      if (isLayerShiftKey(event.key)) {
        layer_shift_addr_ = event.addr;
      }
    }
    // If the key is either `sticky` or `locked`, stop the release event
    if (isGlue(event.addr)) {
      return EventHandlerResult::abort;
    }
    // If the trigger key was released, also release any `sticky` glukeys
    if (event.addr == release_trigger_) {
      releaseGlukeys();
    }
#ifdef KALEIDOGLYPH_GLUKEYS_WITH_META
    // If the released key was a `clear` meta-glukey, it needs to be cleared in a
    // different way. Maybe it makes sense to move this to a pre-report hook?
    if (event.addr == meta_glukey_addr_) {
      clearMetaGlukey();
      return EventHandlerResult::abort;
    }
#endif
  }

  return EventHandlerResult::proceed;
}


// Time out glukeys in the `pending` & `sticky` states after the set timeout value
// (`temp_ttl_`). The time for a `temp` state glukey gets extended whenever another `temp`
// glukey is pressed.
void Plugin::preKeyswitchScan() {
  // If there are no `pending` or `sticky` keys, or if glukeys is configured to never time
  // out `temp` glukeys, abort:
  if (temp_key_count_ == 0 || temp_ttl_ == 0) return;

  // Compare the difference between the current time and the last time a `temp` bit was
  // set to the ttl. If we mixed different integer types, this would correct for overflow,
  // so do it that way to stay consistent with other code:
  uint16_t current_time = Controller::scanStartTime();
  uint16_t elapsed_time = current_time - temp_start_time_;
  if (elapsed_time > temp_ttl_) {
    releaseGlukeys();
  }
}


// Check to see if the `Key` is a Glukeys key and if so, return the corresponding
// (looked-up) `Key` value, or `cKey::clear` if there is none.
inline
const Key Plugin::lookupGlukey(const Key key) const {
  Key result_key{cKey::clear};

  if (isGlukeysKey(key)) {
    GlukeysKey glukey{key};
    result_key = getKey(glukey);
    if (result_key == cKey::clear) {
      byte index = glukey.data();
      if (index < glukey_count_) {
        return getProgmemKey(glukeys_[index]);
      } else {
        result_key = cKey::blank;
      }
    }
  }

  if (auto_modifier_glukeys_ && isModifierKey(key)) {
    result_key = key;
  }

  if (auto_layer_glukeys_ && isLayerShiftKey(key)) {
    result_key = key;
  }

  return result_key;
}


// Clear all `pending` keys and release all `sticky` keys. If `release_locked_keys` is
// `true`, also release `locked` glukeys. This can result in sending a release event for a
// `locked` glukey that is still being held, but that seems to be a necessary evil that
// I'm willing to live with.
void Plugin::releaseGlukeys(bool release_locked_keys) {
  for (byte r = 0; r < state_byte_count; ++r) {
    // We expect that most keys won't have any glukey bits set, so if these eight keys are
    // all `clear`, skip to the next batch:
    if ((temp_bits_[r] | glue_bits_[r]) == 0) continue;

    // Bitfield of keys which will be released. Start with all `sticky` & `locked` keys:
    byte release_bits = glue_bits_[r];

    if (release_locked_keys) {
      // Since both `sticky` & `locked` glukeys will be released, clear all glue bits:
      glue_bits_[r] = 0;
    } else {
      // Remove the `locked` glukeys from the set that will be released, then clear only
      // the glue bits of the `sticky` glukeys:
      release_bits &= temp_bits_[r];
      glue_bits_[r] &= ~temp_bits_[r];
    }

    // Clear all temp bits. All `pending` keys become `clear`:
    temp_bits_[r] = 0;

    // For each key that needs to be released, send the event:
    for (byte c = 0; c < 8; ++c) {
      if (bitRead(release_bits, c)) {
        KeyAddr k{byte(r * 8 + c)};
        KeyEvent event{k, cKeyState::injected_release};
        controller_.handleKeyEvent(event);
        //controller_[k] = cKey::clear;
      }
    }
  }

  // There are no `sticky` or `pending` glukeys now, so reset the count:
  temp_key_count_ = 0;

  // Clear the release trigger:
  release_trigger_ = cKeyAddr::invalid;
}


#ifdef KALEIDOGLYPH_GLUKEYS_WITH_META
// Set meta-glukey
void Plugin::setMetaGlukey(KeyAddr k) {
  assert(k.isValid());

  controller_[k] = cGlukey::meta;
  meta_glukey_addr_ = k;
  setTemp(k);
}

// Clear meta-glukey
void Plugin::clearMetaGlukey() {
  assert(meta_glukey_addr_.isValid());

  controller_[meta_glukey_addr_] = cKey::clear;
  clearTemp(meta_glukey_addr_);
  clearGlue(meta_glukey_addr_);
  meta_glukey_addr_ = cKeyAddr::invalid;
}
#endif


// Test for types of keys that are eligible to trigger release of `sticky`
// glukeys. Returns `true` if `key` will trigger release of `sticky` glukeys.
bool isTriggerCandidate(const Key key) {
  // Modifiers never trigger release of `sticky` glukeys
  if (KeyboardKey::verifyType(key)) {
    return ! KeyboardKey(key).isModifier();
  }
  // Layer change keys of all kinds also don't release `sticky` glukeys
  if (LayerKey::verifyType(key)) {
    return false;
  }
  return true;
}


} // namespace glukeys {
} // namespace kaleidoglyph {
