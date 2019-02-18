// -*- c++ -*-

// TODO: decide the file names
#include "glukeys/Glukeys.h"

#include <Arduino.h>

#include <kaleidoglyph/Key.h>
#include <kaleidoglyph/KeyAddr.h>
#include <kaleidoglyph/KeyArray.h>
#include <kaleidoglyph/KeyEvent.h>
#include <kaleidoglyph/KeyState.h>
#include <kaleidoglyph/Plugin.h>
#include <kaleidoglyph/hid/Report.h>
#include <kaleidoglyph/cKey.h>


namespace kaleidoglyph {
namespace glukeys {

// Event handler
EventHandlerResult Plugin::onKeyEvent(KeyEvent& event) {

  // This might make the code more efficient, but might not be worth it:
  // KeyAddr k = event.addr;
  // byte r = k.addr() / 8;
  // byte c = k.addr() % 8;
  if (event.state.isInjected()) {
    return EventHandlerResult::proceed;
  }

  if (event.state.toggledOn()) {
    // This key can't be `pending` if it's toggling on (unless another plugin is injecting
    // this event)
    if (isTemp(event.addr)) {
      // `sticky` => `locked`
      clearTemp(event.addr);
      return EventHandlerResult::abort;
    }
    if (isSticky(event.addr)) {
      // `locked` => `clear`
      clearSticky(event.addr);
      return EventHandlerResult::abort;
    }
    // Determine if the pressed key is a glukey
    const Key glukey = lookupGlukey(event.key);
    // If it's not a GlukeysKey, ignore this event and proceed
    if (glukey == cKey::clear) {
      // It's not a glukey; all glukeys should be marked for release after the report for
      // this key is sent. There are several ways to do this, but they all have drawbacks.
      if (release_trigger_ == cKeyAddr::invalid) {
        // If no `temp_state_[]` bits are set, don't set release trigger
        if (temp_state_count_ != 0) {
          release_trigger_ = event.addr;
        }
      } else {
        releaseAllTempKeys();
        // If we clear `release_trigger_` here (as we probably should), we end up using 40
        // more bytes of PROGMEM for some reason, and the observed behaviour is the
        // same. The problem is that we end up calling `releaseAllTempKeys()` during every
        // normal keypress, which means executing a for loop to find `temp_state_[]`
        // empty, and therefore slowing down every event.
        //release_trigger_ = cKeyAddr::invalid;
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
    return EventHandlerResult::proceed;
  }

  if (event.state.toggledOff()) {
    // If the key is `pending`, make it `sticky`
    if (isTemp(event.addr)) {
      // `pending` => `sticky`
      setSticky(event.addr);
    }
    // If the key is either `sticky` or `locked`, stop the release event
    if (isSticky(event.addr)) {
      return EventHandlerResult::abort;
    }
    // If the trigger key was released, also release any `sticky` glukeys
    if (event.addr == release_trigger_) {
      releaseAllTempKeys();
      release_trigger_ = cKeyAddr::invalid;
    }
  }

  return EventHandlerResult::proceed;
}


// Released sticky keys (usually) get released here
void Plugin::postKeyboardReport(KeyEvent event) {
  // To be implemented later
}


// Check to see if the `Key` is an Glukeys key and if so, return the corresponding
// (looked-up) `Key` value, or `cKey::clear` if there is none.
inline
const Key Plugin::lookupGlukey(Key key) const {
  if (GlukeysKey::verify(key)) {
    byte glukey_index = GlukeysKey(key).index();
    if (glukey_index < glukey_count_) {
      return glukeys_[glukey_index];
    }
    // Indicator for an invalid index
    return cKey::blank;
  }
  // Indicator for a non-GlukeysKey
  return cKey::clear;
}


// Clear all `pending` keys and release all `sticky` keys
void Plugin::releaseAllTempKeys() {
  for (byte r = 0; r < state_byte_count; ++r) {
    // We expect that most keys won't have any glukey bits set
    if (temp_state_[r] == 0) continue;

    // Store a bitfield of the keys that need to have release events sent
    byte sticky_glukeys = temp_state_[r] & sticky_state_[r];

    // Unset all sticky bits of keys with the temp bit set. This means that any keys in
    // the `sticky` state will end up `clear`, not `locked`:
    sticky_state_[r] &= ~temp_state_[r];

    // Clear all temp bits. This means that all keys that were `pending` become `clear`:
    temp_state_[r] = 0;

    // Send release events for sticky keys:
    for (byte c = 0; c < 8; ++c) {
      if (bitRead(sticky_glukeys, c)) {
        // Send release event
        KeyAddr k { byte(r * 8 + c) };
        KeyEvent event {
          k,
          cKeyState::injected_release
        };
        controller_.handleKeyEvent(event);
      }
    }
  }
  temp_state_count_ = 0;
  // It would make sense to clear the release trigger key here, as well, but it's not
  // always necessary, because in the normal case, it's not possible to get a release of
  // the trigger key until after it is pressed again. See comment above where
  // releaseAllKeys() is called.
}

} // namespace glukeys {
} // namespace kaleidoglyph {
