#include "MoveGen.hpp"

namespace MoveGen {
  static void addMoveIfLegal(Board& board, std::vector<Move>& moves, const Move& move) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;

    Piece capturedPiece = board.pieceOn(move.to);
    Piece movingPiece = board.pieceOn(move.from);

    // Captures
    Square enPassantCaptureSquare = Square::NONE;
    if (move.flag == MoveFlag::EN_PASSANT) {
      enPassantCaptureSquare = (currentColor == Color::WHITE)
        ? intToSquare(squareToInt(move.to) - 8)
        : intToSquare(squareToInt(move.to) + 8);
      board.removePiece(enPassantCaptureSquare);
    }

    if (capturedPiece != Piece::NONE && move.flag != MoveFlag::EN_PASSANT) {
      board.removePiece(move.to);
    }

    // Moves
    board.removePiece(move.from);

    // Promotions
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

    // Check
    bool legal = !board.isInCheck(currentColor);

    // Undo changes
    if (move.flag == MoveFlag::CASTLE_KING) {
      Square rookFrom = (currentColor == Color::WHITE) ? Square::H1 : Square::H8;
      Square rookTo = (currentColor == Color::WHITE) ? Square::F1 : Square::F8;
      board.removePiece(rookTo);
      board.putPiece(currentColor, Piece::ROOK, rookFrom);
    } else if (move.flag == MoveFlag::CASTLE_QUEEN) {
      Square rookFrom = (currentColor == Color::WHITE) ? Square::A1 : Square::A8;
      Square rookTo = (currentColor == Color::WHITE) ? Square::D1 : Square::D8;
      board.removePiece(rookTo);
      board.putPiece(currentColor, Piece::ROOK, rookFrom);
    }
      
    board.removePiece(move.to);
    board.putPiece(currentColor, movingPiece, move.from);

    if (move.flag == MoveFlag::EN_PASSANT) {
      board.putPiece(enemyColor, Piece::PAWN, enPassantCaptureSquare);
    } else if (capturedPiece != Piece::NONE) {
      board.putPiece(enemyColor, capturedPiece, move.to);
    }

