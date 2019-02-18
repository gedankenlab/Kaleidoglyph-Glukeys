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
      // It's not a glukey; all glukeys should be marked for release at the end of this
      // cycle (not just after the report is sent)
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

} // namespace glukeys {
} // namespace kaleidoglyph {
