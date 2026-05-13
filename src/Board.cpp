#include "Board.hpp"
#include "Attacks.hpp"
#include "Zobrist.hpp"
#include <iostream>
#include <sstream>

static const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
static const char* PIECE_CHARACTERS_LOWER = "pnbrqk";
static const char* PIECE_CHARACTERS_UPPER = "PNBRQK";

Board::Board()
  : pieceBitboards_{}
  , colorBitboards_{}
  , currentTurn_(Color::WHITE)
  , castlingRights_(CASTLE_ALL)
  , enPassantSquare_(Square::NONE)
  , halfmoveClock_(0)
  , fullmoveNumber_(1)
  , hash_(0)
  , pieceMailbox_{}
  , colorMailbox_{}
{
  pieceMailbox_.fill(Piece::NONE);
  colorMailbox_.fill(Color::WHITE);
  setFromFEN(START_FEN);
}

Board::~Board() {

}




void Board::setFromFEN(const std::string& fen) {
  pieceBitboards_ = {};
  colorBitboards_ = {};
  pieceMailbox_.fill(Piece::NONE);
  colorMailbox_.fill(Color::WHITE);

  std::istringstream stream(fen);
  std::string pieceLayoutString, currentTurnString, castlingString, enPassantString;
  int halfmoveClockValue, fullmoveNumberValue;

  stream >> pieceLayoutString >> currentTurnString >> castlingString >> enPassantString
          >> halfmoveClockValue >> fullmoveNumberValue;

  int rank = 7;
  int file = 0;

  for (char character : pieceLayoutString) {
    if (character == '/') {
      rank--;
      file = 0;
    } else if (character >= '1' && character <= '8') {
      file += (character - '0');
    } else {
      Color color = (character >= 'A' && character <= 'Z') ? Color::WHITE : Color::BLACK;
      Piece piece = Piece::NONE;

      switch (tolower(character)) {
        case 'p':
          piece = Piece::PAWN;
          break;
        case 'n':
          piece = Piece::KNIGHT;
          break;
        case 'b':
          piece = Piece::BISHOP;
          break;
        case 'r':
          piece = Piece::ROOK;
          break;
        case 'q':
          piece = Piece::QUEEN; 
          break;
        case 'k':
          piece = Piece::KING;
          break;
        default:
          break;
      }

      if (piece != Piece::NONE) {
        Square square = makeSquare(file, rank);
        putPiece(color, piece, square);
      }

      file++;
    }
  }

  currentTurn_ = (currentTurnString == "w") ? Color::WHITE : Color::BLACK;

  castlingRights_ = 0;
  for (char character : castlingString) {
    switch (character) {
      case 'K':
        castlingRights_ |= CASTLE_WHITE_KING; 
        break;
      case 'Q':
        castlingRights_ |= CASTLE_WHITE_QUEEN;
        break;
      case 'k':
        castlingRights_ |= CASTLE_BLACK_KING;
        break;
      case 'q':
        castlingRights_ |= CASTLE_BLACK_QUEEN;
        break;
      default:
        break;
    }
  }

  if (enPassantString == "-") {
    enPassantSquare_ = Square::NONE;
  } else {
    enPassantSquare_ = stringToSquare(enPassantString);
  }

  halfmoveClock_ = halfmoveClockValue;
  fullmoveNumber_ = fullmoveNumberValue;

  hash_ = 0;
  for (int color = 0; color < NUM_COLORS; color++) {
    for (int piece = 0; piece < NUM_PIECES; piece++) {
      Bitboard pieces = pieceBitboards_[color][piece];

      while (pieces) {
        int square = popLowestBit(pieces);
        hash_ ^= Zobrist::pieceKeys[color][piece][square];
      }
    }
  }
 
  hash_ ^= Zobrist::castlingKeys[castlingRights_];
 
  if (enPassantSquare_ != Square::NONE) {
    hash_ ^= Zobrist::enPassantKeys[fileOf(enPassantSquare_)];
  }
 
  if (currentTurn_ == Color::BLACK) {
    hash_ ^= Zobrist::sideKey;
  }
}




