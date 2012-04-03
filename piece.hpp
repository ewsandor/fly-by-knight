//piece.hpp
//flyByKnight - Chess Engine
//Created by Edward Sandor 2011-2012.
//Read the README for more.

#ifndef _PIECE_INCLUDED_
#define _PIECE_INCLUDED_

#include <vector>
class Game;

class Piece{
protected:
  int value;
  int color;
  int X;
  int Y;
  Game * gm;


public:
  bool hasMoved;

  Piece(int col, Game * game, int val);
  int getColor();
  int getValue();
  int getX();
  int getY();
  void setLocation(int x, int y);
  bool virtual isLeagal(int x, int y) = 0;
  bool virtual causesCheck(int x, int y);
  std::string virtual toString() = 0;
  std::string virtual toShortString() = 0;
  bool virtual move(int x, int y);
  void virtual getMoves(std::vector<int> &moves) = 0;
  Game * getGame();

  static bool onBoard(int x, int y);
};

class Pawn : public Piece{
private:
  
public:

  Pawn(int col, Game * game);
  bool isLeagal(int x, int y);
  std::string toString();
  std::string toShortString();
  bool causesCheck(int x, int y);
  bool move(int x, int y);  
  void getMoves(std::vector<int> &moves);
};

class Knight : public Piece{
public:
  Knight(int col, Game * game);
  bool isLeagal(int x, int y);
  std::string toString();
  std::string toShortString();
  void getMoves(std::vector<int> &moves);
};

class Bishop : public Piece{
public:
  Bishop(int col, Game * game);
  bool isLeagal(int x, int y);
  std::string toString();
  std::string toShortString();
  void getMoves(std::vector<int> &moves);
};

class Rook : public Piece{
private:
public:
  Rook(int col, Game * game);
  bool isLeagal(int x, int y);
  bool getHasMoved();
  std::string toString();
  std::string toShortString();
  bool move(int x, int y);
  void getMoves(std::vector<int> &moves);
};

class Queen : public Piece{
public:
  Queen(int col, Game * game);
  bool isLeagal(int x, int y);  
  std::string toString();
  std::string toShortString();
  void getMoves(std::vector<int> &moves);
};

class King : public Piece{
private:
public:
  King(int col, Game * game);
  bool isLeagal(int x, int y);
  std::string toString();
  std::string toShortString();
  bool move(int x, int y);
  void getMoves(std::vector<int> &moves);

  bool castleLegal(int x, int y);
};

#endif
