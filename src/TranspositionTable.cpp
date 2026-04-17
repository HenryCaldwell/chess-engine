#include "TranspositionTable.hpp"

TranspositionTable::TranspositionTable(int sizeMB) {
  size_ = (sizeMB * 1024 * 1024) / sizeof(HashEntry);
  table_.resize(size_);
  clear();
}

void TranspositionTable::store(uint64_t hash, int depth, int score, HashFlag flag, const Move& bestMove) {
  uint64_t index = hash % size_;
  HashEntry& entry = table_[index];

  if (entry.hash == 0 || depth >= entry.depth || entry.hash == hash) {
    entry.hash = hash;
    entry.depth = depth;
    entry.score = score;
    entry.flag = flag;
    entry.bestMove = bestMove;
  }
}

bool TranspositionTable::probe(uint64_t hash, int depth, int alpha, int beta, int& score, Move& bestMove) const {
  uint64_t index = hash % size_;
  const HashEntry& entry = table_[index];

  if (entry.hash != hash) {
    return false;
  }

  bestMove = entry.bestMove;

  if (entry.depth < depth) {
    return false;
  }

  if (entry.flag == HashFlag::EXACT) {
    score = entry.score;

    return true;
  }

  if (entry.flag == HashFlag::BETA && entry.score >= beta) {
    score = entry.score;

    return true;
  }

  if (entry.flag == HashFlag::ALPHA && entry.score <= alpha) {
    score = entry.score;

    return true;
  }

  return false;
}

void TranspositionTable::clear() {
  for (uint64_t i = 0; i < size_; i++) {
    table_[i] = HashEntry{};
  }
}