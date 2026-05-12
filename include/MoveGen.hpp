#ifndef MOVEGEN_HPP
#define MOVEGEN_HPP

#include "Board.hpp"
#include "Move.hpp"
#include <vector>

namespace MoveGen {
  void generateMoves(Board& board, Move* moves, int& moveCount);
  void generateCaptures(Board& board, Move* moves, int& moveCount);
}

#endif