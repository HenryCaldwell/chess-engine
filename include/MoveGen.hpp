#ifndef MOVEGEN_HPP
#define MOVEGEN_HPP

#include "Board.hpp"
#include "Move.hpp"
#include <vector>

namespace MoveGen {
  std::vector<Move> generateMoves(Board& board);
  std::vector<Move> generateCaptures(Board& board);
}

#endif