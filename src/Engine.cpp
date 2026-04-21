#include "Engine.hpp"
#include "MoveGen.hpp"
#include "TranspositionTable.hpp"
#include <algorithm>
#include <limits>

namespace Engine {
  static TranspositionTable transpositionTable;

  static constexpr int TT_MOVE_BONUS = 100000;

  static constexpr int PIECE_VALUES[] = { 100, 320, 330, 500, 900, 100000};

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
    int score = 0;

    for (int pieceIndex = 0; pieceIndex < NUM_PIECES; pieceIndex++) {
      Bitboard whitePieces = board.pieceBitboard(Color::WHITE, intToPiece(pieceIndex));
      while (whitePieces) {
        int square = popLowestBit(whitePieces);
        score += PIECE_VALUES[pieceIndex];
        score += PIECE_SQUARE_TABLES[pieceIndex][square];
      }

      Bitboard blackPieces = board.pieceBitboard(Color::BLACK, intToPiece(pieceIndex));
      while (blackPieces) {
        int square = popLowestBit(blackPieces);
        score -= PIECE_VALUES[pieceIndex];
        score -= PIECE_SQUARE_TABLES[pieceIndex][mirrorSquare(square)];
      }
    }

    return (board.currentTurn() == Color::WHITE) ? score : -score;
  }

  static int scoreMove(const Board& board, const Move& move, const Move& ttMove) {
    int score = 0;

    if (move == ttMove && !ttMove.isNull()) {
      score += TT_MOVE_BONUS;
    }

    if (move.isCapture()) {
      Piece capturedPiece = board.pieceOn(move.to);
      Piece movingPiece = board.pieceOn(move.from);

      if (capturedPiece != Piece::NONE) {
        score += PIECE_VALUES[pieceToInt(capturedPiece)] * 10 - PIECE_VALUES[pieceToInt(movingPiece)];
      }

      if (move.flag == MoveFlag::EN_PASSANT) {
        score += PIECE_VALUES[pieceToInt(Piece::PAWN)] * 10;
      }
    }

    if (move.isPromotion()) {
      score += PIECE_VALUES[pieceToInt(Piece::QUEEN)];
    }

    return score;
  }

  static void makeMove(Board& board, const Move& move) {
    Color currentColor = board.currentTurn();
    Piece movingPiece = board.pieceOn(move.from);

    // Captures
    if (move.flag == MoveFlag::EN_PASSANT) {
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

      switch (move.flag) {
        case MoveFlag::PROMOTE_KNIGHT:
          promotionPiece = Piece::KNIGHT;
          break;
        case MoveFlag::PROMOTE_BISHOP:
          promotionPiece = Piece::BISHOP;
          break;
        case MoveFlag::PROMOTE_ROOK:
          promotionPiece = Piece::ROOK;
          break;
        case MoveFlag::PROMOTE_QUEEN:
          promotionPiece = Piece::QUEEN;
          break;
        default:
          break;
      }

      board.putPiece(currentColor, promotionPiece, move.to);
    } else {
      board.putPiece(currentColor, movingPiece, move.to);
    }

    // Castling
    if (move.flag == MoveFlag::CASTLE_KING) {
      Square rookFrom = (currentColor == Color::WHITE) ? Square::H1 : Square::H8;
      Square rookTo = (currentColor == Color::WHITE) ? Square::F1 : Square::F8;
      board.removePiece(rookFrom);
      board.putPiece(currentColor, Piece::ROOK, rookTo);
    } else if (move.flag == MoveFlag::CASTLE_QUEEN) {
      Square rookFrom = (currentColor == Color::WHITE) ? Square::A1 : Square::A8;
      Square rookTo = (currentColor == Color::WHITE) ? Square::D1 : Square::D8;
      board.removePiece(rookFrom);
      board.putPiece(currentColor, Piece::ROOK, rookTo);
    }

    // En passant square
    if (move.flag == MoveFlag::DOUBLE_PAWN) {
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
    int standPatScore = evaluate(board);

    if (standPatScore >= beta) {
      return beta;
    }

    if (standPatScore > alpha) {
      alpha = standPatScore;
    }

    std::vector<Move> moves = MoveGen::generate(board);

    Move ttMove;
    std::sort(moves.begin(), moves.end(), [&board, &ttMove](const Move& moveA, const Move& moveB) {
      return scoreMove(board, moveA, ttMove) > scoreMove(board, moveB, ttMove);
    });

    for (const Move& move : moves) {
      if (!move.isCapture()) {
        continue;
      }

      Board backup = board;
      makeMove(board, move);
      int score = -quiescence(board, -beta, -alpha);
      board = backup;

      if (score >= beta) {
        return beta;
      }

      if (score > alpha) {
        alpha = score;
      }
    }

    return alpha;
  }

  static int alphaBeta(Board& board, int depth, int alpha, int beta) {
    if (depth == 0) {
      return quiescence(board, alpha, beta);
    }

    int originalAlpha = alpha;

    int ttScore;
    Move ttMove;
    if (transpositionTable.probe(board.hash(), depth, alpha, beta, ttScore, ttMove)) {
      return ttScore;
    }

    if (depth >= 3 && !board.isInCheck(board.currentTurn())) {
      Board backup = board;
      board.flipCurrentTurn();
      board.setEnPassantSquare(Square::NONE);

      int nullScore = -alphaBeta(board, depth - 3, -beta, -beta + 1);

      board = backup;

      if (nullScore >= beta) {
        return beta;
      }
    }

    std::vector<Move> moves = MoveGen::generate(board);

    if (moves.empty()) {
      if (board.isInCheck(board.currentTurn())) {
        return -100000 + (100 - depth);
      }

      return 0;
    }

    std::sort(moves.begin(), moves.end(), [&board, &ttMove](const Move& moveA, const Move& moveB) {
      return scoreMove(board, moveA, ttMove) > scoreMove(board, moveB, ttMove);
    });

    Move bestMove = moves[0];

    for (const Move& move : moves) {
      Board backup = board;
      makeMove(board, move);
      int score = -alphaBeta(board, depth - 1, -beta, -alpha);
      board = backup;

      if (score >= beta) {
        transpositionTable.store(board.hash(), depth, beta, HashFlag::BETA, move);

        return beta;
      }

      if (score > alpha) {
        alpha = score;
        bestMove = move;
      }
    }

    HashFlag flag = (alpha > originalAlpha) ? HashFlag::EXACT : HashFlag::ALPHA;
    transpositionTable.store(board.hash(), depth, alpha, flag, bestMove);

    return alpha;
  }

  Move search(Board& board, int depth) {
    Move bestMove;

    for (int currentDepth = 1; currentDepth <= depth; currentDepth++) {
      std::vector<Move> moves = MoveGen::generate(board);

      if (moves.empty()) {
        return bestMove;
      }

      int ttScore;
      Move ttMove;
      transpositionTable.probe(board.hash(), 0, 0, 0, ttScore, ttMove);

      std::sort(moves.begin(), moves.end(), [&board, &ttMove](const Move& moveA, const Move& moveB) {
        return scoreMove(board, moveA, ttMove) > scoreMove(board, moveB, ttMove);
      });

      int currentBestScore = std::numeric_limits<int>::min();
      Move currentBestMove = moves[0];

      for (const Move& move : moves) {
        Board backup = board;
        makeMove(board, move);
        int score = -alphaBeta(board, currentDepth - 1, std::numeric_limits<int>::min() + 1, std::numeric_limits<int>::max());
        board = backup;

        if (score > currentBestScore) {
          currentBestScore = score;
          currentBestMove = move;
        }
      }

      bestMove = currentBestMove;
    }

    return bestMove;
  }
}