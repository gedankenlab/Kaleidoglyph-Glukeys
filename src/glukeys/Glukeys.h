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

  void preKeyswitchScan();

  // Set the length of time from the last time a glukey entered the `pending` state before
  // all `sticky` glukeys will release, and `pending` glukeys will be cleared.
  void setTimeout(uint16_t ttl) {
    temp_ttl_ = ttl;
  }

  void setAutoModifiers(bool on = true) {
    auto_modifier_glukeys_ = on;
  }
  void setAutoLayers(bool on = true) {
    auto_layer_glukeys_ = on;
  }

 private:
  // An array of Glukey objects
  const Key* const glukeys_;
  const byte       glukey_count_;

  // A reference to the keymap for lookups
  Controller& controller_;

  // State variables -- one `temp` bit and one `sticky` bit for each valid `KeyAddr`
  byte temp_bits_[state_byte_count];
  byte glue_bits_[state_byte_count];

  // How many `temp_bits_` bits are set?
  byte temp_key_count_{0};

  // The last time a glukey's temp bit was set
  uint16_t temp_start_time_{0};
  // Reset glukeys temp bits after this much time (ms) 0 == never time out
  uint16_t temp_ttl_{2000};

  // Signal that `sticky` glukeys should be released
  KeyAddr release_trigger_{cKeyAddr::invalid};

  // The address of the current layer-shift glukey, if any
  KeyAddr layer_shift_addr_{cKeyAddr::invalid};

#ifdef KALEIDOGLYPH_GLUKEYS_WITH_META
  // Address of the active `meta_glukey`, if any
  KeyAddr meta_glukey_addr_{cKeyAddr::invalid};
#endif

  bool auto_modifier_glukeys_{true};
  bool auto_layer_glukeys_{false};

  const Key lookupGlukey(const Key key) const;

  void releaseGlukeys(bool release_locked_keys = false);

#ifdef KALEIDOGLYPH_GLUKEYS_WITH_META
  void setMetaGlukey(KeyAddr k);
  void clearMetaGlukey();
#endif

  bool isTemp(KeyAddr k) const {
    byte r = k.addr() / 8;
    byte c = k.addr() % 8;
    return bitRead(temp_bits_[r], c);
  }
  void setTemp(KeyAddr k) {
    byte r = k.addr() / 8;
    byte c = k.addr() % 8;
    if (! isTemp(k)) {
      bitSet(temp_bits_[r], c);
      ++temp_key_count_;
    }
    temp_start_time_ = uint16_t(controller_.scanStartTime());
  }
  void clearTemp(KeyAddr k) {
    if (isTemp(k)) {
      byte r = k.addr() / 8;
      byte c = k.addr() % 8;
      bitClear(temp_bits_[r], c);
      --temp_key_count_;
    }
  }

  bool isGlue(KeyAddr k) const {
    byte r = k.addr() / 8;
    byte c = k.addr() % 8;
    return bitRead(glue_bits_[r], c);
  }
  void setGlue(KeyAddr k) {
    byte r = k.addr() / 8;
    byte c = k.addr() % 8;
    bitSet(glue_bits_[r], c);
  }
  void clearGlue(KeyAddr k) {
    byte r = k.addr() / 8;
    byte c = k.addr() % 8;
    bitClear(glue_bits_[r], c);
  }

};

bool isTriggerCandidate(const Key key);

} // namespace qukeys {
} // namespace kaleidoglyph {