    if (legal) {
      moves.push_back(move);
    }
  }




  static void generatePawnMoves(Board& board, std::vector<Move>& moves) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentPawns = board.pieceBitboard(currentColor, Piece::PAWN);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Bitboard occupiedBitboard = board.allBitboard();

    int direction = (currentColor == Color::WHITE) ? 8 : -8;
    Bitboard promotionRank = (currentColor == Color::WHITE) ? RANK_8 : RANK_1;
    Bitboard startingRank = (currentColor == Color::WHITE) ? RANK_2 : RANK_7;

    // Single pawn pushes
    Bitboard singlePush = (currentColor == Color::WHITE)
      ? (currentPawns << 8) & ~occupiedBitboard
      : (currentPawns >> 8) & ~occupiedBitboard;

    while (singlePush) {
      int toIndex = popLowestBit(singlePush);
      Square toSquare = intToSquare(toIndex);
      Square fromSquare = intToSquare(toIndex - direction);

      if (squareBitboard(toSquare) & promotionRank) {
        addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::PROMOTE_KNIGHT));
        addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::PROMOTE_BISHOP));
        addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::PROMOTE_ROOK));
        addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::PROMOTE_QUEEN));
      } else {
        addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::QUIET));
      }
    }

    // Double pawn pushes
    Bitboard doublePush = (currentColor == Color::WHITE)
      ? ((currentPawns & startingRank) << 16) & ~occupiedBitboard & ~(occupiedBitboard << 8)
      : ((currentPawns & startingRank) >> 16) & ~occupiedBitboard & ~(occupiedBitboard >> 8);

    while (doublePush) {
      int toIndex = popLowestBit(doublePush);
      Square toSquare = intToSquare(toIndex);
      Square fromSquare = intToSquare(toIndex - direction * 2);
      addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::DOUBLE_PAWN));
    }

    // En passant
    Square enPassantSquare = board.enPassantSquare();
    if (enPassantSquare != Square::NONE) {
      Bitboard enPassantAttackers = Attacks::pawnAttacks[colorToInt(enemyColor)][squareToInt(enPassantSquare)] & currentPawns;
      
      while (enPassantAttackers) {
        int fromIndex = popLowestBit(enPassantAttackers);
        Square fromSquare = intToSquare(fromIndex);
        addMoveIfLegal(board, moves, Move(fromSquare, enPassantSquare, MoveFlag::EN_PASSANT));
      }
    }

    // Pawn captures
    while (currentPawns) {
      int fromIndex = popLowestBit(currentPawns);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard attacks = Attacks::pawnAttacks[colorToInt(currentColor)][fromIndex] & enemyColorBitboard;

      while (attacks) {
        int toIndex = popLowestBit(attacks);
        Square toSquare = intToSquare(toIndex);

        if (squareBitboard(toSquare) & promotionRank) {
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::PROMOTE_KNIGHT));
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::PROMOTE_BISHOP));
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::PROMOTE_ROOK));
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::PROMOTE_QUEEN));
        } else {
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
        }
      }
    }
  }

  static void generateKnightMoves(Board& board, std::vector<Move>& moves) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentKnights = board.pieceBitboard(currentColor, Piece::KNIGHT);
    Bitboard currentColorBitboard = board.colorBitboard(currentColor);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);

    while (currentKnights) {
      int fromIndex = popLowestBit(currentKnights);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::knightAttacks[fromIndex] & ~currentColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);

        if (squareBitboard(toSquare) & enemyColorBitboard) {
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
        } else {
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::QUIET));
        }
      }
    }
  }

  static void generateBishopMoves(Board& board, std::vector<Move>& moves) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentBishops = board.pieceBitboard(currentColor, Piece::BISHOP);
    Bitboard currentColorBitboard = board.colorBitboard(currentColor);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Bitboard occupiedBitboard = board.allBitboard();

    while (currentBishops) {
      int fromIndex = popLowestBit(currentBishops);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::bishopAttacks(occupiedBitboard, fromSquare) & ~currentColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);

        if (squareBitboard(toSquare) & enemyColorBitboard) {
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
        } else {
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::QUIET));
        }
      }
    }
  }

  static void generateRookMoves(Board& board, std::vector<Move>& moves) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentRooks = board.pieceBitboard(currentColor, Piece::ROOK);
    Bitboard currentColorBitboard = board.colorBitboard(currentColor);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Bitboard occupiedBitboard = board.allBitboard();

    while (currentRooks) {
      int fromIndex = popLowestBit(currentRooks);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::rookAttacks(occupiedBitboard, fromSquare) & ~currentColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);

        if (squareBitboard(toSquare) & enemyColorBitboard) {
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
        } else {
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::QUIET));
        }
      }
    }
  }

  static void generateQueenMoves(Board& board, std::vector<Move>& moves) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentQueens = board.pieceBitboard(currentColor, Piece::QUEEN);
    Bitboard currentColorBitboard = board.colorBitboard(currentColor);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Bitboard occupiedBitboard = board.allBitboard();

    while (currentQueens) {
      int fromIndex = popLowestBit(currentQueens);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::queenAttacks(occupiedBitboard, fromSquare) & ~currentColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);

        if (squareBitboard(toSquare) & enemyColorBitboard) {
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
        } else {
          addMoveIfLegal(board, moves, Move(fromSquare, toSquare, MoveFlag::QUIET));
        }
      }
    }
  }

  static void generateKingMoves(Board& board, std::vector<Move>& moves) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentColorBitboard = board.colorBitboard(currentColor);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Square kingSquare = board.findKing(currentColor);
    int kingIndex = squareToInt(kingSquare);

    // King moves
    Bitboard targets = Attacks::kingAttacks[kingIndex] & ~currentColorBitboard;

    while (targets) {
      int toIndex = popLowestBit(targets);
      Square toSquare = intToSquare(toIndex);

      if (squareBitboard(toSquare) & enemyColorBitboard) {
        addMoveIfLegal(board, moves, Move(kingSquare, toSquare, MoveFlag::CAPTURE));
      } else {
        addMoveIfLegal(board, moves, Move(kingSquare, toSquare, MoveFlag::QUIET));
      }
    }

    // Castling
    Bitboard occupiedBitboard = board.allBitboard();
    int castlingRights = board.castlingRights();

    if (currentColor == Color::WHITE) {
      if ((castlingRights & CASTLE_WHITE_KING)
          && !(occupiedBitboard & (squareBitboard(Square::F1) | squareBitboard(Square::G1)))
          && !board.isAttacked(Square::E1, enemyColor)
          && !board.isAttacked(Square::F1, enemyColor)
          && !board.isAttacked(Square::G1, enemyColor)) {
        moves.push_back(Move(Square::E1, Square::G1, MoveFlag::CASTLE_KING));
      }
      if ((castlingRights & CASTLE_WHITE_QUEEN)
          && !(occupiedBitboard & (squareBitboard(Square::D1) | squareBitboard(Square::C1) | squareBitboard(Square::B1)))
          && !board.isAttacked(Square::E1, enemyColor)
          && !board.isAttacked(Square::D1, enemyColor)
          && !board.isAttacked(Square::C1, enemyColor)) {
        moves.push_back(Move(Square::E1, Square::C1, MoveFlag::CASTLE_QUEEN));
      }
    } else {
      if ((castlingRights & CASTLE_BLACK_KING)
          && !(occupiedBitboard & (squareBitboard(Square::F8) | squareBitboard(Square::G8)))
          && !board.isAttacked(Square::E8, enemyColor)
          && !board.isAttacked(Square::F8, enemyColor)
          && !board.isAttacked(Square::G8, enemyColor)) {
          moves.push_back(Move(Square::E8, Square::G8, MoveFlag::CASTLE_KING));
      }
      if ((castlingRights & CASTLE_BLACK_QUEEN)
          && !(occupiedBitboard & (squareBitboard(Square::D8) | squareBitboard(Square::C8) | squareBitboard(Square::B8)))
          && !board.isAttacked(Square::E8, enemyColor)
          && !board.isAttacked(Square::D8, enemyColor)
          && !board.isAttacked(Square::C8, enemyColor)) {
          moves.push_back(Move(Square::E8, Square::C8, MoveFlag::CASTLE_QUEEN));
      }
    }
  }


  

  std::vector<Move> generate(Board& board) {
    std::vector<Move> moves;
    moves.reserve(256);

    generatePawnMoves(board, moves);
    generateKnightMoves(board, moves);
    generateBishopMoves(board, moves);
    generateRookMoves(board, moves);
    generateQueenMoves(board, moves);
    generateKingMoves(board, moves);

    return moves;
  }
}