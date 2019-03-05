// -*- c++ -*-

#pragma once

#include <Arduino.h>

#include <kaleidoglyph/KeyAddr.h>
#include <kaleidoglyph/KeyAddr.h>
#include <kaleidoglyph/led/LedForegroundMode.h>
#include <kaleidoglyph/led/LedController.h>

#include "glukeys/Glukeys.h"

namespace kaleidoglyph {
namespace glukeys {

class LedMode : public LedForegroundMode {

 public:
  LedMode(glukeys::Plugin& glukeys_plugin)
      : glukeys_plugin_(glukeys_plugin) {}

  bool setForegroundColor(KeyAddr k, LedController& led_controller);

 private:
  const glukeys::Plugin& glukeys_plugin_;

};

} // namespace glukeys {
} // namespace kaleidoglyph {
