#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Board.hpp"
#include "Move.hpp"

namespace Engine {
  Move search(Board& board, int depth);




  int evaluate(const Board& board);
}

#endif