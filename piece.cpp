//piece.cpp
//Fly By Knight - Chess Engine
//Created by Edward Sandor 2011-2012.
//Read the README for more.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////BEGIN///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include "piece.hpp"
#include "game.hpp"
#include "board.hpp"
#include "moveTree.hpp"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////PIECE///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Piece::Piece(int col, Game * game, int val){
  X = 0;
  Y = 0;
  color = col;
  value = val;
  gm = game;
  hasMoved = false;
}
int Piece::getColor(){
  return color;
}
int Piece::getValue(){
  return value;
}
int Piece::getX(){
  return X;
}
int Piece::getY(){
  return Y;
}
void Piece::setLocation(int x, int y){
  X = x;
  Y = y;
}
bool Piece::causesCheck(int x, int y){
  Piece * oldPiece = gm->getBoard()->getPiece(x, y);

  int oldLoc = X*10 + Y;
  
  gm->getBoard()->pieces[oldLoc/10][oldLoc%10]->setLocation(x, y);
  gm->getBoard()->pieces[x][y] = gm->getBoard()->pieces[oldLoc/10][oldLoc%10];
  gm->getBoard()->pieces[oldLoc/10][oldLoc%10] = NULL;
  
  bool check = gm->inCheck(gm->getKing(color));
  
  gm->getBoard()->pieces[x][y]->setLocation(oldLoc/10, oldLoc%10);
  gm->getBoard()->pieces[oldLoc/10][oldLoc%10] = gm->getBoard()->pieces[x][y];
  gm->getBoard()->pieces[x][y] = oldPiece;    

  return check;
}
bool Piece::move(int x, int y){

  if(x < 0 || x > 7 || y < 0 || y > 7 || color != gm->getTurn() || !isLeagal(x,y))
    return false;

  gm->addTurn();
  // tmp(0);

  change_t c;
  c.moded = this;
  c.oldLoc = X*10 + Y;
  c.newLoc = x*10 +y;
  c.captured = false;
  c.firstMove = hasMoved?false:true;
	hasMoved = true;
  c.ep = false;
  gm->addChange(c);
  

  int oldLoc = (X*10 + Y);
  Piece * oldPiece = gm->getBoard()->getPiece(x,y);

  gm->getBoard()->pieces[x][y] = this;
  
  setLocation(x,y);

  gm->getBoard()->pieces[oldLoc / 10][oldLoc % 10] = NULL;
  
  if(gm->inCheck(gm->getKing(color))){
    this->gm->getBoard()->pieces[oldLoc / 10][oldLoc % 10] = this;
    setLocation(oldLoc / 10, oldLoc % 10);
    
    this->gm->getBoard()->pieces[x][y] = oldPiece;
    
	gm->moveTree->current = gm->moveTree->current->parent;
	gm->moveTree->current->choices.pop_back();
	  /*Move * tmp = gm->moveTree->current;
	  gm->moveBack();
	  gm->moveTree->current->choices.pop_back();*/

    //cout << "CHECK" << endl;
    
    return false;
  }
  
  if(oldPiece != NULL){
    change_t c;
    c.moded = oldPiece;
    c.oldLoc = x*10 + y;
    c.newLoc = x*10 + y;
    c.captured = true;
    c.firstMove = false;
    c.ep = true;

    gm->addChange(c);
  }
  
  return true;
}
Game * Piece::getGame(){
  return gm;
}
bool Piece::onBoard(int x, int y){
  return (x >= 0 && x < 8 && y >=0 && y <8);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////PAWN/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Pawn::Pawn(int col, Game * game) : Piece(col, game, 1){
  hasMoved = false;
}
bool Pawn::isLeagal(int x, int y){
  int colAdjust = (color == WHITE ? 1 : -1);

  if(y ==  Y + colAdjust){
    if(x == X && gm->getBoard()->getPiece(x,y) == NULL)
      return true;
    else if(abs(x - X) == 1 && gm->getBoard()->getPiece(x,y) != NULL && gm->getBoard()->getPiece(x,y)->getColor() != color){
      return true;
    }
    else if(abs(x - X) == 1 && gm->enpasantable != NULL && gm->enpasantable->getColor() != color && gm->enpasantable->getX() == x && gm->enpasantable->getY() == Y)
      return true;
    else
      return false;
  }
  else if (y == Y + (colAdjust * 2))
    return (!hasMoved && x == X && gm->getBoard()->getPiece(x,y - colAdjust) == NULL && gm->getBoard()->getPiece(x,y) == NULL);
  else
    return false;
}
string Pawn::toString(){

  string str = (getColor()==WHITE? "White":"Black"); 
  return str + " Pawn";
}
string Pawn::toShortString(){
  /*string str = (getColor()==WHITE? "W":"B"); 
  return str + "P";*/
  return (getColor()==WHITE? "P":"p");
}
bool Pawn::causesCheck(int x, int y){
  Piece * epv = gm->enpasantable;
  bool epd = (abs(x-X) == 1 && abs(y-Y) == 1 && epv != NULL && epv->getX() == x && gm->getBoard()->pieces[x][y] == NULL);

  int eX,eY = -1;
  if(epd){
    eX = epv->getX();
    eY = epv->getY();
  }

  if(epd)
    gm->getBoard()->pieces[eX][eY] = NULL;

  bool ck = Piece::causesCheck(x,y);
 
  if(epd)
    gm->getBoard()->pieces[eX][eY] = epv;

  return ck;
}
bool Pawn::move(int x, int y){
  
  bool ep = abs(y - Y) == 2;
  Piece * epv = gm->enpasantable;
  bool epd = (abs(x-X) == 1 && abs(y-Y) == 1 && epv != NULL && epv->getX() == x && gm->getBoard()->pieces[x][y] == NULL);
  
  int eX,eY = -1;
  if(epd){
    eX = epv->getX();
    eY = epv->getY();
  }

  if(epd)
    gm->getBoard()->pieces[eX][eY] = NULL;
    
  if(Piece::move(x,y)){
    if(ep){
		change_t * c = &gm->moveTree->current->changes.back();
      c->ep = true;
      gm->enpasantable = this;
    }  
    else if(epd){
      change_t c;
      c.moded = epv;
      c.oldLoc = epv->getX() * 10 + epv->getY();
      c.newLoc = c.oldLoc;
      c.captured = true;
      c.firstMove = false;
      c.ep = false;

      gm->addChange(c);
    }
    return true;
  }
  
  if(epd)
    gm->getBoard()->pieces[eX][eY] = epv;
  return false;
}
void Pawn::getMoves(vector<string> &moves){

  int currentLoc = X*1000 + Y*100;
  int rowDisplace = -1;
  for(int i = 0; i < 2; i++){
    for(int j = -1; j <= 1; j++){
      int x = X+j;
      int y = Y+rowDisplace;
      
      if(Piece::onBoard(x,y) && isLeagal(x,y))
		if(y != 0 && y != 7)
			moves.push_back(Board::toStr(currentLoc+x*10+y));
		else{
			moves.push_back(Board::toStr(currentLoc+x*10+y) + "q");
			moves.push_back(Board::toStr(currentLoc+x*10+y) + "r");
			moves.push_back(Board::toStr(currentLoc+x*10+y) + "b");
			moves.push_back(Board::toStr(currentLoc+x*10+y) + "n");
		}
    }
    rowDisplace = 1;
  }
  int y = Y + 2;
  if(Piece::onBoard(X,y) && isLeagal(X,y))
	moves.push_back(Board::toStr(currentLoc+X*10+y));
  y = Y - 2;
  if(Piece::onBoard(X,y) && isLeagal(X,y))
	moves.push_back(Board::toStr(currentLoc+X*10+y));

  return;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////KNIGHT/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Knight::Knight(int col, Game * game) : Piece(col, game, 3){}
bool Knight::isLeagal(int x, int y){

  if(x==X || y == Y)
    return false;

  return ((abs(( y-Y)/( x-X)) == 2  || abs(( x-X)/( y-Y)) == 2) 
  &&  (abs(x-X) == 1 || abs(y-Y) == 1) 
  && (gm->getBoard()->getPiece(x,y) == NULL || gm->getBoard()->getPiece(x,y)->getColor() != color));
}
string Knight::toString(){

  string str = (getColor()==WHITE? "White":"Black"); 
  return str + " Knight";
}
string Knight::toShortString(){
  /*string str = (getColor()==WHITE? "W":"B"); 
    return str + "N";*/
  return (getColor()==WHITE? "N":"n");
}
void Knight::getMoves(vector<string> &moves){
  
  int oldloc = X * 1000 + Y * 100;

  for(int i = -2; i <= 2; i++){
    if(i==0)      continue;
    for(int j = -2; j <= 2; j++){
		int x = X + i;
		int y = Y + j;
      if(j == 0 || i == j)	continue;
      if(onBoard(x,y) && isLeagal(x,y))
	moves.push_back(Board::toStr(oldloc + x * 10 + y));
    }
  }

  return;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////BISHOP////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Bishop::Bishop(int col, Game * game) : Piece(col, game, 3){}
bool Bishop::isLeagal(int x, int y){

  if(x==X || abs(y-Y) != abs(x-X))
    return false;

  int hortmod = (x<X)?-1:1;
  int vertmod = (y<Y)?-1:1;

  int tx = X + hortmod;
  int ty = Y + vertmod;

  while(tx != x && ty != y){
    if(gm->getBoard()->getPiece(tx,ty) != NULL)
      return false;
    tx += hortmod;
    ty += vertmod;
  }
  
  return (gm->getBoard()->getPiece(x,y) == NULL || gm->getBoard()->getPiece(x,y)->getColor() != color);
}
string Bishop::toString(){
  
  string str = (getColor()==WHITE? "White":"Black"); 
  return str + " Bishop";
}
string Bishop::toShortString(){
  /*string str = (getColor()==WHITE? "W":"B"); 
  return str + "B";*/

  return (getColor()==WHITE? "B":"b");
}
void Bishop::getMoves(vector<string> &moves){
  
  int oldloc = X * 1000 + Y * 100;

  for(int i = 1; i < 8; i++){
    int x = X + i;
    int y = Y + i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X - i;
    y = Y + i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X + i;
    y = Y - i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X - i;
    y = Y - i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
  }

  return;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////ROOK//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Rook::Rook(int col, Game * game) : Piece(col, game, 5){
  hasMoved = false;
}
bool Rook::isLeagal(int x, int y){
  
  if(x != X && y != Y)
    return false;
  
  int hortmod = (x==X)? 0:((x<X)?-1:1);
  int vertmod = (y==Y)? 0:((y<Y)?-1:1);
  
  int tx = X + hortmod;
  int ty = Y + vertmod;
      
  while(tx != x || ty != y){
    if(gm->getBoard()->getPiece(tx,ty) != NULL)
      return false;
    tx += hortmod;
    ty += vertmod;
  }
  
  return (gm->getBoard()->getPiece(x,y) == NULL || gm->getBoard()->getPiece(x,y)->getColor() != color);
}
bool Rook::getHasMoved(){
  return hasMoved;
}
string Rook::toString(){
  
  string str = (getColor()==WHITE? "White":"Black"); 
  return str + " Rook";
}
string Rook::toShortString(){
  /*string str = (getColor()==WHITE? "W":"B"); 
  return str + "R";*/

  return (getColor()==WHITE? "R":"r");
}
bool Rook::move(int x, int y){

  if(Piece::move(x,y)){
    if(!hasMoved){
		change_t * c = &gm->moveTree->current->changes.back();
      c->firstMove = true;
      hasMoved = true;
    }
    return true;
  }
  else
    return false;
}
void Rook::getMoves(vector<string> &moves){
	  int oldloc = X * 1000 + Y * 100;

  for(int i = 1; i < 8; i++){
    int x = X + i;
    int y = Y;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X - i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X;
    y = Y + i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    y = Y - i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
  }

	return;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////QUEEN/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Queen::Queen(int col, Game * game) : Piece(col, game, 9){}
bool Queen::isLeagal(int x, int y){

  int vertmod = 0;
  int hortmod = 0;
  
  if(x == X || y == Y){  
    hortmod = (x==X)? 0:((x<X)?-1:1);
    vertmod = (y==Y)? 0:((y<Y)?-1:1);
  }
  else if(abs(y-Y) == abs(x-X)){
    vertmod = (y<Y)?-1:1;
    hortmod = (x<X)?-1:1;
  }
  else
    return false;
  
  int tx = X + hortmod;
  int ty = Y + vertmod;
      
  while(tx != x || ty != y){
    if(gm->getBoard()->getPiece(tx,ty) != NULL)
      return false;
    tx += hortmod;
    ty += vertmod;
  }
  
  return (gm->getBoard()->getPiece(x,y) == NULL || gm->getBoard()->getPiece(x,y)->getColor() != color);
}
string Queen::toString(){
  
  string str = (getColor()==WHITE? "White":"Black"); 
  return str + " Queen";
}
string Queen::toShortString(){
  /*string str = (getColor()==WHITE? "W":"B"); 
  return str + "Q";*/

  return (getColor()==WHITE? "Q":"q");
}
void Queen::getMoves(vector<string> &moves){
  
  int oldloc = X*1000 + Y*100;
  
  for(int i = 1; i < 8; i++){
    int x = X + i;
    int y = Y;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X - i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X;
    y = Y + i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X;
    y = Y - i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X + i;
    y = Y + i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X - i;
    y = Y + i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X + i;
    y = Y - i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
    x = X - i;
    y = Y - i;
    if(Piece::onBoard(x,y) && isLeagal(x,y))
      moves.push_back(Board::toStr(oldloc+x*10+y));
  }
  
  return;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////KING/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

King::King(int col, Game * game) : Piece(col, game, 25){
  hasMoved = false;
}
bool King::isLeagal(int x, int y){

  return (abs(x-X) <= 1 && abs(y-Y) <= 1 && (gm->getBoard()->getPiece(x,y) == NULL || gm->getBoard()->getPiece(x,y)->getColor() != color));// {
  //   cout << X << x << "yes" << endl;
  //   return true;
  // }
  // return false;
}
string King::toString(){
  
  string str = (getColor()==WHITE? "White":"Black"); 
  return str + " King";
}
string King::toShortString(){
  /*string str = (getColor()==WHITE? "W":"B"); 
  return str + "K";*/

  return (getColor()==WHITE? "K":"k");
}
bool King::move(int x, int y){
  
  if(abs(x-X) == 2 && castleLegal(x,y)){
	  
    int rko; 
	if(x<X)
		rko = 0;
	else
		rko = 7;

    int rkn;
	if(x<X)
		rkn = 3;
	else
		rkn = 5;
	if(gm->getBoard()->pieces[rko][y] == NULL || !gm->getBoard()->pieces[rko][y]->move(rkn, y))
		return false;

    gm->getBoard()->pieces[x][y] = this;
    gm->getBoard()->pieces[X][Y] = NULL;

    change_t c;
    c.moded = this;
    c.oldLoc = X*10 + Y;
    c.newLoc = x*10 + y;
    c.captured = false;
    c.firstMove = true;
    c.ep = false;
    gm->addChange(c);

    X=x;
    Y=y;

	return true;
  }
  else if(Piece::move(x,y)){
    if(!hasMoved){
		change_t * c = &gm->moveTree->current->changes.back();
      c->firstMove = true;
      hasMoved = true;
    }
    return true;
  }
  else{
    return false;
  }
}
bool King::castleLegal(int x, int y){
	//initial king test
	if(abs(x-X) != 2 || (x != 2 && x != 6) || this->hasMoved || gm->getBoard()->pieces[x][y] != NULL || gm->inCheck(gm->getKing(this->color)))
		return false;
	
	//test rook
	int rx = 0;
	if(x>X)
		rx = 7;
	if(gm->getBoard()->pieces[rx][y] == NULL || (gm->getBoard()->pieces[rx][y]->toShortString().find("R") != 0 && gm->getBoard()->pieces[rx][y]->toShortString().find("r") != 0)
		|| gm->getBoard()->pieces[rx][y]->hasMoved)
		return false;

	//test clear path and doesn't enter check
	int oX = X;
	int hortmod = (x<X)?-1:1;
	//first space
	X=oX + hortmod;
	if(gm->getBoard()->pieces[X][y] != NULL){
		X = oX;
		return false;
	}
	gm->getBoard()->pieces[X][y] = gm->getBoard()->pieces[oX][Y];
	gm->getBoard()->pieces[oX][Y] = NULL;
	bool check = gm->inCheck(gm->getKing(this->color));
	gm->getBoard()->pieces[oX][Y] = gm->getBoard()->pieces[X][y];
	gm->getBoard()->pieces[X][y] = NULL;
	X = oX;
	if(check){
		return false;
	}
	//test destination
	X=x;
	gm->getBoard()->pieces[x][y] = gm->getBoard()->pieces[oX][Y];
	gm->getBoard()->pieces[oX][Y] = NULL;
	check = gm->inCheck(gm->getKing(this->color));
	gm->getBoard()->pieces[oX][Y] = gm->getBoard()->pieces[x][y];
	gm->getBoard()->pieces[x][y] = NULL;
	X = oX;
	if(check)
		return false;

	//test left rook space clear
	if(x<X && gm->getBoard()->pieces[1][y] != NULL)
		return false;
	return true;
}
void King::getMoves(vector<string> &moves){
  
  int currentLoc = X * 1000 + Y * 100;
  for(int i = -1; i <=1; i++){
    for(int j = -1; j <=1; j++){
      if(i==0 && j == 0) continue;
      int x = X + i;
      int y = Y + j;
      
      if(Piece::onBoard(x,y) && isLeagal(x,y))
	moves.push_back(Board::toStr(currentLoc+x*10+y));
    }
  }

  if(color == WHITE && castleLegal(6,0))
    moves.push_back(Board::toStr(currentLoc+6*10+0));
  if(color == BLACK && castleLegal(6,7))
    moves.push_back(Board::toStr(currentLoc+6*10+7));
  if(color == WHITE && castleLegal(2,0))
    moves.push_back(Board::toStr(currentLoc+2*10+0));
  if(color == BLACK && castleLegal(2,7))
    moves.push_back(Board::toStr(currentLoc+2*10+7));
  
  return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////END/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
