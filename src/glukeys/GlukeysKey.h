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

  static bool verify(Key key) {
    return ((uint16_t(key) >> index_bits) == type_id);
  }
};

namespace cGlukeysKey {
constexpr GlukeysKey meta_glukey { 0xFF };
constexpr GlukeysKey esc_glukey  { 0xFE };
}

namespace cGlukey {
constexpr Key meta = Key( cGlukeysKey::meta_glukey );
constexpr Key esc  = Key( cGlukeysKey::esc_glukey  );
}

}
} // namespace kaleidoglyph {
