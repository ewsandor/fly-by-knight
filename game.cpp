//game.cpp
//Fly By Knight - Chess Engine
//Created by Edward Sandor 2011-2012.
//Read the README for more.

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <time.h>
#include <stack>
#include "main.hpp"
#include "game.hpp"
#include "board.hpp"
#include "piece.hpp"
#include "moveTree.hpp"
using namespace std;

Game::Game(){
	board = new Board(this);
	resetGame();
	place = 0;
	moveTree = new MoveTree(this);
}

void Game::clear(){
	for(int i = 0; i < 8; i++)
		for(int j = 0; j< 8; j++)
			if(board->pieces[i][j] != NULL){
				change_t c;
				c.moded = board->pieces[i][j];
				c.newLoc = i * 10+ j;
				c.oldLoc = c.newLoc;
				c.captured = true;
				c.ep = false;
				c.firstMove = false;

				addChange(c);

				board->pieces[i][j] = NULL;
			}
}

void Game::resetGame(){
	//reverse tree to move 0
	enpasantable = NULL;
	turn = WHITE;
	playAs = BLACK;  
	setupBoard();
	moveTree->current = moveTree->root;
	clearEnd();
}

void Game::setupBoard(){
	board->clearBoard();

	board->placePiece(new Rook(WHITE, this), 0, 0);
	board->placePiece(new Rook(WHITE, this), 7, 0);

	board->placePiece(new Knight(WHITE, this), 1, 0);
	board->placePiece(new Knight(WHITE, this), 6, 0);

	board->placePiece(new Bishop(WHITE, this), 2, 0);
	board->placePiece(new Bishop(WHITE, this), 5, 0);

	board->placePiece(new Queen(WHITE, this), 3, 0);
	board->placePiece(new King(WHITE, this), 4, 0);
	whiteKing = board->getPiece(4,0);

	for(int i = 0; i < 8; i++){
		board->placePiece(new Pawn(WHITE, this), i, 1);
	}

	board->placePiece(new Rook(BLACK, this), 0, 7);
	board->placePiece(new Rook(BLACK, this), 7, 7);

	board->placePiece(new Knight(BLACK, this), 1, 7);
	board->placePiece(new Knight(BLACK, this), 6, 7);

	board->placePiece(new Bishop(BLACK, this), 2, 7);
	board->placePiece(new Bishop(BLACK, this), 5, 7);

	board->placePiece(new Queen(BLACK, this), 3, 7);
	board->placePiece(new King(BLACK, this), 4, 7);
	blackKing = board->getPiece(4,7);

	for(int i = 0; i < 8; i++){
		board->placePiece(new Pawn(BLACK, this), i, 6);
	}
}
void Game::changeTurn(){
	enpasantable = NULL;
	turn = abs(turn - 1);
}
int Game::getTurn(){  
	return turn;
}
Board * Game::getBoard(){
	return board;
}
bool Game::move(string str){

	if(str.length() < 4 || str.length() > 5){
		return false;
	}
	int from = Board::toInts(str.substr(0,2));

	Piece * p = board->pieces[from / 10][from % 10];

	if(move(Board::toInts(str.substr(0,2)),Board::toInts(str.substr(2,2)))){
		if(p->toShortString().at(0) == 'P' || p->toShortString().at(0) == 'p'){
			board->promotePawn((str.length() == 5?str.at(4):'e'));
		}
		return true;
	}

	return false;
}

