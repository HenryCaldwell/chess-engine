#include "Move.hpp"
#include "Board.hpp"
#include "Attacks.hpp"

std::string Move::toSAN(const Board& board) const {
  int flagBits = static_cast<int>(flag);

  if (flagBits & static_cast<int>(MoveFlag::CASTLE_KING)) {
    return isCheck() ? "O-O+" : "O-O";
  }

  if (flagBits & static_cast<int>(MoveFlag::CASTLE_QUEEN)) {
    return isCheck() ? "O-O-O+" : "O-O-O";
  }

  Piece piece = board.pieceOn(from);
  std::string san = "";
  
  if (piece != Piece::PAWN) {
    switch (piece) {
      case Piece::KNIGHT:
        san += "N";
        break;
      case Piece::BISHOP:
        san += "B";
        break;
      case Piece::ROOK:
        san += "R";
        break;
      case Piece::QUEEN:
        san += "Q";
        break;
      case Piece::KING:
        san += "K"; 
        break;
      default:
        break;
    }
  }

  if (piece != Piece::PAWN && piece != Piece::KING) {
    Color color = board.colorOn(from);
    Bitboard candidates = board.pieceBitboard(color, piece);

    int ambigFile = 0;
    int ambigRank = 0;

    while (candidates) {
      Square candidate = intToSquare(popLowestBit(candidates));

      if (candidate == from) {
        continue;
      }

      Bitboard attacks = 0;
      Bitboard occupied = board.allBitboard();

      switch (piece) {
        case Piece::KNIGHT:
          attacks = Attacks::knightAttacks(candidate);
          break;
        case Piece::BISHOP:
          attacks = Attacks::bishopAttacks(candidate, occupied);
          break;
        case Piece::ROOK:
          attacks = Attacks::rookAttacks(candidate, occupied);
          break;
        case Piece::QUEEN:
          attacks = Attacks::queenAttacks(candidate, occupied);
          break;
        default:
          break;
      }

      if (getBit(attacks, to)) {
        if (squareToInt(from) % 8 != squareToInt(candidate) % 8) {
          ambigFile = 1;
        } else {
          ambigRank = 1;
        }
      }
    }

    if (ambigFile) {
      san += ('a' + squareToInt(from) % 8);
    }

    if (ambigRank) {
      san += ('1' + squareToInt(from) / 8);
    }
  }

  if (isCapture()) {
    if (piece == Piece::PAWN) {
      san += ('a' + squareToInt(from) % 8);
    }

    san += "x";
  }

  san += squareToString(to);

  if (isPromotion()) {
    san += "=";

    if (flagBits & static_cast<int>(MoveFlag::PROMOTE_KNIGHT)) {
      san += "N";
    } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_BISHOP)) {
      san += "B";
    } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_ROOK)) {
      san += "R";
    } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_QUEEN)) {
      san += "Q";
    }
  }

  if (isCheck()) {
    san += "+";
  }

  return san;
}
