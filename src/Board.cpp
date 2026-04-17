#include "Board.hpp"
#include "Attacks.hpp"
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




void Board::putPiece(Color color, Piece piece, Square square) {
  int colorIndex = colorToInt(color);
  int pieceIndex = pieceToInt(piece);
  int squareIndex = squareToInt(square);

  setBit(pieceBitboards_[colorIndex][pieceIndex], square);
  setBit(colorBitboards_[colorIndex], square);

  pieceMailbox_[squareIndex] = piece;
  colorMailbox_[squareIndex] = color;
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
}

void Board::flipCurrentTurn() {
  currentTurn_ = (currentTurn_ == Color::WHITE) ? Color::BLACK : Color::WHITE;
}

void Board::setCastlingRights(int rights) {
  castlingRights_ = rights;
}

void Board::setEnPassantSquare(Square square) {
  enPassantSquare_ = square;
}

void Board::setHalfmoveClock(int count) {
  halfmoveClock_ = count;
}




Square Board::findKing(Color color) const {
  Bitboard kingBitboard = pieceBitboards_[colorToInt(color)][pieceToInt(Piece::KING)];

  if (kingBitboard == 0) {
    return Square::NONE;
  }

  return intToSquare(lowestBit(kingBitboard));
}




bool Board:: isAttacked(Square square, Color attackingColor) const {
  int squareIndex = squareToInt(square);
  int attackerIndex = colorToInt(attackingColor);
  Bitboard occupiedBitboard = allBitboard();

  // Pawn attacks
  Color defendingColor = (attackingColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
  if (Attacks::pawnAttacks[colorToInt(defendingColor)][squareIndex] & pieceBitboards_[attackerIndex][pieceToInt(Piece::PAWN)]) {
    return true;
  }

  // Knight attacks
  if (Attacks::knightAttacks[squareIndex] & pieceBitboards_[attackerIndex][pieceToInt(Piece::KNIGHT)]) {
    return true;
  }

  // Bishop/queen attacks (diagonal)
  Bitboard bishopsAndQueens = pieceBitboards_[attackerIndex][pieceToInt(Piece::BISHOP)]
    | pieceBitboards_[attackerIndex][pieceToInt(Piece::QUEEN)];
  if (Attacks::bishopAttacks(occupiedBitboard, square) & bishopsAndQueens) {
    return true;
  }

  // Rook/queen attacks (straight)
  Bitboard rooksAndQueens = pieceBitboards_[attackerIndex][pieceToInt(Piece::ROOK)]
    | pieceBitboards_[attackerIndex][pieceToInt(Piece::QUEEN)];
  if (Attacks::rookAttacks(occupiedBitboard, square) & rooksAndQueens) {
    return true;
  }

  // King attacks
  if (Attacks::kingAttacks[squareIndex] & pieceBitboards_[attackerIndex][pieceToInt(Piece::KING)]) {
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