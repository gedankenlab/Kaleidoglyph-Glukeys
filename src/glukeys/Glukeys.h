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

  const Glukey* lookupGlukey(Key key);

};

} // namespace qukeys {
} // namespace kaleidoglyph {
