//board.cpp
//Fly By Knight - Chess Engine
//Created by Edward Sandor 2011-2012.
//Read the README for more.

#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include "board.hpp"
#include "game.hpp"
#include "piece.hpp"
#include "moveTree.hpp"

using namespace std;

Board::Board(Game * g){
  gm = g;
  clearBoard();
}

void Board::clearBoard(){
  for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++){
      //if(pieces[i][j] != NULL)
		//delete pieces[i][j];
      pieces[i][j] = NULL;
    }
 }

void Board::placePiece(Piece * piece, int x, int y){
  removePiece(x, y);
  pieces[x][y] = piece;
  if(piece != NULL)
    piece->setLocation(x, y);
}
void Board::removePiece(int x, int y){
  if(pieces[x][y] != NULL)
    delete pieces[x][y];
}
Piece * Board::getPiece(int x, int y){
	return pieces[x][y];
}
void Board::printBoard(){
  /*cout << " |--|--|--|--|--|--|--|--|" << endl;
  for(int i = 7; i >= 0; i--){
    cout << (i + 1) << "|";
    for(int j = 0 ; j < 8; j++)
      cout << (pieces[j][i] == NULL? "  ":pieces[j][i]->toShortString()) << "|";
    cout << endl << " |--|--|--|--|--|--|--|--|" << endl;
  }
  
  cout  << "   A  B  C  D  E  F  G  H" << endl;*/
  
for(int i = 7; i >= 0; i--){
    cout << i + 1 << "  ";
    for(int j = 0 ; j < 8; j++)
      cout << (pieces[j][i] == NULL? (((i+j) % 2 == 0)? "#": ".") :pieces[j][i]->toShortString()) << " ";
    cout << endl;
  }
  cout << /*"   ---------------" <<*/ endl <<  "   a b c d e f g h" << endl;
}
void Board::clearSpace(int x, int y){
  pieces[x][y]  = NULL;
}

int Board::toInts(string str){

  /*if(str.length() < 2)
    return -1;

  int ret = 0;
  ret = str.at(0)-97;
  if(ret > 7 || ret < 0)
    return -1;
  if(str.at(1) > 56 || str.at(1) < 49)
    return -1;

  return ((ret * 10) + str.at(1) - 49);*/

	if(str.size() < 2)
		return -1;
	int ret = 0;
	ret += (int)(str.at(0))-97;
	if(ret > 7 || ret < 0)
		return -1;
	ret *= 10;
	if((int)(str.at(1)) > 56 || (int)(str.at(1)) < 49)
		return -1;
	ret += str.at(1) - 49;
	if(str.size() >= 4){
		ret *= 100;
		ret += (((int)(str.at(2))-97)*10);
		if((ret%100)/10 > 7 || (ret%100)/10 < 0)
			return -1;
		if((int)(str.at(3)) > 56 || (int)(str.at(3)) < 49)
			return -1;
		ret += (int)(str.at(3)) - 49;
		return ret;
	}
	else
		return ret;
}
string Board::toStr(int mov){
  
	if(mov < 100){
		if((mov / 10) >= 8 || (mov / 10) < 0 || (mov % 10) >= 8 || (mov % 10) < 0 )
			 return "...---...";

		string str = "";
		str += (mov / 10) + 97;
		str += (mov % 10) + 49;
		return str;
	}
	else{
		if(mov/1000 >= 8 || mov / 1000 < 0 || (mov%1000)/100 >= 8 || (mov%1000)/100 < 0 || (mov%100)/10 >= 8 || mov%100/10 < 0 || mov%10 >= 8 || mov%10 < 0)
		return "...---...";
	
		string str = "";
		str += (mov/1000) + 97;
		str += (mov%1000)/100 + 49;
		str += (mov%100) /10 + 97;
		str += (mov%10) + 49;
		return str;
	}
}
bool Board::moveFormat(string str){
  
  return (Board::toInts(str) != -1  && (str.length() == 4 || (str.length() == 5 && (str.at(4) == 'b' || str.at(4) == 'n' || str.at(4) == 'r' || str.at(4) == 'q'))));
}

void Board::getPieces(int color, vector<Piece *> &found){
  for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
      if(pieces[i][j] != NULL && pieces[i][j]->getColor() == color)
	found.push_back(pieces[i][j]);
}

bool Board::promotePawn(char newPiece){
  
  int row = 0;
  for(int i = 0; i < 2; i++){
    for(int j = 0; j < 8; j++){
      Piece * p = pieces[j][row];
      if(p != NULL && (p->toShortString().at(0) == 'P' || p->toShortString().at(0) == 'p')){
	Piece * nPiece;
	switch(newPiece){
	case 'N':
	case 'n':
	  nPiece = new Knight(p->getColor(), p->getGame());
	  break;
	case 'B':
	case 'b':
	  nPiece = new Bishop(p->getColor(), p->getGame());
	  break;
	case 'R':
	case 'r':
	  nPiece = new Rook(p->getColor(), p->getGame());
	  static_cast<Rook *>(nPiece)->hasMoved = true;
	  break;
	case 'Q':
	case 'q':
	default:
	  nPiece = new Queen(p->getColor(), p->getGame());
	  break;
	}
	
	if(p != NULL){
	  change_t c;
	  c.moded = p;
	  c.newLoc = j*10 + row;
	  c.oldLoc = -11;
	  c.captured = false;
	  c.firstMove = false;
	  c.ep = false;

	  gm->addChange(c);
	}
	
	pieces[j][row] = nPiece;
	nPiece->setLocation(j,row);

	change_t c;
	c.moded = nPiece;
	c.oldLoc = -11;
	c.newLoc = j*10 + row;
	c.captured = false;
	c.firstMove = false;
	c.ep = false;
	
	gm->addChange(c);	
      }
    }
    row = 7;
  }
  
  return false;
}
