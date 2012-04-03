//game.hpp
//flyByKnight - Chess Engine
//Created by Edward Sandor 2011-2012.

#ifndef _GAME_INCLUDED_
#define _GAME_INCLUDED_
#define WHITE 0
#define BLACK 1
#define NONE  2
#include <stdlib.h>
#include <vector>

class Board;
class Piece;

struct change_t{
  Piece * moded;
  int oldLoc;
  int newLoc;
  bool captured;
  bool firstMove;
  bool ep;
};

class Game{
private:  
  Piece * whiteKing;
  Piece * blackKing;
  int turn;
  unsigned int place;
  Board * board;

public:
  std::vector<std::vector<change_t> > changes;
  Piece * enpasantable;
  int playAs;

  Game();

  void clear();
  void resetGame();
  void setupBoard();
  void changeTurn();
  int getTurn();
  Board * getBoard();
  Piece * getKing(int color);
  void setKing(int color, Piece * k);
  bool move(std::string str);
  bool move(int from, int to);
  bool inCheck(Piece * checkie);
  bool inCheckmate(Piece * matey);
  bool inStalemate(Piece * matey);
  bool causesCheck(Piece * piece, int mov);
  bool modSquare(std::string mod, int color);
  bool endGame();
  double evaluateBoard();
  std::string chooseMove();
  void addTurn();
  void removeTurn();
  void addChange(change_t change);
  void clearEnd();
  bool moveBack();
  bool moveBack(int steps);
  bool moveForeward();
  bool moveForeward(int steps);
  
private:
  bool inMate(Piece * matey);
};
#endif
