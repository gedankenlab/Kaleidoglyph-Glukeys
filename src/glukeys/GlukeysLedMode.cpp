// -*- c++ -*-

#include <Arduino.h>

#include "glukeys/GlukeysLedMode.h"

namespace kaleidoglyph {
namespace glukeys {

static constexpr Color sticky_color{100, 200, 50};
static constexpr Color locked_color{200, 50, 100};

bool LedMode::setForegroundColor(KeyAddr k) {
  if (glukeys_plugin_.isSticky(k)) {
    kaleidoglyph::LedMode::manager().setKeyColor(k, sticky_color);
    return true;
  }
  if (glukeys_plugin_.isLocked(k)) {
    kaleidoglyph::LedMode::manager().setKeyColor(k, locked_color);
    return true;
  }
  return false;
}

} // namespace glukeys {
} // namespace kaleidoglyph {
