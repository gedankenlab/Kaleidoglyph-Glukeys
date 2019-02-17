// -*- c++ -*-

#include <Arduino.h>

#include KALEIDOGLYPH_HARDWARE_H
#include KALEIDOGLYPH_KEYADDR_H
#include <kaleidoglyph/Key.h>
#include <kaleidoglyph/Plugin.h>
#include <kaleidoglyph/Keymap.h>
#include <kaleidoglyph/Controller.h>
#include <kaleidoglyph/cKey.h>
#include <kaleidoglyph/EventHandlerResult.h>
#include <kaleidoglyph/hid/Report.h>

#include "glukeys/GlukeysKey.h"

namespace kaleidoglyph {
namespace glukeys {

// The number of bytes needed to store a bitfield with one bit per KeyAddr
constexpr byte state_byte_count = bitfieldSize(total_keys);

class Plugin : public kaleidoglyph::Plugin {

 public:
  Plugin(const Key* const glukeys, const byte glukey_count)
      : glukeys_(glukeys), glukey_count_(glukey_count) {}

  EventHandlerResult onKeyEvent(KeyEvent& event);

  void postKeyboardReport(KeyEvent event);

 private:
  // An array of Glukey objects
  const Key* const glukeys_;
  const byte       glukey_count_;

  // State variables -- one `temp` bit and one `sticky` bit for each valid `KeyAddr`
  byte sticky_state_[state_byte_count];

  const Key lookupGlukey(Key key) const;

  bool isSticky(KeyAddr k) const {
    byte r = k.addr() / 8;
    byte c = k.addr() % 8;
    return bitRead(sticky_state_[r], c);
  }
  void setSticky(KeyAddr k) {
    byte r = k.addr() / 8;
    byte c = k.addr() % 8;
    bitSet(sticky_state_[r], c);
  }
  void clearSticky(KeyAddr k) {
    byte r = k.addr() / 8;
    byte c = k.addr() % 8;
    bitSet(sticky_state_[r], c);
  }

};

} // namespace qukeys {
} // namespace kaleidoglyph {