bool Game::move(int from, int to){

	if(from / 10 < 0 || from / 10 >7 || from % 10 < 0 || from % 10 >7 || to / 10 < 0 || to / 10 > 7 || to % 10 < 0 || to % 10 > 7 || board->pieces[from / 10][from % 10] == NULL)
		return false;

	return ( board->pieces[from / 10][from % 10]->move(to / 10, to % 10));
}
bool Game::move(Move * mov){
	while(moveTree->current->turn >= mov->turn){
		if(moveTree->current->turn == 0)
			return false;
		moveBack();
	}
	stack<Move *> chain;
	while(mov->turn - chain.top()->turn > 1){
		if(chain.top()->turn == 0)
			return false;
		chain.push(mov);
		mov = mov->parent;
	}
	return true;
}
bool Game::inCheck(Piece * checkie)
{
	vector<Piece *> watchOut;
	board->getPieces(checkie->getColor() == WHITE?BLACK:WHITE, watchOut);

	for(unsigned int i = 0; i < watchOut.size(); i++)
		if(watchOut[i]->isLeagal(checkie->getX(), checkie->getY())){
			//cout << watchOut[i]->toString() << endl;
			return true;
		}

	return false;
}
void Game::setKing(int color, Piece * k){
	if(color == WHITE)
		whiteKing = k;
	else
		blackKing = k;
}
Piece * Game::getKing(int color){
	return color==WHITE?whiteKing:blackKing;
}
bool Game::inCheckmate(Piece * matey){

	if(!inCheck(matey))
		return false;

	return inMate(matey);
}
bool Game::inStalemate(Piece * matey){

	if(inCheck(matey))
		return false;

	return inMate(matey);
}
bool Game::inMate(Piece * matey){
	vector <Piece *> pieces;

	board->getPieces(matey->getColor(), pieces);

	for(unsigned int i = 0; i < pieces.size(); i++){
		vector<int> moves;
		pieces[i]->getMoves(moves);

		for(unsigned int i = 0; i < moves.size(); i++){

			bool causesCheck = board->pieces[moves[i]/1000][(moves[i]%1000)/100]->causesCheck((moves[i]%100)/10, moves[i]%10);

			/*int oldLoc = moves[i]/100;
			  int newLoc = moves[i]%100;
			  Piece * oldPiece = board->getPiece(newLoc/10, newLoc%10);

			  board->pieces[oldLoc/10][oldLoc%10]->setLocation(newLoc/10, newLoc%10);
			  board->pieces[newLoc/10][newLoc%10] = board->pieces[oldLoc/10][oldLoc%10];
			  board->pieces[oldLoc/10][oldLoc%10] = NULL;

			  int check = inCheck(matey);

			  board->pieces[newLoc/10][newLoc%10]->setLocation(oldLoc/10, oldLoc%10);
			  board->pieces[oldLoc/10][oldLoc%10] = board->pieces[newLoc/10][newLoc%10];
			  board->pieces[newLoc/10][newLoc%10] = oldPiece;    */
			if(!causesCheck)
				return false;
		}
	}

	return true;
}
bool Game::modSquare(string mod, int color){
	if(mod.length() != 3)
		return false;

	int loc = Board::toInts(mod.substr(1,2));
	if(loc < 0) return false;

	int x = loc / 10;
	int y = loc % 10;

	Piece * nPiece;
	switch(mod.at(0)){
		case 'P':
		case 'p':
			nPiece = new Pawn(color, this);
			if(!(color == WHITE && y == 1) && !(color == BLACK && y == 6))
				static_cast<Pawn *>(nPiece)->hasMoved = true;
			break;
		case 'N':
		case 'n':
			nPiece = new Knight(color, this);
			break;
		case 'B':
		case 'b':
			nPiece = new Bishop(color, this);
			break;
		case 'R':
		case 'r':
			nPiece = new Rook(color, this);
			if(!(color == WHITE && y == 0 && (x == 0 || x == 7)) && !(color == BLACK && y == 7 && (x == 0 || x == 7)))
				static_cast<Rook *>(nPiece)->hasMoved = true;
			break;
		case 'Q':
		case 'q':
			nPiece = new Queen(color, this);
			break;
		case 'K':
		case 'k':
			nPiece = new King(color, this);
			setKing(color, nPiece);
			if(!(color == WHITE && y == 0 && x == 4) && !(color == BLACK && y == 7 && x == 4))
				static_cast<King *>(nPiece)->hasMoved = true;
			break;
		case 'X':
		case 'x':
		default:
			nPiece = NULL;
			break;
	}
	if(board->pieces[x][y] != NULL){
		
		//is this where the bug was?
		/*bool add = true;
		for(unsigned int i = 0; i < changes[place].size(); i++){
			if(changes[place][i].moded == board->pieces[x][y]){
				add = false;
			}
		}

		if(add){*/
			change_t c;
			c.moded = board->pieces[x][y];
			c.oldLoc =x*10 + y;
			c.newLoc = c.oldLoc;
			c.captured = true;
			c.firstMove = false;
			c.ep = false;

			addChange(c);
		//}
	}
	if(nPiece != NULL){
		nPiece->setLocation(x, y);
		board->pieces[x][y] = nPiece;

		change_t c;
		c.moded = nPiece;
		c.oldLoc = -11;
		c.newLoc = x*10+y;
		c.captured = false;
		c.firstMove = false;
		c.ep = false;

		addChange(c);
	}
	else
		board->pieces[x][y] = NULL;

	return true;
}
bool Game::causesCheck(Piece * piece, int mov){
	int oldLoc = mov/100;
	int newLoc = mov%100;
	Piece * oldPiece = board->getPiece(newLoc/10, newLoc%10);

	board->pieces[oldLoc/10][oldLoc%10]->setLocation(newLoc/10, newLoc%10);
	board->pieces[newLoc/10][newLoc%10] = board->pieces[oldLoc/10][oldLoc%10];
	board->pieces[oldLoc/10][oldLoc%10] = NULL;

	int check = inCheck(piece);

	board->pieces[newLoc/10][newLoc%10]->setLocation(oldLoc/10, oldLoc%10);
	board->pieces[oldLoc/10][oldLoc%10] = board->pieces[newLoc/10][newLoc%10];
	board->pieces[newLoc/10][newLoc%10] = oldPiece;    

	return check;
}
bool Game::endGame(){
	if(inCheckmate(getKing(BLACK))){
		handleOutput("1-0 {White mates}");
		handleInput("force");
		return true;
	}
	else if(inCheckmate(getKing(WHITE))){
		handleOutput("0-1 {Black mates}");
		handleInput("force");
		return true;
	}
	else if((turn == BLACK && inStalemate(getKing(BLACK))) || (turn == WHITE && inCheckmate(getKing(WHITE)))){
		handleOutput("1/2-1/2 {Stalemate}");
		handleInput("force");
		return true;
	}
	return false;
}  
void Game::addTurn(){
	//implement based on moveTree
	clearEnd();
	place++;
	changes.push_back(vector<change_t>());
}
void Game::removeTurn(){
	//implement based on moveTree
	if(place <= 0) return;
	place--;
	changes.pop_back();
}
void Game::addChange(change_t change){
	//add to move tree's changes instaed of game's
	moveTree->current->changes.push_back(change);
}
void Game::clearEnd(){  
	//replaced in moveTree with clearChildren() or deconstructor?
	while(changes.size() > place){
		changes.pop_back();
	}
	if(place == 0 && turn == BLACK) changeTurn();
}
bool Game::moveBack(){
	//fix to use moveTrees changes
	//done ?
	if(place <= 0) return false;  

	changeTurn();
	if(moveTree->current->turn > 0)
		moveTree->current = moveTree->current->parent;
	else return false;

	enpasantable = NULL;

	for(unsigned int i = 0; i < moveTree->current->changes.size(); i++){
		change_t  * c = &changes[place][i];
		if(c->oldLoc != c->newLoc){
			if(Piece::onBoard(c->oldLoc/10, c->oldLoc%10)){
				board->pieces[c->oldLoc / 10][c->oldLoc % 10] = c->moded;
				c->moded->setLocation(c->oldLoc / 10, c->oldLoc % 10);
			}
			if(Piece::onBoard(c->newLoc/10, c->newLoc%10) && board->pieces[c->newLoc / 10][c->newLoc % 10] == c->moded)
				board->pieces[c->newLoc / 10][c->newLoc % 10] = NULL;
		}
		if(c->captured && Piece::onBoard(c->oldLoc / 10,c->oldLoc % 10))
			board->pieces[c->oldLoc / 10][c->oldLoc % 10] = c->moded;
		if(c->firstMove)
			c->moded->hasMoved = false;
		if(place > 0)
			for(unsigned int i = 0; i < changes[place - 1].size(); i++){
				change_t * c = &changes[place - 1][i];
				if(c->ep)
					enpasantable = c->moded;
			}
	}
	if(place == 0 && turn == BLACK) changeTurn();
	return true; //return if did move back
}
bool Game::moveBack(int steps){
	for(int i = 0; i < steps; i++){
		if(!moveBack()) return false;
	}
	return true; //return if all moved back
}
bool Game::moveForeward(){
	//fix to use moveTrees changes
	//stop if multiple options...
	//if(place >= changes.size()) return false;
	//done ?

	if(moveTree->current->choices.size() != 1) return false;

	for(unsigned int i = 0; i < moveTree->current->changes.size(); i++){
		change_t  * c = &moveTree->current->changes[i];
		if(c->oldLoc != c->newLoc){
			if(Piece::onBoard(c->newLoc/10, c->newLoc%10)){
				board->pieces[c->newLoc / 10][c->newLoc % 10] = c->moded;
				c->moded->setLocation(c->newLoc / 10, c->newLoc % 10);
			}
			if(Piece::onBoard(c->oldLoc/10, c->oldLoc%10) && board->pieces[c->oldLoc / 10][c->oldLoc % 10] == c->moded)
				board->pieces[c->oldLoc / 10][c->oldLoc % 10] = NULL;
		}
		if(c->captured && Piece::onBoard(c->newLoc/10, c->newLoc%10) && board->pieces[c->newLoc / 10][c->newLoc % 10] == c->moded)
			board->pieces[c->newLoc / 10][c->newLoc % 10] = NULL;
		if(c->firstMove)
			c->moded->hasMoved = true;
		if(c->ep)
			enpasantable = c->moded;    
	}
	moveTree->current = moveTree->current->choices[0];
	return true; //return if successfully moved forward
}
bool Game::moveForeward(int steps){
	for(int i = 0; i < steps; i++){
		if(!moveForeward()) return false;
	}
	return true; //return if all successfully moved forward
}

