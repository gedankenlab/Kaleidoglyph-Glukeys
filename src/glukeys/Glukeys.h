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

// Glukey structure. I don't think we even need to bother with this
struct Glukey {
  Key key;

  Glukey(Key _key) : key(_key) {}
};


class Plugin : public kaleidoglyph::Plugin {

 public:
  Plugin(const Key* const glukeys, const byte glukey_count)
      : glukeys_(glukeys), glukey_count_(glukey_count) {}

  EventHandlerResult onKeyEvent(KeyEvent& event);

  bool preKeyboardReport(hid::keyboard::Report& keyboard_report);

  void postKeyboardReport(KeyEvent event);

 private:
  // An array of Glukey objects
  const Glukey* const glukeys_;
  const byte          glukey_count_;

  byte temp_state_[bitfieldSize(total_keys)];
  byte sticky_state_[bitfieldSize(total_keys)];

  const Glukey* lookupGlukey(Key key);

};

} // namespace qukeys {
} // namespace kaleidoglyph {