std::string Board::toFEN() const {
  std::string fen;

  for (int rank = 7; rank >= 0; rank--) {
    int emptyCount = 0;
    
    for (int file = 0; file < 8; file++) {
      Square square = makeSquare(file, rank);
      int squareIndex = squareToInt(square);
      Piece piece = pieceMailbox_[squareIndex];

      if (piece == Piece::NONE) {
        emptyCount++;
      } else {
        if (emptyCount > 0) {
          fen += std::to_string(emptyCount);
          emptyCount = 0;
        }

        char character = (colorMailbox_[squareIndex] == Color::WHITE)
          ? PIECE_CHARACTERS_UPPER[pieceToInt(piece)]
          : PIECE_CHARACTERS_LOWER[pieceToInt(piece)];
        fen += character;
      }
    }

    if (emptyCount > 0) {
      fen += std::to_string(emptyCount);
    }

    if (rank > 0) {
      fen += '/';
    }
  }

  fen += (currentTurn_ == Color::WHITE) ? " w " : " b ";

  if (castlingRights_ == 0) {
    fen += '-';
  } else {
    if (castlingRights_ & CASTLE_WHITE_KING) {
      fen += 'K';
    }

    if (castlingRights_ & CASTLE_WHITE_QUEEN) {
      fen += 'Q';
    }

    if (castlingRights_ & CASTLE_BLACK_KING) {
      fen += 'k';
    }

    if (castlingRights_ & CASTLE_BLACK_QUEEN) {
      fen += 'q';
    }
  }

  fen += ' ';
  if (enPassantSquare_ == Square::NONE) {
    fen += '-';
  } else {
    fen += squareToString(enPassantSquare_);
  }

  fen += ' ' + std::to_string(halfmoveClock_);
  fen += ' ' + std::to_string(fullmoveNumber_);

  return fen;
}




Bitboard Board::pieceBitboard(Color color, Piece piece) const {
  return pieceBitboards_[colorToInt(color)][pieceToInt(piece)];
}

Bitboard Board::colorBitboard(Color color) const {
  return colorBitboards_[colorToInt(color)];
}

Bitboard Board::allBitboard() const {
  return colorBitboards_[0] | colorBitboards_[1];
}




Piece Board::pieceOn(Square square) const {
  return pieceMailbox_[squareToInt(square)];
}

Color Board::colorOn(Square square) const {
  return colorMailbox_[squareToInt(square)];
}




Color Board::currentTurn() const {
  return currentTurn_;
}

int Board::castlingRights() const {
  return castlingRights_;
}

Square Board::enPassantSquare() const {
  return enPassantSquare_;
}

int Board::halfmoveClock() const {
  return halfmoveClock_;
}

int Board::fullmoveNumber() const {
  return fullmoveNumber_;
}

uint64_t Board::hash() const {
  return hash_;
}




void Board::putPiece(Color color, Piece piece, Square square) {
  int colorIndex = colorToInt(color);
  int pieceIndex = pieceToInt(piece);
  int squareIndex = squareToInt(square);

  setBit(pieceBitboards_[colorIndex][pieceIndex], square);
  setBit(colorBitboards_[colorIndex], square);

  pieceMailbox_[squareIndex] = piece;
  colorMailbox_[squareIndex] = color;

  hash_ ^= Zobrist::pieceKeys[colorIndex][pieceIndex][squareIndex];
}

void Board::removePiece(Square square) {
  int squareIndex = squareToInt(square);
  Color color = colorMailbox_[squareIndex];
  Piece piece = pieceMailbox_[squareIndex];

  if (piece == Piece::NONE) {
    return;
  }

  int colorIndex = colorToInt(color);
  int pieceIndex = pieceToInt(piece);

  clearBit(pieceBitboards_[colorIndex][pieceIndex], square);
  clearBit(colorBitboards_[colorIndex], square);

  pieceMailbox_[squareIndex] = Piece::NONE;

  hash_ ^= Zobrist::pieceKeys[colorIndex][pieceIndex][squareIndex]; 
}

void Board::flipCurrentTurn() {
  currentTurn_ = (currentTurn_ == Color::WHITE) ? Color::BLACK : Color::WHITE;
  hash_ ^= Zobrist::sideKey;
}

