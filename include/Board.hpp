#ifndef BOARD_HPP
#define BOARD_HPP

#include "Bitboard.hpp"
#include <array>
#include <string>

constexpr int CASTLE_WHITE_KING = 1;
constexpr int CASTLE_WHITE_QUEEN = 2;
constexpr int CASTLE_BLACK_KING = 4;
constexpr int CASTLE_BLACK_QUEEN = 8;
constexpr int CASTLE_ALL = 15;




class Board {
public:
  Board();
  ~Board();

  void setFromFEN(const std::string& fen);
  std::string toFEN() const;




  Bitboard pieceBitboard(Color color, Piece piece) const;
  Bitboard colorBitboard(Color color) const;
  Bitboard allBitboard() const;




  Piece pieceOn(Square square) const;
  Color colorOn(Square square) const;




  Color currentTurn() const;
  int castlingRights() const;
  Square enPassantSquare() const;
  int halfmoveClock() const;
  int fullmoveNumber() const;




  void putPiece(Color color, Piece piece, Square square);
  void removePiece(Square square);
  void flipCurrentTurn();




  Square findKing(Color color) const;




  bool isAttacked(Square square, Color attackingColor) const;
  bool isInCheck(Color color) const;




  void print() const;

private:
  std::array<std::array<Bitboard, NUM_PIECES>, NUM_COLORS> pieceBitboards_;
  std::array<Bitboard, NUM_COLORS> colorBitboards_;

  Color currentTurn_;
  int castlingRights_;
  Square enPassantSquare_;
  int halfmoveClock_;
  int fullmoveNumber_;

  std::array<Piece, NUM_SQUARES> pieceMailbox_;
  std::array<Color, NUM_SQUARES> colorMailbox_;
};

#endif