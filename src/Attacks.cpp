#include "Attacks.hpp"
#include <array>
#include <vector>
#include <immintrin.h>

namespace Attacks {
  static std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLORS> pawnAttackTable;
  static std::array<Bitboard, NUM_SQUARES> knightAttackTable;
  static std::array<Bitboard, NUM_SQUARES> kingAttackTable;

  static std::array<std::vector<Bitboard>, NUM_SQUARES> bishopAttackTable;
  static std::array<std::vector<Bitboard>, NUM_SQUARES> rookAttackTable;

  static std::array<Bitboard, NUM_SQUARES> bishopMasks;
  static std::array<Bitboard, NUM_SQUARES> rookMasks;




  static void initPawnAttacks() {
    for (int square = 0; square < NUM_SQUARES; square++) {
      Bitboard currentSquareBitboard = squareBitboard(intToSquare(square));

      Bitboard whiteAttack = EMPTY_BITBOARD;
      whiteAttack |= (currentSquareBitboard << 7) & ~FILE_H;
      whiteAttack |= (currentSquareBitboard << 9) & ~FILE_A;
      pawnAttackTable[colorToInt(Color::WHITE)][square] = whiteAttack;

      Bitboard blackAttack = EMPTY_BITBOARD;
      blackAttack |= (currentSquareBitboard >> 7) & ~FILE_A;
      blackAttack |= (currentSquareBitboard >> 9) & ~FILE_H;
      pawnAttackTable[colorToInt(Color::BLACK)][square] = blackAttack;
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
      knightAttackTable[square] = attacks;
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
      kingAttackTable[square] = attacks;
    }
  }




  static Bitboard bishopMask(Square square) {
    Bitboard mask = EMPTY_BITBOARD;

    int file = fileOf(square);
    int rank = rankOf(square);

    // Northeast
    for (int f = file + 1, r = rank + 1; f <= 6 && r <= 6; f++, r++) {
      mask |= squareBitboard(makeSquare(f, r));
    }

    // Northwest
    for (int f = file - 1, r = rank + 1; f >= 1 && r <= 6; f--, r++) {
      mask |= squareBitboard(makeSquare(f, r));
    }

    // Southeast
    for (int f = file + 1, r = rank - 1; f <= 6 && r >= 1; f++, r--) {
      mask |= squareBitboard(makeSquare(f, r));
    }

    // Southwest
    for (int f = file - 1, r = rank - 1; f >= 1 && r >= 1; f--, r--) {
      mask |= squareBitboard(makeSquare(f, r));
    }

    return mask;
  }

  static Bitboard rookMask(Square square) {
    Bitboard mask = EMPTY_BITBOARD;

    int file = fileOf(square);
    int rank = rankOf(square);

    // North
    for (int r = rank + 1; r <= 6; r++) {
      mask |= squareBitboard(makeSquare(file, r));
    }

    // South
    for (int r = rank - 1; r >= 1; r--) {
      mask |= squareBitboard(makeSquare(file, r));
    }

    // East
    for (int f = file + 1; f <= 6; f++) {
      mask |= squareBitboard(makeSquare(f, rank));
    }

    // West
    for (int f = file - 1; f >= 1; f--) {
      mask |= squareBitboard(makeSquare(f, rank));
    }

    return mask;
  }

  static void initBishopMasks() {
    for (int square = 0; square < NUM_SQUARES; square++) {
      bishopMasks[square] = bishopMask(intToSquare(square));
    }
  }

  static void initRookMasks() {
    for (int square = 0; square < NUM_SQUARES; square++) {
      rookMasks[square] = rookMask(intToSquare(square));
    }
  }




  static int bishopIndex(Square square, Bitboard occupancy) {
    int squareIndex = squareToInt(square);
    return static_cast<int>(_pext_u64(occupancy, bishopMasks[squareIndex]));
  }

  static int rookIndex(Square square, Bitboard occupancy) {
    int squareIndex = squareToInt(square);
    return static_cast<int>(_pext_u64(occupancy, rookMasks[squareIndex]));
  }

  static std::vector<Bitboard> blockerSubsets(Bitboard mask) {
    std::vector<Bitboard> subsets;
    Bitboard subset = EMPTY_BITBOARD;

    while (true) {
      subsets.push_back(subset);
      subset = (subset - mask) & mask;

      if (subset == EMPTY_BITBOARD) {
        break;
      }
    }

    return subsets;
  }