void Board::setCastlingRights(int rights) {
  hash_ ^= Zobrist::castlingKeys[castlingRights_];
  castlingRights_ = rights;
  hash_ ^= Zobrist::castlingKeys[castlingRights_];

}

void Board::setEnPassantSquare(Square square) {
  if (enPassantSquare_ != Square::NONE) {
    hash_ ^= Zobrist::enPassantKeys[fileOf(enPassantSquare_)];
  }

  enPassantSquare_ = square;

  if (enPassantSquare_ != Square::NONE) {
    hash_ ^= Zobrist::enPassantKeys[fileOf(enPassantSquare_)];
  }
}

void Board::setHalfmoveClock(int count) {
  halfmoveClock_ = count;
}




MoveState Board::makeMove(const Move& move) {
  MoveState state;
  state.capturedPiece = pieceMailbox_[squareToInt(move.to)];
  state.castlingRights = castlingRights_;
  state.enPassantSquare = enPassantSquare_;
  state.halfmoveClock = halfmoveClock_;
  state.hash = hash_;

  Color currentColor = currentTurn_;
  Piece movingPiece = pieceMailbox_[squareToInt(move.from)];

  // Captures
  if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::EN_PASSANT)) {
    Square enPassantCaptureSquare = (currentColor == Color::WHITE)
      ? intToSquare(squareToInt(move.to) - 8)
      : intToSquare(squareToInt(move.to) + 8);
    removePiece(enPassantCaptureSquare);
  }

  if (pieceMailbox_[squareToInt(move.to)] != Piece::NONE) {
    removePiece(move.to);
  }

  // Moves
  removePiece(move.from);

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

    putPiece(currentColor, promotionPiece, move.to);
  } else {
    putPiece(currentColor, movingPiece, move.to);
  }

  // Castling
  if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::CASTLE_KING)) {
    Square rookFrom = (currentColor == Color::WHITE) ? Square::H1 : Square::H8;
    Square rookTo = (currentColor == Color::WHITE) ? Square::F1 : Square::F8;
    removePiece(rookFrom);
    putPiece(currentColor, Piece::ROOK, rookTo);
  } else if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::CASTLE_QUEEN)) {
    Square rookFrom = (currentColor == Color::WHITE) ? Square::A1 : Square::A8;
    Square rookTo = (currentColor == Color::WHITE) ? Square::D1 : Square::D8;
    removePiece(rookFrom);
    putPiece(currentColor, Piece::ROOK, rookTo);
  }

  // En passant square
  if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::DOUBLE_PAWN)) {
    int enPassantIndex = (squareToInt(move.from) + squareToInt(move.to)) / 2;
    setEnPassantSquare(intToSquare(enPassantIndex));
  } else {
    setEnPassantSquare(Square::NONE);
  }

  // Castling rights
  int castlingRights = castlingRights_;
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

  setCastlingRights(castlingRights);

  // Halfmove clock
  bool pawnMove = (movingPiece == Piece::PAWN);
  bool capture = (state.capturedPiece != Piece::NONE) || (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::EN_PASSANT));
  if (pawnMove || capture) {
    halfmoveClock_ = 0;
  } else  {
    halfmoveClock_++;
  }

  // Switch turn
  flipCurrentTurn();

  return state;
}

