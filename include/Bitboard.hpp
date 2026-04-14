#ifndef BITBOARD_HPP
#define BITBOARD_HPP

#include <cstdint>
#include <string>

using Bitboard = uint64_t;

enum class Square : int {
  A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8,
  NONE = 64
};

enum class Color : int {
  WHITE = 0, BLACK = 1
};

enum class Piece : int {
  PAWN = 0,
  KNIGHT = 1,
  BISHOP = 2,
  ROOK = 3,
  QUEEN = 4,
  KING = 5,
  NONE = 6
};



constexpr int NUM_SQUARES = 64;
constexpr int NUM_PIECES = 6;
constexpr int NUM_COLORS = 2;

constexpr int squareToInt(Square square) {
  return static_cast<int>(square);
}

constexpr int colorToInt(Color color) {
  return static_cast<int>(color);
}

constexpr int pieceToInt(Piece piece) {
  return static_cast<int>(piece);
}

constexpr Square intToSquare(int i) {
  return static_cast<Square>(i);
}

constexpr int fileOf(Square square) {
  return squareToInt(square) & 7;
}

constexpr int rankOf(Square square) {
  return squareToInt(square) >> 3;
}

constexpr Square makeSquare(int file, int rank) {
  return intToSquare(rank * 8 + file);
}




constexpr Bitboard squareBitboard(Square square) {
  return 1ULL << squareToInt(square);
}

constexpr bool getBit(Bitboard bitboard, Square square) {
  return (bitboard & squareBitboard(square)) != 0;
}

inline void setBit(Bitboard& bitboard, Square square) {
  bitboard |= squareBitboard(square);
}

inline void clearBit(Bitboard& bitboard, Square square) {
  bitboard &= ~squareBitboard(square);
}

inline void toggleBit(Bitboard& bitboard, Square square) {
  bitboard ^= squareBitboard(square); 
}




inline int countBits(Bitboard bitboard) {
  return __builtin_popcountll(bitboard);
}

inline int lowestBit(Bitboard bitboard) {
    return __builtin_ctzll(bitboard);
}

inline int popLowestBit(Bitboard& bitboard) {
    int idx = lowestBit(bitboard);
    bitboard &= bitboard - 1;
    return idx;
}




constexpr Bitboard RANK_1 = 0x00000000000000FFULL;
constexpr Bitboard RANK_2 = 0x000000000000FF00ULL;
constexpr Bitboard RANK_3 = 0x0000000000FF0000ULL;
constexpr Bitboard RANK_4 = 0x00000000FF000000ULL;
constexpr Bitboard RANK_5 = 0x000000FF00000000ULL;
constexpr Bitboard RANK_6 = 0x0000FF0000000000ULL;
constexpr Bitboard RANK_7 = 0x00FF000000000000ULL;
constexpr Bitboard RANK_8 = 0xFF00000000000000ULL;
 
constexpr Bitboard FILE_A = 0x0101010101010101ULL;
constexpr Bitboard FILE_B = 0x0202020202020202ULL;
constexpr Bitboard FILE_C = 0x0404040404040404ULL;
constexpr Bitboard FILE_D = 0x0808080808080808ULL;
constexpr Bitboard FILE_E = 0x1010101010101010ULL;
constexpr Bitboard FILE_F = 0x2020202020202020ULL;
constexpr Bitboard FILE_G = 0x4040404040404040ULL;
constexpr Bitboard FILE_H = 0x8080808080808080ULL;




constexpr Bitboard EMPTY_BITBOARD = 0ULL;
constexpr Bitboard FULL_BITBOARD = ~EMPTY_BITBOARD;

std::string squareToString(Square square);
Square stringToSquare(const std::string& str);

#endif