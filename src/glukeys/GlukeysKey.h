// -*- c++ -*-

#pragma once

#include <Arduino.h>

#include "kaleidoglyph/Key.h"
#include <assert.h>


#if defined(GLUKEYS_CONSTANTS_H)
#include GLUKEYS_CONSTANTS_H
#else
namespace kaleidoglyph {
namespace glukeys {

constexpr byte type_id_bits {8};
constexpr byte index_bits {8};

constexpr byte type_id { 0b01000010 };

// Special glukeys have the high bit set. The next bit tells which type of special it is
// (modifier or layer change)
constexpr byte category_mask        { 0b11000000 };
constexpr byte layer_category_id    { 0b10000000 };
constexpr byte layer_mask           { 0b00011111 }; // max < 32
// Maybe I can use the third highest bit to indicate EEPROM layers?
constexpr byte modifier_category_id { 0b11000000 };
constexpr byte modifier_mask        { 0b00000111 }; // max <  8
// I've got six bits left, so I could use the bottom three for the first modifier, and the
// top three for another, but it's probably better to just use the bottom three.
constexpr byte glukey_category_id   { 0b00000000 };
constexpr byte glukey_mask          { 0b01111111 };

constexpr byte invalid_glukey_index { 0b11100000 };

}
}
#endif

namespace kaleidoglyph {
namespace glukeys {


class GlukeysKey {

 private:
  uint16_t index_ : index_bits, type_id_ : type_id_bits;

 public:
  byte index() const { return index_; }

  GlukeysKey() : index_    (0),
                 type_id_ (type_id) {}

  constexpr explicit
  GlukeysKey(byte index) : index_   (index),
                           type_id_ (type_id) {}

  explicit
  GlukeysKey(Key key) : index_   (uint16_t(key)              ),
                        type_id_ (uint16_t(key) >> index_bits)  {
    assert(type_id_ == type_id);
  }

  constexpr
  operator Key() const {
    return Key( index_                 |
                type_id_ << index_bits   );
  }

  static constexpr
  bool verify(Key key) {
    return ((uint16_t(key) >> index_bits) == type_id);
  }

  constexpr
  bool isModifierKey() const {
    return ( (index_ & category_mask) == modifier_category_id );
  }

  constexpr
  bool isLayerKey() const {
    return ( (index_ & category_mask) == layer_category_id );
  }

  constexpr
  Key modifierKey() const {
    return ( bool(index_ & modifier_category_id)
             ? cKey::blank
             : KeyboardKey::modifierKey(index_ & modifier_mask) );
  }

  constexpr
  Key layerShiftKey() const {
    return ( bool(index_ & layer_category_id)
             ? cKey::blank
             : LayerKey::layerShiftKey(index_ & layer_mask) );
  }

  // Return a `Key` determined by the index bits of the GlukeysKey. Possible return values
  // are: a keyboard modifier key, a layer-shift key, an indicator that this key can be
  // used to look up a `Key` value in the `glukeys_[]` array (`cKey::clear`), and an
  // indicator that this GlukeysKey is invalid (`cKey::blank`).
  Key getKey() const {
    byte category_id = index_ & category_mask;
    switch (category_id) {
      case modifier_category_id :
        return KeyboardKey::modifierKey(index_ & modifier_mask);
      case layer_category_id :
        return LayerKey::layerShiftKey(index_ & layer_mask);
      case glukey_category_id :
        return cKey::clear;
      default:
        return cKey::blank;
    }
  }

};

constexpr
bool isGlukeysKey(Key key) {
  return { GlukeysKey::verify(key) };
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

}
} // namespace kaleidoglyph {
