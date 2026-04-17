#ifndef ZOBRIST_HPP
#define ZOBRIST_HPP

#include "Bitboard.hpp"
#include <array>

namespace Zobrist {
  void init();




  extern std::array<std::array<std::array<uint64_t, NUM_SQUARES>, NUM_PIECES>, NUM_COLORS> pieceKeys;
  extern std::array<uint64_t, 16> castlingKeys;
  extern std::array<uint64_t, 8> enPassantKeys;
  extern uint64_t sideKey;
}

#endif