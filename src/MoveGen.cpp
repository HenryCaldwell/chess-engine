#include "MoveGen.hpp"
#include "Attacks.hpp"

namespace MoveGen {
  static Bitboard pinnedBitboard = 0;

  static bool inCheck = false;

  static Bitboard pinnedPieces(Board& board) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Square kingSquare = board.findKing(currentColor);

    Bitboard pinned = 0;

    Bitboard occupiedBitboard = board.allBitboard();
    Bitboard currentColorBitboard = board.colorBitboard(currentColor);

    // Diagonal pinners
    Bitboard diagonalSliders = board.pieceBitboard(enemyColor, Piece::BISHOP) | board.pieceBitboard(enemyColor, Piece::QUEEN);
    Bitboard potentialPinners = diagonalSliders & Attacks::bishopAttacks(kingSquare, 0);

    while (potentialPinners) {
      Square sliderSquare = intToSquare(popLowestBit(potentialPinners));
      Bitboard endpoints = squareBitboard(kingSquare) | squareBitboard(sliderSquare);
      Bitboard between = Attacks::bishopAttacks(kingSquare, endpoints) & Attacks::bishopAttacks(sliderSquare, endpoints);
      Bitboard piecesBetween = between & occupiedBitboard;

      if (__builtin_popcountll(piecesBetween) == 1 && (piecesBetween & currentColorBitboard)) {
        pinned |= piecesBetween & currentColorBitboard;
      }
    }

    // Straight pinners
    Bitboard straightSliders = board.pieceBitboard(enemyColor, Piece::ROOK) | board.pieceBitboard(enemyColor, Piece::QUEEN);
    potentialPinners = straightSliders & Attacks::rookAttacks(kingSquare, 0);

    while (potentialPinners) {
      Square sliderSquare = intToSquare(popLowestBit(potentialPinners));
      Bitboard endpoints = squareBitboard(kingSquare) | squareBitboard(sliderSquare);
      Bitboard between = Attacks::rookAttacks(kingSquare, endpoints) & Attacks::rookAttacks(sliderSquare, endpoints);
      Bitboard piecesBetween = between & occupiedBitboard;

      if (__builtin_popcountll(piecesBetween) == 1 && (piecesBetween & currentColorBitboard)) {
        pinned |= piecesBetween & currentColorBitboard;
      }
    }

