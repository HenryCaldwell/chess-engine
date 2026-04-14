#include "Bitboard.hpp"

std::string squareToString(Square square) {
  if (square == Square::NONE) {
    return "-";
  }

  std::string result;
  result += static_cast<char>('a' + fileOf(square));
  result += static_cast<char>('1' + rankOf(square));

  return result;
}

Square stringToSquare(const std::string& str) {
  if (str.size() < 2) {
    return Square::NONE;
  }

  int file = str[0] - 'a';
  int rank = str[1] - '1';

  if (file < 0 || file > 7 || rank < 0 || rank > 7) {
    return Square::NONE;
  }

  return makeSquare(file, rank);
}