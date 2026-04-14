#ifndef MOVE_HPP
#define MOVE_HPP

#include "Bitboard.hpp"
#include <iostream>
#include <string>

enum class MoveFlag : int {
  QUIET = 0,
  CAPTURE = 1,
  DOUBLE_PAWN = 2,
  EN_PASSANT = 3,
  CASTLE_KING = 4,
  CASTLE_QUEEN = 5,
  PROMOTE_KNIGHT = 6,
  PROMOTE_BISHOP = 7,
  PROMOTE_ROOK = 8,
  PROMOTE_QUEEN = 9
};




struct Move {
  Square from;
  Square to;
  MoveFlag flag;

  Move()
    : from(Square::NONE)
    , to(Square::NONE)
    , flag(MoveFlag::QUIET) {}

  Move(Square from, Square to, MoveFlag flag = MoveFlag::QUIET)
    : from(from)
    , to(to)
    , flag(flag) {}

  

  
  bool isPromotion() const {
    return flag == MoveFlag::PROMOTE_KNIGHT
      || flag == MoveFlag::PROMOTE_BISHOP
      || flag == MoveFlag::PROMOTE_ROOK
      || flag == MoveFlag::PROMOTE_QUEEN;
  }

  bool isCapture() const {
    return flag == MoveFlag::CAPTURE || flag == MoveFlag::EN_PASSANT;
  }

  bool isCastle() const {
    return flag == MoveFlag::CASTLE_KING || flag == MoveFlag::CASTLE_QUEEN;
  }

  bool isNull() const {
    return to == Square::NONE && from == Square::NONE;
  }

  std::string toUCI() const {
    std::string uci = squareToString(from) + squareToString(to);
    switch (flag) {
      case MoveFlag::PROMOTE_KNIGHT:
        uci += "n";
        break;
      case MoveFlag::PROMOTE_BISHOP:
        uci += "b";
        break;
      case MoveFlag::PROMOTE_ROOK:
        uci += "r";
        break;
      case MoveFlag::PROMOTE_QUEEN:
        uci += "q";
        break;
      default:
        break;
    }

    return uci;
  }




  bool operator==(const Move& other) const {
    return from == other.from
      && to == other.to
      && flag == other.flag;
  }

  bool operator!=(const Move& other) const {
    return !(*this == other);
  }

  bool operator<(const Move& other) const {
    if (from != other.from) {
      return from < other.from;
    }

    if (to != other.to) {
      return to < other.to;
    }

    return flag < other.flag;
  }
};

inline std::ostream& operator<<(std::ostream& stream, const Move& move) {
  stream << move.toUCI();

  return stream;
}

#endif