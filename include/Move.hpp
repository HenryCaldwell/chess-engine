#ifndef MOVE_HPP
#define MOVE_HPP

#include "Bitboard.hpp"
#include "Board.hpp"
#include <iostream>
#include <string>

enum class MoveFlag : int {
  QUIET = 0,
  CAPTURE = 1 << 0,
  DOUBLE_PAWN = 1 << 1,
  EN_PASSANT = 1 << 2,
  CASTLE_KING = 1 << 3,
  CASTLE_QUEEN = 1 << 4,
  PROMOTE_KNIGHT = 1 << 5,
  PROMOTE_BISHOP = 1 << 6,
  PROMOTE_ROOK = 1 << 7,
  PROMOTE_QUEEN = 1 << 8,
  CHECK = 1 << 9,
  DOUBLE_CHECK = 1 << 10
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
    return static_cast<int>(flag) & (
      static_cast<int>(MoveFlag::PROMOTE_KNIGHT) |
      static_cast<int>(MoveFlag::PROMOTE_BISHOP) |
      static_cast<int>(MoveFlag::PROMOTE_ROOK) |
      static_cast<int>(MoveFlag::PROMOTE_QUEEN)
    );
  }

  bool isCapture() const {
    return static_cast<int>(flag) & (
      static_cast<int>(MoveFlag::CAPTURE) |
      static_cast<int>(MoveFlag::EN_PASSANT)
    );
  }

  bool isCastle() const {
    return static_cast<int>(flag) & (
      static_cast<int>(MoveFlag::CASTLE_KING) |
      static_cast<int>(MoveFlag::CASTLE_QUEEN)
    );
  }

  bool isCheck() const {
    return static_cast<int>(flag) & static_cast<int>(MoveFlag::CHECK);
  }

  bool isDoubleCheck() const {
    return static_cast<int>(flag) & static_cast<int>(MoveFlag::DOUBLE_CHECK);
  }

  bool isNull() const {
    return to == Square::NONE && from == Square::NONE;
  }

  std::string toUCI() const {
    std::string uci = squareToString(from) + squareToString(to);

    int flagBits = static_cast<int>(flag);
    if (flagBits & static_cast<int>(MoveFlag::PROMOTE_KNIGHT)) {
      uci += "n";
    } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_BISHOP)) {
      uci += "b";
    } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_ROOK)) {
      uci += "r";
    } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_QUEEN)) {
      uci += "q";
    }

    return uci;
  }

  std::string toSAN(const Board& board) const;




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