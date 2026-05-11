#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Board.hpp"
#include "Move.hpp"

namespace Engine {
  struct Result {
    Move bestMove;
    uint64_t nodes;
  };




  Result search(Board& board, int depth);



  
  int evaluate(const Board& board);
}

#endif