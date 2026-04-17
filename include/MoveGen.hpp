#include "Board.hpp"
#include "Move.hpp"
#include "Attacks.hpp"
#include <vector>

namespace MoveGen {
  std::vector<Move> generate(Board& board);
}