  static Bitboard generateBishopAttacks(Bitboard occupancy, Square square) {
    Bitboard attacks = EMPTY_BITBOARD;
    int file = fileOf(square);
    int rank = rankOf(square);

    for (int currentFile = file + 1, currentRank = rank + 1; currentFile <= MAX_FILE && currentRank <= MAX_RANK; currentFile++, currentRank++) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, currentRank));
      attacks |= currentSquareBitboard;

      if (occupancy & currentSquareBitboard) {
        break;
      }
    }

    for (int currentFile = file - 1, currentRank = rank + 1; currentFile >= MIN_FILE && currentRank <= MAX_RANK; currentFile--, currentRank++) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, currentRank));
      attacks |= currentSquareBitboard;

      if (occupancy & currentSquareBitboard) {
        break;
      }
    }

    for (int currentFile = file + 1, currentRank = rank - 1; currentFile <= MAX_FILE && currentRank >= MIN_RANK; currentFile++, currentRank--) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, currentRank));
      attacks |= currentSquareBitboard;

      if (occupancy & currentSquareBitboard) {
        break;
      }
    }

    for (int currentFile = file - 1, currentRank = rank - 1; currentFile >= MIN_FILE && currentRank >= MIN_RANK; currentFile--, currentRank--) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, currentRank));
      attacks |= currentSquareBitboard;

      if (occupancy & currentSquareBitboard) {
        break;
      }
    }

    return attacks;
  }

  static Bitboard generateRookAttacks(Bitboard occupancy, Square square) {
    Bitboard attacks = EMPTY_BITBOARD;
    int file = fileOf(square);
    int rank = rankOf(square);

    for (int currentFile = file + 1; currentFile <= MAX_FILE; currentFile++) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, rank));
      attacks |= currentSquareBitboard;

      if (occupancy & currentSquareBitboard) {
        break;
      }
    }

    for (int currentFile = file - 1; currentFile >= MIN_FILE; currentFile--) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(currentFile, rank));
      attacks |= currentSquareBitboard;

      if (occupancy & currentSquareBitboard) {
        break;
      }
    }

    for (int currentRank = rank + 1; currentRank <= MAX_RANK; currentRank++) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(file, currentRank));
      attacks |= currentSquareBitboard;

      if (occupancy & currentSquareBitboard) {
        break;
      }
    }

    for (int currentRank = rank - 1; currentRank >= MIN_RANK; currentRank--) {
      Bitboard currentSquareBitboard = squareBitboard(makeSquare(file, currentRank));
      attacks |= currentSquareBitboard;

      if (occupancy & currentSquareBitboard) {
        break;
      }
    }

    return attacks;
  }

  static void initBishopAttacks() {
    for (int square = 0; square < NUM_SQUARES; square++) {
      Square currentSquare = intToSquare(square);

      Bitboard mask = bishopMasks[square];
      int relevantBits = countBits(mask);

      bishopAttackTable[square].resize(1ULL << relevantBits);

      for (Bitboard blockers : blockerSubsets(mask)) {
        int index = bishopIndex(currentSquare, blockers);
        bishopAttackTable[square][index] = generateBishopAttacks(blockers, currentSquare);
      }
    }
  }

  static void initRookAttacks() {
    for (int square = 0; square < NUM_SQUARES; square++) {
      Square currentSquare = intToSquare(square);

      Bitboard mask = rookMasks[square];
      int relevantBits = countBits(mask);

      rookAttackTable[square].resize(1ULL << relevantBits);

      for (Bitboard blockers : blockerSubsets(mask)) {
        int index = rookIndex(currentSquare, blockers);
        rookAttackTable[square][index] = generateRookAttacks(blockers, currentSquare);
      }
    }
  }




  void init() {
    initPawnAttacks();
    initKnightAttacks();
    initKingAttacks();
    
    initBishopMasks();
    initRookMasks();

    initBishopAttacks();
    initRookAttacks();
  }




  Bitboard pawnAttacks(Color color, Square square) {
    return pawnAttackTable[colorToInt(color)][squareToInt(square)];
  }

  Bitboard knightAttacks(Square square) {
    return knightAttackTable[squareToInt(square)];
  }

  Bitboard kingAttacks(Square square) {
    return kingAttackTable[squareToInt(square)];
  }

  Bitboard bishopAttacks(Square square, Bitboard occupancy) {
    int squareIndex = squareToInt(square);
    int index = bishopIndex(square, occupancy);

    return bishopAttackTable[squareIndex][index];
  }

  Bitboard rookAttacks(Square square, Bitboard occupancy) {
    int squareIndex = squareToInt(square);
    int index = rookIndex(square, occupancy);

    return rookAttackTable[squareIndex][index];
  }

  Bitboard queenAttacks(Square square, Bitboard occupancy) {
    return bishopAttacks(square, occupancy) | rookAttacks(square, occupancy);
  }
}