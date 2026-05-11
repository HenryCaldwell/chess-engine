#include "Engine.hpp"
#include "MoveGen.hpp"
#include "TranspositionTable.hpp"
#include <algorithm>
#include <limits>

namespace Engine {
  // Total nodes visited during search for benchmarking pruning efficiency
  static uint64_t nodeCount;

  // Shared transposition table reused across searches for move ordering and cached bounds
  static TranspositionTable transpositionTable;

  // Large bonus so the transposition table move is searched first
  static constexpr int TT_MOVE_BONUS = 100000;

  // Aspiration window size in centipawns around the previous iteration score
  static constexpr int WINDOW_SIZE = 50;

  // Basic material values in centipawns
  static constexpr int PIECE_VALUES[] = { 100, 320, 330, 500, 900, 100000 };

  static constexpr int PAWN_TABLE[NUM_SQUARES] = {
     0,  0,   0,   0,   0,   0,   0,   0,
     5, 10,  10, -20, -20,  10,  10,   5,
     5, -5, -10,   0,   0, -10,  -5,   5,
     0,  0,   0,  20,  20,   0,   0,   0,
     5,  5,  10,  25,  25,  10,   5,   5,
    10, 10,  20,  30,  30,  20,  10,  10,
    50, 50,  50,  50,  50,  50,  50,  50,
     0,  0,   0,   0,   0,   0,   0,   0
  };