void Board::undoMove(const Move& move, const MoveState& state) {
  flipCurrentTurn();

  Color currentColor = currentTurn_;
  Color enemyColor = (currentColor == Color::WHITE) ? Color::BLACK : Color::WHITE;

  // Castling
  if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::CASTLE_KING)) {
    Square rookFrom = (currentColor == Color::WHITE) ? Square::H1 : Square::H8;
    Square rookTo = (currentColor == Color::WHITE) ? Square::F1 : Square::F8;
    removePiece(rookTo);
    putPiece(currentColor, Piece::ROOK, rookFrom);
  } else if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::CASTLE_QUEEN)) {
    Square rookFrom = (currentColor == Color::WHITE) ? Square::A1 : Square::A8;
    Square rookTo = (currentColor == Color::WHITE) ? Square::D1 : Square::D8;
    removePiece(rookTo);
    putPiece(currentColor, Piece::ROOK, rookFrom);
  }

  // Moves
  Piece movingPiece = pieceMailbox_[squareToInt(move.to)];
  removePiece(move.to);

  Piece originalPiece = move.isPromotion() ? Piece::PAWN : movingPiece;
  putPiece(currentColor, originalPiece, move.from);

  // Captures
  if (state.capturedPiece != Piece::NONE) {
    putPiece(enemyColor, state.capturedPiece, move.to);
  }

  if (static_cast<int>(move.flag) & static_cast<int>(MoveFlag::EN_PASSANT)) {
    Square enPassantCaptureSquare = (currentColor == Color::WHITE)
      ? intToSquare(squareToInt(move.to) - 8)
      : intToSquare(squareToInt(move.to) + 8);
    putPiece(enemyColor, Piece::PAWN, enPassantCaptureSquare);
  }

  // Castling rights
  castlingRights_ = state.castlingRights;
  // En passant square
  enPassantSquare_ = state.enPassantSquare;
  // Halfmove clock
  halfmoveClock_ = state.halfmoveClock;
  // Hash
  hash_ = state.hash;
}




Square Board::findKing(Color color) const {
  Bitboard kingBitboard = pieceBitboards_[colorToInt(color)][pieceToInt(Piece::KING)];

  if (kingBitboard == 0) {
    return Square::NONE;
  }

  return intToSquare(lowestBit(kingBitboard));
}




bool Board::isAttacked(Square square, Color attackingColor) const {
  int attackerIndex = colorToInt(attackingColor);
  Bitboard occupiedBitboard = allBitboard();

  // Pawn attacks
  Color defendingColor = (attackingColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
  if (Attacks::pawnAttacks(defendingColor, square) & pieceBitboards_[attackerIndex][pieceToInt(Piece::PAWN)]) {
    return true;
  }

  // Knight attacks
  if (Attacks::knightAttacks(square) & pieceBitboards_[attackerIndex][pieceToInt(Piece::KNIGHT)]) {
    return true;
  }

  // Bishop/queen attacks (diagonal)
  Bitboard bishopsAndQueens = pieceBitboards_[attackerIndex][pieceToInt(Piece::BISHOP)]
    | pieceBitboards_[attackerIndex][pieceToInt(Piece::QUEEN)];
  if (Attacks::bishopAttacks(square, occupiedBitboard) & bishopsAndQueens) {
    return true;
  }

  // Rook/queen attacks (straight)
  Bitboard rooksAndQueens = pieceBitboards_[attackerIndex][pieceToInt(Piece::ROOK)]
    | pieceBitboards_[attackerIndex][pieceToInt(Piece::QUEEN)];
  if (Attacks::rookAttacks(square, occupiedBitboard) & rooksAndQueens) {
    return true;
  }

  // King attacks
  if (Attacks::kingAttacks(square) & pieceBitboards_[attackerIndex][pieceToInt(Piece::KING)]) {
    return true;
  }

  return false;
}

bool Board::isInCheck(Color color) const {
  Square kingSquare = findKing(color);
  Color attackingColor = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;

  return isAttacked(kingSquare, attackingColor);
}




void Board::print() const {
  for (int rank = 7; rank >= 0; rank--) {
    std::cout << (rank + 1) << "  ";

    for (int file = 0; file < 8; file++) {
      Square square = makeSquare(file, rank);
      int squareIndex = squareToInt(square);
      Piece piece = pieceMailbox_[squareIndex];

      if (piece == Piece::NONE) {
        std::cout << ". ";
      } else {
        char character = (colorMailbox_[squareIndex] == Color::WHITE)
          ? PIECE_CHARACTERS_UPPER[pieceToInt(piece)]
          : PIECE_CHARACTERS_LOWER[pieceToInt(piece)];

        std::cout << character << " "; 
      }
    }

    std::cout << "\n";
  }

  std::cout << "   a b c d e f g h\n";
  std::cout << "\nFEN: " << toFEN() << "\n";
}