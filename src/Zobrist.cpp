#include "Zobrist.hpp"
#include <random>

namespace Zobrist {
  std::array<std::array<std::array<uint64_t, NUM_SQUARES>, NUM_PIECES>, NUM_COLORS> pieceKeys;
  std::array<uint64_t, 16> castlingKeys;
  std::array<uint64_t, 8> enPassantKeys;
  uint64_t sideKey;

  void init() {
    std::mt19937_64 rng(1234567890);

    for (int color = 0; color < NUM_COLORS; color++) {
      for (int piece = 0; piece < NUM_PIECES; piece++) {
        for (int square = 0; square < NUM_SQUARES; square++) {
          pieceKeys[color][piece][square] = rng();
        }
      }
    }

    for (int i = 0; i < 16; i++) {
      castlingKeys[i] = rng();
    }

    for (int file = 0; file < 8; file++) {
      enPassantKeys[file] = rng();
    }

    sideKey = rng();
  }
}