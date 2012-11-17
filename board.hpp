//board.hpp
//Fly By Knight - Chess Engine
//Created by Edward Sandor 2011-2012.
//Read the README for more.

#ifndef _BOARD_INCLUDED_
#define _BOARD_INCLUDED_

#include <vector>
class Piece;
class Game;

class Board{
public:
  Piece * pieces[8][8];
  Game * gm;

  Board(Game * g);

  static int toInts(std::string str);
  static std::string toStr(int mov);
  static bool moveFormat(std::string str);
  static double squareVal(int x, int y);

  void clearBoard();
  void placePiece(Piece * piece, int x, int y);
  void removePiece(int x, int y);
  Piece * getPiece(int x, int y);
  void printBoard();
  void clearSpace(int x, int y);

  void getPieces(int color, std::vector<Piece *> &found);
  bool promotePawn(char newPiece);
};

#endif
