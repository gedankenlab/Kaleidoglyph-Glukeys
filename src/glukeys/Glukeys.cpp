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
    if (const Key key = lookupGlukey(event.key)) {
      if (key == cKey::clear) {
        return EventHandlerResult::abort;
      }
      // Change the `event.key` value to the one looked up in the `glukeys_[]` array of `Key` objects
      event.key = key;
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
const Key* Plugin::lookupGlukey(Key key) {
  if (GlukeysKey::verify(key)) {
    byte glukey_index = GlukeysKey(key).index();
    if (glukey_index < glukey_count_) {
      return glukeys_[glukey_index];
    }
  }
  // It might be more satisfying to return `cKey::blank` (indicating lookup failure), but
  // using `clear` instead might mean we can drop some code in the event handler, because
  // the key will just be simply ignored. Note that `blank` might work just as well -- I
  // haven't thought it through.
  return cKey::clear;
}

} // namespace glukeys {
} // namespace kaleidoglyph {
