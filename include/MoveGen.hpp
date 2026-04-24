#include "Board.hpp"
#include "Move.hpp"
#include <vector>

namespace MoveGen {
  std::vector<Move> generateMoves(Board& board);
  std::vector<Move> generateCaptures(Board& board);
}