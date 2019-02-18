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

  if (event.state.toggledOn()) {
    // If the key is sticky (i.e. locked), clear the sticky bit
    if (isSticky(event.addr)) {
      clearSticky(event.addr);
      return EventHandlerResult::abort;
    }
    const Key key = lookupGlukey(event.key);
    // If it's not a GlukeysKey, ignore this event and proceed
    if (key == cKey::clear) {
      return EventHandlerResult::proceed;
    }
    // If it's a GlukeysKey, but its index value is out of bounds, abort
    if (key == cKey::blank) {
      return EventHandlerResult::abort;
    }
    // Change the `event.key` value to the one looked up in the `glukeys_[]` array of
    // `Key` objects (and let Controller restart the onKeyEvent() processing
    event.key = key;
    setSticky(event.addr);
    return EventHandlerResult::proceed;
  }

  if (event.state.toggledOff()) {
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
