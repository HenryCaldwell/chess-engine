#ifndef ATTACKS_HPP
#define ATTACKS_HPP

#include "Bitboard.hpp"
#include <array>

namespace Attacks {
  void init();




  extern std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLORS> pawnAttacks;
  extern std::array<Bitboard, NUM_SQUARES> knightAttacks;
  extern std::array<Bitboard, NUM_SQUARES> kingAttacks;




  Bitboard bishopAttacks(Bitboard bitboard, Square square);
  Bitboard rookAttacks(Bitboard bitboard, Square square);

  inline Bitboard queenAttacks(Bitboard bitboard, Square square) {
    return bishopAttacks(bitboard, square) | rookAttacks(bitboard, square);
  }
}

#endif