    return pinned;
  }

  static bool givesCheck(Board& board, const Move& move, Square enemyKingSquare) {
    Color currentColor = board.currentTurn();
    Piece movingPiece = board.pieceOn(move.from);

    if (move.isPromotion()) {
      int flagBits = static_cast<int>(move.flag);
      if (flagBits & static_cast<int>(MoveFlag::PROMOTE_KNIGHT)) {
        movingPiece = Piece::KNIGHT;
      } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_BISHOP)) {
        movingPiece = Piece::BISHOP;
      } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_ROOK)) {
        movingPiece = Piece::ROOK;
      } else if (flagBits & static_cast<int>(MoveFlag::PROMOTE_QUEEN)) {
        movingPiece = Piece::QUEEN;
      }
    }

    Bitboard occupiedBitboard = board.allBitboard();
    occupiedBitboard &= ~squareBitboard(move.from);
    occupiedBitboard |= squareBitboard(move.to);

    if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::EN_PASSANT)) {
      Square enPassantCaptureSquare = (currentColor == Color::WHITE)
        ? intToSquare(squareToInt(move.to) - 8)
        : intToSquare(squareToInt(move.to) + 8);
      occupiedBitboard &= ~squareBitboard(enPassantCaptureSquare);
    }

    Bitboard enemyKingBitboard = squareBitboard(enemyKingSquare);

    switch (movingPiece) {
      case Piece::PAWN:
        if (Attacks::pawnAttacks(currentColor, move.to) & enemyKingBitboard) {
          return true;
        }

        break;
      case Piece::KNIGHT:
        if (Attacks::knightAttacks(move.to) & enemyKingBitboard) {
          return true;
        }

        break;
      case Piece::BISHOP:
        if (Attacks::bishopAttacks(move.to, occupiedBitboard) & enemyKingBitboard) {
          return true;
        }

        break;
      case Piece::ROOK:
        if (Attacks::rookAttacks(move.to, occupiedBitboard) & enemyKingBitboard) {
          return true;
        }

        break;
      case Piece::QUEEN:
        if (Attacks::queenAttacks(move.to, occupiedBitboard) & enemyKingBitboard) {
          return true;
        }

        break;
      default:
        break;
    }

    Bitboard diagonalSliders = board.pieceBitboard(currentColor, Piece::BISHOP) | board.pieceBitboard(currentColor, Piece::QUEEN);
    Bitboard straightSliders = board.pieceBitboard(currentColor, Piece::ROOK) | board.pieceBitboard(currentColor, Piece::QUEEN);
    Bitboard destinationBitboard = squareBitboard(move.to);

    if (Attacks::bishopAttacks(enemyKingSquare, occupiedBitboard) & diagonalSliders & ~destinationBitboard) {
      return true;
    }

    if (Attacks::rookAttacks(enemyKingSquare, occupiedBitboard) & straightSliders & ~destinationBitboard) {
      return true;
    }

    return false;
  }

  static void addMoveIfLegal(Board& board, Move* moves, int& moveCount, Move move) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Piece movingPiece = board.pieceOn(move.from);

    bool kingMove = (movingPiece == Piece::KING);
    bool pinned = (pinnedBitboard & squareBitboard(move.from)) != 0;
    bool enPassant = static_cast<int>(move.flag) & static_cast<int>(MoveFlag::EN_PASSANT);

    if (!inCheck && !kingMove && !pinned && !enPassant) {
      Square enemyKingSquare = board.findKing(enemyColor);

      if (givesCheck(board, move, enemyKingSquare)) {
        move.flag = static_cast<MoveFlag>(static_cast<int>(move.flag) | static_cast<int>(MoveFlag::CHECK));
      }

      moves[moveCount++] = move;
      
      return;
    }

    Piece capturedPiece = board.pieceOn(move.to);

    // Captures
    Square enPassantCaptureSquare = Square::NONE;
    if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::EN_PASSANT)) {
      enPassantCaptureSquare = (currentColor == Color::WHITE)
        ? intToSquare(squareToInt(move.to) - 8)
        : intToSquare(squareToInt(move.to) + 8);
      board.removePiece(enPassantCaptureSquare);
    }

    if (capturedPiece != Piece::NONE && !(static_cast<int>(move.flag) & static_cast<int>(MoveFlag::EN_PASSANT))) {
      board.removePiece(move.to);
    }

    // Moves
    board.removePiece(move.from);

    // Promotions
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

    // Check
    bool legal = !board.isInCheck(currentColor);
    bool check = board.isInCheck(enemyColor);

    // Undo changes
    if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::CASTLE_KING)) {
      Square rookFrom = (currentColor == Color::WHITE) ? Square::H1 : Square::H8;
      Square rookTo = (currentColor == Color::WHITE) ? Square::F1 : Square::F8;
      board.removePiece(rookTo);
      board.putPiece(currentColor, Piece::ROOK, rookFrom);
    } else if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::CASTLE_QUEEN)) {
      Square rookFrom = (currentColor == Color::WHITE) ? Square::A1 : Square::A8;
      Square rookTo = (currentColor == Color::WHITE) ? Square::D1 : Square::D8;
      board.removePiece(rookTo);
      board.putPiece(currentColor, Piece::ROOK, rookFrom);
    }
      
    board.removePiece(move.to);
    board.putPiece(currentColor, movingPiece, move.from);

    if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::EN_PASSANT)) {
      board.putPiece(enemyColor, Piece::PAWN, enPassantCaptureSquare);
    } else if (capturedPiece != Piece::NONE) {
      board.putPiece(enemyColor, capturedPiece, move.to);
    }

    if (legal) {
      if (check) {
        move.flag = static_cast<MoveFlag>(static_cast<int>(move.flag) | static_cast<int>(MoveFlag::CHECK));
      }

      moves[moveCount++] = move;
    }
  }




  static void generatePawnMoves(Board& board, Move* moves, int& moveCount) {
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
        addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_KNIGHT));
        addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_BISHOP));
        addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_ROOK));
        addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_QUEEN));
      } else {
        addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::QUIET));
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
      addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::DOUBLE_PAWN));
    }

    // En passant
    Square enPassantSquare = board.enPassantSquare();
    if (enPassantSquare != Square::NONE) {
      Bitboard enPassantAttackers = Attacks::pawnAttacks(enemyColor, enPassantSquare) & currentPawns;
      
      while (enPassantAttackers) {
        int fromIndex = popLowestBit(enPassantAttackers);
        Square fromSquare = intToSquare(fromIndex);
        addMoveIfLegal(board, moves, moveCount, Move(fromSquare, enPassantSquare, MoveFlag::EN_PASSANT));
      }
    }

    // Pawn captures
    while (currentPawns) {
      int fromIndex = popLowestBit(currentPawns);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard attacks = Attacks::pawnAttacks(currentColor, fromSquare) & enemyColorBitboard;

      while (attacks) {
        int toIndex = popLowestBit(attacks);
        Square toSquare = intToSquare(toIndex);

        if (squareBitboard(toSquare) & promotionRank) {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_KNIGHT));
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_BISHOP));
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_ROOK));
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_QUEEN));
        } else {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
        }
      }
    }
  }

  static void generateKnightMoves(Board& board, Move* moves, int& moveCount) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentKnights = board.pieceBitboard(currentColor, Piece::KNIGHT);
    Bitboard currentColorBitboard = board.colorBitboard(currentColor);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);

    while (currentKnights) {
      int fromIndex = popLowestBit(currentKnights);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::knightAttacks(fromSquare) & ~currentColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);

        if (squareBitboard(toSquare) & enemyColorBitboard) {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
        } else {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::QUIET));
        }
      }
    }
  }

  static void generateBishopMoves(Board& board, Move* moves, int& moveCount) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentBishops = board.pieceBitboard(currentColor, Piece::BISHOP);
    Bitboard currentColorBitboard = board.colorBitboard(currentColor);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Bitboard occupiedBitboard = board.allBitboard();

    while (currentBishops) {
      int fromIndex = popLowestBit(currentBishops);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::bishopAttacks(fromSquare, occupiedBitboard) & ~currentColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);

        if (squareBitboard(toSquare) & enemyColorBitboard) {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
        } else {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::QUIET));
        }
      }
    }
  }

  static void generateRookMoves(Board& board, Move* moves, int& moveCount) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentRooks = board.pieceBitboard(currentColor, Piece::ROOK);
    Bitboard currentColorBitboard = board.colorBitboard(currentColor);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Bitboard occupiedBitboard = board.allBitboard();

    while (currentRooks) {
      int fromIndex = popLowestBit(currentRooks);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::rookAttacks(fromSquare, occupiedBitboard) & ~currentColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);

        if (squareBitboard(toSquare) & enemyColorBitboard) {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
        } else {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::QUIET));
        }
      }
    }
  }

  static void generateQueenMoves(Board& board, Move* moves, int& moveCount) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentQueens = board.pieceBitboard(currentColor, Piece::QUEEN);
    Bitboard currentColorBitboard = board.colorBitboard(currentColor);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Bitboard occupiedBitboard = board.allBitboard();

    while (currentQueens) {
      int fromIndex = popLowestBit(currentQueens);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::queenAttacks(fromSquare, occupiedBitboard) & ~currentColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);

        if (squareBitboard(toSquare) & enemyColorBitboard) {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
        } else {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::QUIET));
        }
      }
    }
  }

  static void generateKingMoves(Board& board, Move* moves, int& moveCount) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentColorBitboard = board.colorBitboard(currentColor);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Square kingSquare = board.findKing(currentColor);

    // King moves
    Bitboard targets = Attacks::kingAttacks(kingSquare) & ~currentColorBitboard;

    while (targets) {
      int toIndex = popLowestBit(targets);
      Square toSquare = intToSquare(toIndex);

      if (squareBitboard(toSquare) & enemyColorBitboard) {
        addMoveIfLegal(board, moves, moveCount, Move(kingSquare, toSquare, MoveFlag::CAPTURE));
      } else {
        addMoveIfLegal(board, moves, moveCount, Move(kingSquare, toSquare, MoveFlag::QUIET));
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
        moves[moveCount++] = Move(Square::E1, Square::G1, MoveFlag::CASTLE_KING);
      }
      if ((castlingRights & CASTLE_WHITE_QUEEN)
          && !(occupiedBitboard & (squareBitboard(Square::D1) | squareBitboard(Square::C1) | squareBitboard(Square::B1)))
          && !board.isAttacked(Square::E1, enemyColor)
          && !board.isAttacked(Square::D1, enemyColor)
          && !board.isAttacked(Square::C1, enemyColor)) {
        moves[moveCount++] = Move(Square::E1, Square::C1, MoveFlag::CASTLE_QUEEN);
      }
    } else {
      if ((castlingRights & CASTLE_BLACK_KING)
          && !(occupiedBitboard & (squareBitboard(Square::F8) | squareBitboard(Square::G8)))
          && !board.isAttacked(Square::E8, enemyColor)
          && !board.isAttacked(Square::F8, enemyColor)
          && !board.isAttacked(Square::G8, enemyColor)) {
          moves[moveCount++] = Move(Square::E8, Square::G8, MoveFlag::CASTLE_KING);
      }
      if ((castlingRights & CASTLE_BLACK_QUEEN)
          && !(occupiedBitboard & (squareBitboard(Square::D8) | squareBitboard(Square::C8) | squareBitboard(Square::B8)))
          && !board.isAttacked(Square::E8, enemyColor)
          && !board.isAttacked(Square::D8, enemyColor)
          && !board.isAttacked(Square::C8, enemyColor)) {
          moves[moveCount++] = Move(Square::E8, Square::C8, MoveFlag::CASTLE_QUEEN);
      }
    }
  }




  static void generatePawnCaptures(Board& board, Move* moves, int& moveCount) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentPawns = board.pieceBitboard(currentColor, Piece::PAWN);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Bitboard promotionRank = (currentColor == Color::WHITE) ? RANK_8 : RANK_1;

    // En passant
    Square enPassantSquare = board.enPassantSquare();
    if (enPassantSquare != Square::NONE) {
      Bitboard enPassantAttackers = Attacks::pawnAttacks(enemyColor, enPassantSquare) & currentPawns;
      
      while (enPassantAttackers) {
        int fromIndex = popLowestBit(enPassantAttackers);
        Square fromSquare = intToSquare(fromIndex);
        addMoveIfLegal(board, moves, moveCount, Move(fromSquare, enPassantSquare, MoveFlag::EN_PASSANT));
      }
    }

    // Pawn captures
    while (currentPawns) {
      int fromIndex = popLowestBit(currentPawns);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard attacks = Attacks::pawnAttacks(currentColor, fromSquare) & enemyColorBitboard;

      while (attacks) {
        int toIndex = popLowestBit(attacks);
        Square toSquare = intToSquare(toIndex);

        if (squareBitboard(toSquare) & promotionRank) {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_KNIGHT));
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_BISHOP));
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_ROOK));
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::PROMOTE_QUEEN));
        } else {
          addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
        }
      }
    }
  }

  static void generateKnightCaptures(Board& board, Move* moves, int& moveCount) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentKnights = board.pieceBitboard(currentColor, Piece::KNIGHT);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);

    while (currentKnights) {
      int fromIndex = popLowestBit(currentKnights);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::knightAttacks(fromSquare) & enemyColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);
        addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
      }
    }
  }

  static void generateBishopCaptures(Board& board, Move* moves, int& moveCount) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentBishops = board.pieceBitboard(currentColor, Piece::BISHOP);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Bitboard occupiedBitboard = board.allBitboard();

    while (currentBishops) {
      int fromIndex = popLowestBit(currentBishops);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::bishopAttacks(fromSquare, occupiedBitboard) & enemyColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);
        addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
      }
    }
  }

  static void generateRookCaptures(Board& board, Move* moves, int& moveCount) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentRooks = board.pieceBitboard(currentColor, Piece::ROOK);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Bitboard occupiedBitboard = board.allBitboard();

    while (currentRooks) {
      int fromIndex = popLowestBit(currentRooks);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::rookAttacks(fromSquare, occupiedBitboard) & enemyColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);
        addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
      }
    }
  }

  static void generateQueenCaptures(Board& board, Move* moves, int& moveCount) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard currentQueens = board.pieceBitboard(currentColor, Piece::QUEEN);
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Bitboard occupiedBitboard = board.allBitboard();

    while (currentQueens) {
      int fromIndex = popLowestBit(currentQueens);
      Square fromSquare = intToSquare(fromIndex);
      Bitboard targets = Attacks::queenAttacks(fromSquare, occupiedBitboard) & enemyColorBitboard;

      while (targets) {
        int toIndex = popLowestBit(targets);
        Square toSquare = intToSquare(toIndex);
        addMoveIfLegal(board, moves, moveCount, Move(fromSquare, toSquare, MoveFlag::CAPTURE));
      }
    }
  }

  static void generateKingCaptures(Board& board, Move* moves, int& moveCount) {
    Color currentColor = board.currentTurn();
    Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard enemyColorBitboard = board.colorBitboard(enemyColor);
    Square kingSquare = board.findKing(currentColor);

    Bitboard targets = Attacks::kingAttacks(kingSquare) & enemyColorBitboard;

    while (targets) {
      int toIndex = popLowestBit(targets);
      Square toSquare = intToSquare(toIndex);
      addMoveIfLegal(board, moves, moveCount, Move(kingSquare, toSquare, MoveFlag::CAPTURE));
    }
  }


  

  void generateMoves(Board& board, Move* moves, int& moveCount) {
    moveCount = 0;
    pinnedBitboard = pinnedPieces(board);
    inCheck = board.isInCheck(board.currentTurn());

    generatePawnMoves(board, moves, moveCount);
    generateKnightMoves(board, moves, moveCount);
    generateBishopMoves(board, moves, moveCount);
    generateRookMoves(board, moves, moveCount);
    generateQueenMoves(board, moves, moveCount);
    generateKingMoves(board, moves, moveCount);
  }

  void generateCaptures(Board& board, Move* moves, int& moveCount) {
    moveCount = 0;
    pinnedBitboard = pinnedPieces(board);
    inCheck = board.isInCheck(board.currentTurn());

    generatePawnCaptures(board, moves, moveCount);
    generateKnightCaptures(board, moves, moveCount);
    generateBishopCaptures(board, moves, moveCount);
    generateRookCaptures(board, moves, moveCount);
    generateQueenCaptures(board, moves, moveCount);
    generateKingCaptures(board, moves, moveCount);
  }
}