#include "Attacks.hpp"

namespace Attacks {
  std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLORS> pawnAttacks;
  std::array<Bitboard, NUM_SQUARES> knightAttacks;
  std::array<Bitboard, NUM_SQUARES> kingAttacks;




  static void initPawnAttacks() {
    for (int square = 0; square < NUM_SQUARES; square++) {
      Bitboard currentSquareBitboard = squareBitboard(intToSquare(square));

      Bitboard whiteAttack = EMPTY_BITBOARD;
      whiteAttack |= (currentSquareBitboard << 7) & ~FILE_H;
      whiteAttack |= (currentSquareBitboard << 9) & ~FILE_A;
      pawnAttacks[colorToInt(Color::WHITE)][square] = whiteAttack;

      Bitboard blackAttack = EMPTY_BITBOARD;
      blackAttack |= (currentSquareBitboard >> 7) & ~FILE_A;
      blackAttack |= (currentSquareBitboard >> 9) & ~FILE_H;
      pawnAttacks[colorToInt(Color::BLACK)][square] = blackAttack;
    }
  }

  static void initKnightAttacks() {
    for (int square = 0; square < NUM_SQUARES; square++) {
      Bitboard currentSquareBitboard = squareBitboard(intToSquare(square));

      Bitboard attacks = EMPTY_BITBOARD;
      attacks |= (currentSquareBitboard << 17) & ~FILE_A;
      attacks |= (currentSquareBitboard << 15) & ~FILE_H;
      attacks |= (currentSquareBitboard << 10) & ~(FILE_A | FILE_B);
      attacks |= (currentSquareBitboard << 6) & ~(FILE_G | FILE_H);
      attacks |= (currentSquareBitboard >> 17) & ~FILE_H;
      attacks |= (currentSquareBitboard >> 15) & ~FILE_A;
      attacks |= (currentSquareBitboard >> 10) & ~(FILE_G | FILE_H);
      attacks |= (currentSquareBitboard >> 6)  & ~(FILE_A | FILE_B);
      knightAttacks[square] = attacks;
    }
  }

  static void initKingAttacks() {
    for (int square = 0; square < NUM_SQUARES; square++) {
      Bitboard currentSquareBitboard = squareBitboard(intToSquare(square));

      Bitboard attacks = EMPTY_BITBOARD;
      attacks |= (currentSquareBitboard << 9) & ~FILE_A;
      attacks |= (currentSquareBitboard << 8);
      attacks |= (currentSquareBitboard << 7) & ~FILE_H;
      attacks |= (currentSquareBitboard << 1) & ~FILE_A;
      attacks |= (currentSquareBitboard >> 9) & ~FILE_H;
      attacks |= (currentSquareBitboard >> 8);
      attacks |= (currentSquareBitboard >> 7) & ~FILE_A;
      attacks |= (currentSquareBitboard >> 1) & ~FILE_H;
      kingAttacks[square] = attacks;
    }
  }




  Bitboard bishopAttacks(Bitboard bitboard, Square square) {
    Bitboard attacks = EMPTY_BITBOARD;
    int file = fileOf(square);
    int rank = rankOf(square);

    for (int currentFile = file + 1, currentRank = rank + 1; currentFile <= MAX_FILE && currentRank <= MAX_RANK; currentFile++, currentRank++) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, currentRank));
      attacks |= currentSquareBitboard;

      if (bitboard & currentSquareBitboard) {
        break;
      }
    }

    for (int currentFile = file - 1, currentRank = rank + 1; currentFile >= MIN_FILE && currentRank <= MAX_RANK; currentFile--, currentRank++) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, currentRank));
      attacks |= currentSquareBitboard;

      if (bitboard & currentSquareBitboard) {
        break;
      }
    }

    for (int currentFile = file + 1, currentRank = rank - 1; currentFile <= MAX_FILE && currentRank >= MIN_RANK; currentFile++, currentRank--) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, currentRank));
      attacks |= currentSquareBitboard;

      if (bitboard & currentSquareBitboard) {
        break;
      }
    }

    for (int currentFile = file - 1, currentRank = rank - 1; currentFile >= MIN_FILE && currentRank >= MIN_RANK; currentFile--, currentRank--) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, currentRank));
      attacks |= currentSquareBitboard;

      if (bitboard & currentSquareBitboard) {
        break;
      }
    }

    return attacks;
  }

  Bitboard rookAttacks(Bitboard bitboard, Square square) {
    Bitboard attacks = EMPTY_BITBOARD;
    int file = fileOf(square);
    int rank = rankOf(square);

    for (int currentFile = file + 1; currentFile <= MAX_FILE; currentFile++) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, rank));
      attacks |= currentSquareBitboard;

      if (bitboard & currentSquareBitboard) {
        break;
      }
    }

    for (int currentFile = file - 1; currentFile >= MIN_FILE; currentFile--) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, rank));
      attacks |= currentSquareBitboard;

      if (bitboard & currentSquareBitboard) {
        break;
      }
    }

    for (int currentRank = rank + 1; currentRank <= MAX_RANK; currentRank++) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(file, currentRank));
      attacks |= currentSquareBitboard;

      if (bitboard & currentSquareBitboard) {
        break;
      }
    }

    for (int currentRank = rank - 1; currentRank >= MIN_RANK; currentRank--) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(file, currentRank));
      attacks |= currentSquareBitboard;

      if (bitboard & currentSquareBitboard) {
        break;
      }
    }

    return attacks;
  }




  void init() {
    initPawnAttacks();
    initKnightAttacks();
    initKingAttacks();
  }
}