double Game::evaluateBoard(){

	double score = 0;

	vector<Piece *> pieces;
	board->getPieces(WHITE, pieces);
	board->getPieces(BLACK, pieces);

	for(unsigned int i = 0; i < pieces.size(); i++){
		int mute = pieces[i]->getColor() == WHITE?1:-1;

		score += mute * (pieces[i]->getValue());
		vector <int> moves;
		pieces[i]->getMoves(moves);

		score += mute * (moves.size());
	}

	return score;
}
string Game::chooseMove(){
	if(playAs != turn)
		return "...---...";

	vector<Piece *> pieces;

	board->getPieces(playAs, pieces);

	vector<int> moves;

	for(unsigned int i = 0; i < pieces.size(); i++)
		pieces[i]->getMoves(moves);

	// for(unsigned int i = 0; i < moves.size(); i++)
	//   while( i < moves.size() && board->pieces[moves[i]/1000][(moves[i]%1000)/100]->causesCheck((moves[i]%100)/10, moves[i]%10))
	//     moves.erase(moves.begin() + i);

	if(moves.size() == 0)
		return "...---...";

	vector <vector<double> > scores(moves.size(), vector<double>(2));

	for(unsigned int i = 0; i < moves.size(); i++)
		if(!( i < moves.size() && board->pieces[moves[i]/1000][(moves[i]%1000)/100]->causesCheck((moves[i]%100)/10, moves[i]%10))){
			//double[]* tmp = {(double) moves[i], 0.0};
			//scores.push_back(tmp);
			scores[i][0] = (double)moves[i];
		}
		else
			scores[i][0] = -1;

	moves.clear();
	for(unsigned int i = 0; i < scores.size(); i++){
		if(scores[i][0] != -1)
			moves.push_back(scores[i][0]);
	}

	int mov = moves[time(NULL)%moves.size()];

	return Board::toStr(mov / 100) + Board::toStr(mov % 100); 
}
