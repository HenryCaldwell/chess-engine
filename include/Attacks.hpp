#ifndef ATTACKS_HPP
#define ATTACKS_HPP

#include "Bitboard.hpp"

namespace Attacks {
  void init();




  Bitboard pawnAttacks(Color color, Square square);
  Bitboard knightAttacks(Square square);
  Bitboard kingAttacks(Square square);



  
  Bitboard bishopAttacks(Square square, Bitboard occupancy);
  Bitboard rookAttacks(Square square, Bitboard occupancy);
  Bitboard queenAttacks(Square square, Bitboard occupancy);
}

#endif