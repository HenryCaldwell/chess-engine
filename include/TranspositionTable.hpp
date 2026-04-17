#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP

#include "Move.hpp"
#include <cstdint>
#include <vector>

enum class HashFlag : int {
  EXACT = 0,
  ALPHA = 1,
  BETA = 2
};




struct HashEntry {
  uint64_t hash;
  int depth;
  int score;
  HashFlag flag;
  Move bestMove;
};




class TranspositionTable {
public:
  TranspositionTable(int sizeMB = 64);

  void store(uint64_t hash, int depth, int score, HashFlag flag, const Move& bestMove);
  bool probe(uint64_t hash, int depth, int alpha, int beta, int& score, Move& bestMove) const;
  void clear();

private:
  std::vector<HashEntry> table_;
  uint64_t size_;
};

#endif