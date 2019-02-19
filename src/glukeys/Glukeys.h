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
  Plugin(const Key* const glukeys, const byte glukey_count, Controller& controller)
      : glukeys_(glukeys), glukey_count_(glukey_count), controller_(controller) {}

  EventHandlerResult onKeyEvent(KeyEvent& event);

 private:
  // An array of Glukey objects
  const Key* const glukeys_;
  const byte       glukey_count_;

  // A reference to the keymap for lookups
  Controller& controller_;

  // State variables -- one `temp` bit and one `sticky` bit for each valid `KeyAddr`
  byte temp_state_[state_byte_count];
  byte sticky_state_[state_byte_count];

  // How many `temp_state_` bits are set?
  byte temp_state_count_{0};

  // Signal that `sticky` glukeys should be released
  KeyAddr release_trigger_{cKeyAddr::invalid};

  const Key lookupGlukey(const Key key) const;

  void releaseAllTempKeys();

  bool isTemp(KeyAddr k) const {
    byte r = k.addr() / 8;
    byte c = k.addr() % 8;
    return bitRead(temp_state_[r], c);
  }
  void setTemp(KeyAddr k) {
    byte r = k.addr() / 8;
    byte c = k.addr() % 8;
    bitSet(temp_state_[r], c);
    ++temp_state_count_;
  }
  void clearTemp(KeyAddr k) {
    byte r = k.addr() / 8;
    byte c = k.addr() % 8;
    bitClear(temp_state_[r], c);
    --temp_state_count_;
  }

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
    bitClear(sticky_state_[r], c);
  }

};

bool isTriggerCandidate(const Key key);

} // namespace qukeys {
} // namespace kaleidoglyph {