  static constexpr int KNIGHT_TABLE[NUM_SQUARES] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
  };

  static constexpr int BISHOP_TABLE[NUM_SQUARES] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
  };

  static constexpr int ROOK_TABLE[NUM_SQUARES] = {
     0,  0,  0,  5,  5,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     5, 10, 10, 10, 10, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
  };

  static constexpr int QUEEN_TABLE[NUM_SQUARES] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10,   0,   5,  0,  0,   0,   0, -10,
    -10,   5,   5,  5,  5,   5,   0, -10,
      0,   0,   5,  5,  5,   5,   0,  -5,
     -5,   0,   5,  5,  5,   5,   0,  -5,
    -10,   0,   5,  5,  5,   5,   0, -10,
    -10,   0,   0,  0,  0,   0,   0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
  };

  static constexpr int KING_TABLE[NUM_SQUARES] = {
     20,  30,  10,   0,   0,  10,  30,  20,
     20,  20,   0,   0,   0,   0,  20,  20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
  };

  static constexpr const int* PIECE_SQUARE_TABLES[] = {
    PAWN_TABLE, KNIGHT_TABLE, BISHOP_TABLE, ROOK_TABLE, QUEEN_TABLE, KING_TABLE
  };




  static constexpr int mirrorSquare(int square) {
    return square ^ 56;
  }

  


  int evaluate(const Board& board) {
    // Positive score means White is better before converting to side-to-move perspective
    int score = 0;

    // Add material and positional bonuses for each piece type
    for (int pieceIndex = 0; pieceIndex < NUM_PIECES; pieceIndex++) {
      // Score white pieces positively
      Bitboard whitePieces = board.pieceBitboard(Color::WHITE, intToPiece(pieceIndex));
      while (whitePieces) {
        int square = popLowestBit(whitePieces);
        score += PIECE_VALUES[pieceIndex];
        score += PIECE_SQUARE_TABLES[pieceIndex][square];
      }

      // Score black pieces negatively
      Bitboard blackPieces = board.pieceBitboard(Color::BLACK, intToPiece(pieceIndex));
      while (blackPieces) {
        int square = popLowestBit(blackPieces);
        score -= PIECE_VALUES[pieceIndex];
        score -= PIECE_SQUARE_TABLES[pieceIndex][mirrorSquare(square)];
      }
    }

    // Negamax expects evaluation from the side-to-move perspective
    return (board.currentTurn() == Color::WHITE) ? score : -score;
  }

  static int scoreMove(const Board& board, const Move& move, const Move& ttMove) {
    // Heuristic score used only for move ordering, not final evaluation
    int score = 0;

    // Search the TT move first to maximize alpha-beta pruning
    if (move == ttMove && !ttMove.isNull()) {
      score += TT_MOVE_BONUS;
    }

    // MVV-LVA style ordering lets valuable captures by cheap pieces get searched early
    if (move.isCapture()) {
      Piece capturedPiece = board.pieceOn(move.to);
      Piece movingPiece = board.pieceOn(move.from);

      if (capturedPiece != Piece::NONE) {
        score += PIECE_VALUES[pieceToInt(capturedPiece)] * 10 - PIECE_VALUES[pieceToInt(movingPiece)];
      }

      if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::EN_PASSANT)) {
        score += PIECE_VALUES[pieceToInt(Piece::PAWN)] * 10;
      }
    }

    // Promotions are searched early because they are tactically forcing
    if (move.isPromotion()) {
      score += PIECE_VALUES[pieceToInt(Piece::QUEEN)];
    }

    return score;
  }

  static void makeMove(Board& board, const Move& move) {
    Color currentColor = board.currentTurn();
    Piece movingPiece = board.pieceOn(move.from);

    // Captures
    if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::EN_PASSANT)) {
      Square captureSquare = (currentColor == Color::WHITE)
        ? intToSquare(squareToInt(move.to) - 8)
        : intToSquare(squareToInt(move.to) + 8);
      board.removePiece(captureSquare);
    }
    
    if (board.pieceOn(move.to) != Piece::NONE) {
      board.removePiece(move.to);
    }

    // Moves
    board.removePiece(move.from);

    // Promotion
    if (move.isPromotion()) {
      Piece promotionPiece = Piece::NONE;

      int flagBits = static_cast<int>(move.flag);
      if (flagBits & static_cast<int>(MoveFlag::PROMOTE_KNIGHT)) {
        promotionPiece = Piece::KNIGHT;
      } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_BISHOP)) {
        promotionPiece = Piece::BISHOP;
      } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_ROOK)) {
        promotionPiece = Piece::ROOK;
      } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_QUEEN)) {
        promotionPiece = Piece::QUEEN;
      }

      board.putPiece(currentColor, promotionPiece, move.to);
    } else {
      board.putPiece(currentColor, movingPiece, move.to);
    }

    // Castling
    if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::CASTLE_KING)) {
      Square rookFrom = (currentColor == Color::WHITE) ? Square::H1 : Square::H8;
      Square rookTo = (currentColor == Color::WHITE) ? Square::F1 : Square::F8;
      board.removePiece(rookFrom);
      board.putPiece(currentColor, Piece::ROOK, rookTo);
    } else if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::CASTLE_QUEEN)) {
      Square rookFrom = (currentColor == Color::WHITE) ? Square::A1 : Square::A8;
      Square rookTo = (currentColor == Color::WHITE) ? Square::D1 : Square::D8;
      board.removePiece(rookFrom);
      board.putPiece(currentColor, Piece::ROOK, rookTo);
    }

    // En passant square
    if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::DOUBLE_PAWN)) {
      int enPassantIndex = (squareToInt(move.from) + squareToInt(move.to)) / 2;
      board.setEnPassantSquare(intToSquare(enPassantIndex));
    } else {
      board.setEnPassantSquare(Square::NONE);
    }

    // Castling rights
    int castlingRights = board.castlingRights();
    if (movingPiece == Piece::KING) {
      if (currentColor == Color::WHITE) {
        castlingRights &= ~(CASTLE_WHITE_KING | CASTLE_WHITE_QUEEN);
      } else {
        castlingRights &= ~(CASTLE_BLACK_KING | CASTLE_BLACK_QUEEN);
      }
    }

    if (move.from == Square::A1 || move.to == Square::A1) {
      castlingRights &= ~CASTLE_WHITE_QUEEN;
    }

    if (move.from == Square::H1 || move.to == Square::H1) {
      castlingRights &= ~CASTLE_WHITE_KING;
    }

    if (move.from == Square::A8 || move.to == Square::A8) {
      castlingRights &= ~CASTLE_BLACK_QUEEN;
    }

    if (move.from == Square::H8 || move.to == Square::H8) {
      castlingRights &= ~CASTLE_BLACK_KING;
    }

    board.setCastlingRights(castlingRights);

    // Switch turn
    board.flipCurrentTurn();
  }

  static int quiescence(Board& board, int alpha, int beta) {
    nodeCount++;

    // Stand-pat score assumes no more captures are made
    int standPatScore = evaluate(board);

    // If static eval already exceeds beta, prune immediately
    if (standPatScore >= beta) {
      return beta;
    }

    // If the quiet position improves alpha, update alpha
    if (standPatScore > alpha) {
      alpha = standPatScore;
    }

    std::vector<Move> moves = MoveGen::generateCaptures(board);

    Move ttMove;
    // Search promising captures first to stabilize tactical leaf positions quickly
    std::sort(moves.begin(), moves.end(), [&board, &ttMove](const Move& moveA, const Move& moveB) {
      return scoreMove(board, moveA, ttMove) > scoreMove(board, moveB, ttMove);
    });

    for (const Move& move : moves) {
      // Quiescence ignores quiet moves to limit the horizon search
      if (!move.isCapture()) {
        continue;
      }

      // Try capture and flip perspective with negamax
      Board backup = board;
      makeMove(board, move);
      int score = -quiescence(board, -beta, -alpha);
      board = backup;

      // If the capture beats beta, prune the remaining captures
      if (score >= beta) {
        return beta;
      }

      // If the capture improves alpha, update alpha
      if (score > alpha) {
        alpha = score;
      }
    }

    return alpha;
  }

  static int alphaBeta(Board& board, int depth, int alpha, int beta) {
    nodeCount++;

    // At the depth limit, switch to quiescence to resolve forcing captures
    if (depth == 0) {
      return quiescence(board, alpha, beta);
    }

    // Save original alpha so TT can distinguish exact scores from bounds
    int originalAlpha = alpha;

    int ttScore;
    Move ttMove;
    // Probe TT for reusable score/bound and preferred move ordering
    if (transpositionTable.probe(board.hash(), depth, alpha, beta, ttScore, ttMove)) {
      return ttScore;
    }

    // Null move pruning checks if even passing the turn still beats beta so real moves can be pruned
    if (depth >= 3 && !board.isInCheck(board.currentTurn())) {
      Board backup = board;
      // Make a null move by only flipping turn and clearing en passant
      board.flipCurrentTurn();
      board.setEnPassantSquare(Square::NONE);

      // Reduced zero-window search checks if even passing still beats beta
      int nullScore = -alphaBeta(board, depth - 3, -beta, -beta + 1);

      board = backup;

      // If the null move beats beta, prune the node
      if (nullScore >= beta) {
        return beta;
      }
    }

    std::vector<Move> moves = MoveGen::generateMoves(board);

    // No legal moves means checkmate or stalemate
    if (moves.empty()) {
      // Checkmate
      if (board.isInCheck(board.currentTurn())) {
        return -100000 + (100 - depth);
      }

      // Stalemate
      return 0;
    }

    // Good moves are searched first so alpha-beta can prune more aggressively
    std::sort(moves.begin(), moves.end(), [&board, &ttMove](const Move& moveA, const Move& moveB) {
      return scoreMove(board, moveA, ttMove) > scoreMove(board, moveB, ttMove);
    });

    Move bestMove = moves[0];
    int movesSearched = 0;

    for (const Move& move : moves) {
      Board backup = board;
      makeMove(board, move);

      int score;
      // The first move gets a full-window search because it is likely best
      if (movesSearched == 0) {
        score = -alphaBeta(board, depth - 1, -beta, -alpha);
      } else {
        int reduction = 0;
        // LMR reduces quiet late moves because they are less likely to improve alpha
        if (movesSearched >= 3 && depth >= 3 && !move.isCapture() && !move.isPromotion()) {
          reduction = 1;
        }

        // PVS uses a cheap zero-window search to check whether the move can beat alpha
        score = -alphaBeta(board, depth - 1 - reduction, -alpha - 1, -alpha);

        // If the probe beats alpha, re-search full-window for the real score
        if (score > alpha) {
          score = -alphaBeta(board, depth - 1, -beta, -alpha);
        }
      }

      board = backup;

      // If the move beats beta, prune the remaining moves
      if (score >= beta) {
        transpositionTable.store(board.hash(), depth, beta, HashFlag::BETA, move);

        return beta;
      }

      // If the move improves alpha, update alpha and store the best move
      if (score > alpha) {
        alpha = score;
        bestMove = move;
      }

      movesSearched++;
    }

    // Store an exact score if alpha improved, otherwise store an alpha bound
    HashFlag flag = (alpha > originalAlpha) ? HashFlag::EXACT : HashFlag::ALPHA;
    transpositionTable.store(board.hash(), depth, alpha, flag, bestMove);

    return alpha;
  }

  Result search(Board& board, int depth) {
    nodeCount = 0;
    Move bestMove;

    // Previous completed iteration score seeds the next aspiration window
    int previousScore = 0;

    // Iterative deepening searches from shallow depths up to the requested depth
    for (int currentDepth = 1; currentDepth <= depth; currentDepth++) {
      std::vector<Move> moves = MoveGen::generateMoves(board);

      if (moves.empty()) {
        return { bestMove, nodeCount };
      }

      int ttScore;
      Move ttMove;
      // Pull the root TT move so it gets searched first
      transpositionTable.probe(board.hash(), 0, 0, 0, ttScore, ttMove);

      // Order root moves before searching this iteration
      std::sort(moves.begin(), moves.end(), [&board, &ttMove](const Move& moveA, const Move& moveB) {
        return scoreMove(board, moveA, ttMove) > scoreMove(board, moveB, ttMove);
      });

      // The first iteration uses a full window and later iterations use aspiration windows
      int alpha = (currentDepth == 1) ? std::numeric_limits<int>::min() + 1 : previousScore - WINDOW_SIZE;
      int beta = (currentDepth == 1) ? std::numeric_limits<int>::max() : previousScore + WINDOW_SIZE;
      // Save original lower bound so fail-low detection ignores alpha updates
      int originalAlpha = alpha;

      int currentBestScore = std::numeric_limits<int>::min();
      Move currentBestMove = moves[0];

      for (const Move& move : moves) {
        // Try each root move and search the resulting position
        Board backup = board;
        makeMove(board, move);
        int score = -alphaBeta(board, currentDepth - 1, -beta, -alpha);
        board = backup;

        // Store the best move found for this completed depth
        if (score > currentBestScore) {
          currentBestScore = score;
          currentBestMove = move;
        }

        // If the move improves alpha, update alpha for the remaining root searches
        if (score > alpha) {
          alpha = score;
        }
      }

      // If the score falls outside the aspiration window, re-search with a full window
      if (currentBestScore <= originalAlpha || currentBestScore >= beta) {
        currentBestScore = std::numeric_limits<int>::min();
        currentBestMove = moves[0];

        for (const Move& move : moves) {
          // Full-window re-search recovers the exact score after aspiration failure
          Board backup = board;
          makeMove(board, move);
          int score = -alphaBeta(board, currentDepth - 1, std::numeric_limits<int>::min() + 1, std::numeric_limits<int>::max());
          board = backup;

          if (score > currentBestScore) {
            currentBestScore = score;
            currentBestMove = move;
          }
        }
      }

      // Store this depth result for the next aspiration window and final best move
      previousScore = currentBestScore;
      bestMove = currentBestMove;
    }

    return { bestMove, nodeCount };
  }
}