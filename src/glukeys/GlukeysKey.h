// -*- c++ -*-

#pragma once

#include <Arduino.h>

#include <assert.h>
#include "kaleidoglyph/Key.h"
#include "kaleidoglyph/Key/PluginKey.h"

#if defined(KALEIDOGLYPH_GLUKEYS_CONSTANTS_H)
#include KALEIDOGLYPH_GLUKEYS_CONSTANTS_H
#else
namespace kaleidoglyph {
namespace glukeys {

constexpr byte key_type_id{0b0000010};

// Special glukeys have the high bit set. The next bit tells which type of special it is
// (modifier or layer change)
constexpr byte category_mask        { 0b11'000000 };
constexpr byte layer_category_id    { 0b10'000000 };
constexpr byte layer_mask           { 0b00'011111 }; // max < 32
// Maybe I can use the third highest bit to indicate EEPROM layers?
constexpr byte modifier_category_id { 0b11'000000 };
constexpr byte modifier_mask        { 0b00'000111 }; // max <  8
// I've got six bits left, so I could use the bottom three for the first modifier, and the
// top three for another, but it's probably better to just use the bottom three.
constexpr byte glukey_category_id   { 0b0'0000000 };
constexpr byte glukey_mask          { 0b0'1111111 };

}  // namespace qukeys
}  // namespace kaleidoglyph
#endif

namespace kaleidoglyph {
namespace glukeys {

typedef PluginKey<key_type_id> GlukeysKey;

constexpr
bool isGlukeysKey(Key key) {
  return { GlukeysKey::verifyType(key) };
}

// Return a `Key` determined by the index bits of the GlukeysKey. Possible return values
// are: a keyboard modifier key, a layer-shift key, an indicator that this key can be used
// to look up a `Key` value in the `glukeys_[]` array (`cKey::clear`), and an indicator
// that this GlukeysKey is invalid (`cKey::blank`).
inline
Key getKey(GlukeysKey glukeys_key) {
  byte index = glukeys_key.data();
  byte category_id = index & category_mask;
  switch (category_id) {
    case modifier_category_id :
      return modifierKey(index & modifier_mask);
    case layer_category_id :
      return layerShiftKey(index & layer_mask);
    case glukey_category_id :
      return cKey::clear;
    default:
      return cKey::blank;
  }
}

constexpr
GlukeysKey glukeysModifierKey(byte n) {
  return ( GlukeysKey{ byte(modifier_category_id | (modifier_mask & n)) } );
}
constexpr
GlukeysKey glukeysModifierKey(KeyboardKey key) {
  return ( glukeysModifierKey(key.keycode() - KeyboardKey::mod_keycode_offset) );
}

constexpr
GlukeysKey glukeysLayerShiftKey(byte n) {
  return ( GlukeysKey{ byte(layer_category_id | (layer_mask & n)) } );
}
constexpr
GlukeysKey glukeysLayerShiftKey(LayerKey key) {
  return ( glukeysLayerShiftKey(key.index()) );
}

namespace cGlukeysKey {
constexpr GlukeysKey meta_glukey { 0xFF };
constexpr GlukeysKey esc_glukey  { 0xFE };
}

namespace cGlukey {
constexpr Key meta    = Key( cGlukeysKey::meta_glukey );
constexpr Key cancel  = Key( cGlukeysKey::esc_glukey  );
}

}  // namespace qukeys
}  // namespace kaleidoglyph
