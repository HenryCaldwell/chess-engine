#include "Attacks.hpp"
#include <array>
#include <vector>

namespace Attacks {
  static std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLORS> pawnAttackTable;
  static std::array<Bitboard, NUM_SQUARES> knightAttackTable;
  static std::array<Bitboard, NUM_SQUARES> kingAttackTable;

  static std::array<std::vector<Bitboard>, NUM_SQUARES> bishopAttackTable;
  static std::array<std::vector<Bitboard>, NUM_SQUARES> rookAttackTable;

  static std::array<Bitboard, NUM_SQUARES> bishopMasks;
  static std::array<Bitboard, NUM_SQUARES> rookMasks;

  static std::array<Bitboard, NUM_SQUARES> bishopMagics = {
    0x40040844404084ULL, 0x2004208a004208ULL, 0x10190041080202ULL, 0x108060845042010ULL,
    0x581104180800210ULL, 0x2112080446200010ULL, 0x1080820820060210ULL, 0x3c0808410220200ULL,
    0x4050404440404ULL, 0x21001420088ULL, 0x24d0080801082102ULL, 0x1020a0a020400ULL,
    0x40308200402ULL, 0x4011002100800ULL, 0x401484104104005ULL, 0x801010402020200ULL,
    0x400210c3880100ULL, 0x404022024108200ULL, 0x810018200204102ULL, 0x4002801a02003ULL,
    0x85040820080400ULL, 0x810102c808880400ULL, 0xe900410884800ULL, 0x8002020480840102ULL,
    0x220200865090201ULL, 0x2010100a02021202ULL, 0x152048408022401ULL, 0x20080002081110ULL,
    0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL, 0x1c004001012080ULL,
    0x8004200962a00220ULL, 0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
    0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL,
    0x209188240001000ULL, 0x400408a884001800ULL, 0x110400a6080400ULL, 0x1840060a44020800ULL,
    0x90080104000041ULL, 0x201011000808101ULL, 0x1a2208080504f080ULL, 0x8012020600211212ULL,
    0x500861011240000ULL, 0x180806108200800ULL, 0x4000020e01040044ULL, 0x300000261044000aULL,
    0x802241102020002ULL, 0x20906061210001ULL, 0x5a84841004010310ULL, 0x4010801011c04ULL,
    0xa010109502200ULL, 0x4a02012000ULL, 0x500201010098b028ULL, 0x8040002811040900ULL,
    0x28000010020204ULL, 0x6000020202d0240ULL, 0x8918844842082200ULL, 0x4010011029020020ULL
  };
  static std::array<Bitboard, NUM_SQUARES> rookMagics = {
    0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL, 0x100081001000420ULL,
    0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
    0x800098204000ULL, 0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL,
    0x208808088000400ULL, 0x2802200800400ULL, 0x2200800100020080ULL, 0x801000060821100ULL,
    0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL,
    0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
    0x100400080208000ULL, 0x2040002120081000ULL, 0x21200680100081ULL, 0x20100080080080ULL,
    0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL, 0x80004600042881ULL,
    0x4040008040800020ULL, 0x440003000200801ULL, 0x4200011004500ULL, 0x188020010100100ULL,
    0x14800401802800ULL, 0x2080040080800200ULL, 0x124080204001001ULL, 0x200046502000484ULL,
    0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL, 0x100021010009ULL,
    0x2002080100110004ULL, 0x202008004008002ULL, 0x20020004010100ULL, 0x2048440040820001ULL,
    0x101002200408200ULL, 0x40802000401080ULL, 0x4008142004410100ULL, 0x2060820c0120200ULL,
    0x1001004080100ULL, 0x20c020080040080ULL, 0x2935610830022400ULL, 0x44440041009200ULL,
    0x280001040802101ULL, 0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
    0x20030a0244872ULL, 0x12001008414402ULL, 0x2006104900a0804ULL, 0x1004081002402ULL
  };

  static std::array<int, NUM_SQUARES> bishopShifts;
  static std::array<int, NUM_SQUARES> rookShifts;




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

    occupancy &= bishopMasks[squareIndex];
    occupancy *= bishopMagics[squareIndex];
    occupancy >>= bishopShifts[squareIndex];

    return static_cast<int>(occupancy);
  }

  static int rookIndex(Square square, Bitboard occupancy) {
    int squareIndex = squareToInt(square);

    occupancy &= rookMasks[squareIndex];
    occupancy *= rookMagics[squareIndex];
    occupancy >>= rookShifts[squareIndex];

    return static_cast<int>(occupancy);
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

      bishopShifts[square] = 64 - relevantBits;
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

      rookShifts[square] = 64 - relevantBits;
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