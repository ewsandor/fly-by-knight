//game.cpp
//Fly By Knight - Chess Engine
//Created by Edward Sandor 2011-2012.
//Read the README for more.

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#include <stack>
#include <cmath>
#include <fstream>
#include <regex>
#include "main.hpp"
#include "game.hpp"
#include "board.hpp"
#include "piece.hpp"
#include "moveTree.hpp"
using namespace std;


Game::Game(){
	book = "";
	post = true;
	board = new Board(this);
	moveTree = new MoveTree(this);
	resetGame();
	openBook();
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
	bkIndex = 0;
	nodes = 0;
	enpasantable = NULL;
	playAs = BLACK;  
	if(moveTree->root != NULL)
		delete moveTree->root;
	setupBoard();
	moveTree->root = new Move();
	moveTree->root->id = "ROOT";
	moveTree->actual = moveTree->root;
	moveTree->current = moveTree->root;
	analysisQueue.clear();
	analysisQueue.push_back(moveTree->root);
	searchClock = clock();
	draw = false;
	onBook = true;
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
void Game::updateClocks(){
}
int Game::getTurn(){  
	return moveTree->current->turn % 2;
}
Board * Game::getBoard(){
	return board;
}
bool Game::move(string str){

	//maybe put better serch method if sort choices elsewhere

	if(str.compare("ROOT") == 0 && moveTree->root != NULL)
		return move(moveTree->root);
	for(unsigned int i = 0; i < moveTree->current->choices.size(); i++){
		if(moveTree->current->choices[i]->id.compare(str) == 0)
			return move(moveTree->current->choices[i]);
	}

	if(str.length() < 4 || str.length() > 5){
		return false;
	}
	int from = Board::toInts(str.substr(0,2));
	int to = Board::toInts(str.substr(2,2));

	Piece * p = board->pieces[from / 10][from % 10];


	if(move(from, to)){
		if(p->toShortString().at(0) == 'P' || p->toShortString().at(0) == 'p'){
			board->promotePawn((str.length() == 5?str.at(4):'e'));
		}
		moveTree->current->id = str;
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
	if(mov == NULL)
		return false;

	if(mov == moveTree->current)
		return true;
	if(mov == moveTree->root){
		while(moveTree->current != moveTree->root)
			if(!moveBack())
				return false;
		return true;
	}
	if(mov->parent != moveTree->current){
		//bring current position back to same turn as move if ahead
		while(moveTree->current->turn > mov->turn){
			if(moveTree->current->turn == 0)
				return false;
			moveBack();
		}
		//build chain to get mov to current position if ahead
		vector<Move *> chain;
		while(mov->turn > moveTree->current->turn){
			if(!chain.empty() && chain.back()->turn == 0)
				return false;
			chain.push_back(mov);
			mov = mov->parent;
		}
		//move back to find common parent between move and current position
		while(mov->parent != moveTree->current->parent){
			if(moveTree->current->turn == 0)
				return false;
			moveBack();

			if(!chain.empty() && chain.back()->turn == 0)
				return false;
			chain.push_back(mov);
			mov = mov->parent;
		}
		//move current position to parent and follow chain to original mov
		if(moveTree->current->turn == 0)
			return false;
		moveBack();

		if(!moveForward(mov))return false;
		while(chain.size() > 0){
			mov = chain.back();
			chain.pop_back();
			if(!moveForward(mov))return false;
		}
	}
	else
		if(!moveForward(mov))return false;

		
	//write moveForward(Move * mov).  Move to move in choices or add to choices.
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
bool Game::inDraw(){
	return draw;
}
bool Game::inMate(Piece * matey){
	vector <Piece *> pieces;

	board->getPieces(matey->getColor(), pieces);

	for(unsigned int i = 0; i < pieces.size(); i++){
		vector<string> moves;
		pieces[i]->getMoves(moves);

		for(unsigned int i = 0; i < moves.size(); i++){

			int mov = Board::toInts(moves[i]);
			bool causesCheck = board->pieces[mov/1000][(mov%1000)/100]->causesCheck((mov%100)/10, mov%10);

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

	bool check = inCheck(piece);

	board->pieces[newLoc/10][newLoc%10]->setLocation(oldLoc/10, oldLoc%10);
	board->pieces[oldLoc/10][oldLoc%10] = board->pieces[newLoc/10][newLoc%10];
	board->pieces[newLoc/10][newLoc%10] = oldPiece;    

	return check;
}
bool Game::endGame(){
	if(inCheckmate(getKing(BLACK))){
		handleOutput("1-0 {White Mates}");
		handleInput("force");
		return true;
	}
	else if(inCheckmate(getKing(WHITE))){
		handleOutput("0-1 {Black Mates}");
		handleInput("force");
		return true;
	}
	else if((moveTree->actual->turn%2==BLACK && inStalemate(getKing(BLACK))) || (moveTree->actual->turn%2==WHITE && inCheckmate(getKing(WHITE)))){
		handleOutput("1/2-1/2 {Stalemate}");
		handleInput("force");
		return true;
	}
	return false;
}  
void Game::addTurn(){
	//implement based on moveTree
	Move * mov = new Move(moveTree->current);
	moveTree->current->choices.push_back(mov);
	move(mov);
}
/*void Game::removeTurn(){
	//implement based on moveTree
	if(place <= 0) return;
	place--;
	changes.pop_back();
}*/
void Game::addChange(change_t change){
	//add to move tree's changes instaed of game's
	moveTree->current->changes.push_back(change);
	if(change.captured)
		moveTree->current->capture=change.moded;
	if(change.moded != NULL && (change.moded->toShortString().compare("p") == 0 || change.moded->toShortString().compare("P") == 0)
		&& change.oldLoc!=change.newLoc)
		moveTree->current->pawnMove=true;
}
void Game::moveRoot(){  
	//replaced in moveTree with clearChildren() or deconstructor?
	/*while(changes.size() > place){
		changes.pop_back();
	}
	if(place == 0 && turn == BLACK) changeTurn();*/

	while(moveTree->current != moveTree->root)
		moveBack();
}
bool Game::moveBack(){
	//fix to use moveTrees changes
	//done ?
	if(moveTree->current->parent == NULL)
		return false;

	enpasantable = NULL;

	for(unsigned int i = 0; i < moveTree->current->changes.size(); i++){
		change_t  * c = &moveTree->current->changes[i];
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
		//if(place > 0)
		for(unsigned int i = 0; i < moveTree->current->parent->changes.size(); i++){
			change_t * c = &moveTree->current->parent->changes[i];
			if(c->ep)
				enpasantable = c->moded;
		}
	}
	moveTree->current = moveTree->current->parent;
	//if(place == 0 && turn == BLACK) changeTurn();
	return true; //return if did move back
}
bool Game::moveBack(int steps){
	for(int i = 0; i < steps; i++){
		if(!moveBack()) return false;
	}
	return true; //return if all moved back
}
bool Game::moveForward(){
	return (moveTree->current->choices.size() != 1)? false:moveForward(moveTree->current->choices[0]);
}
bool Game::moveForward(Move * mov){
	if(mov == NULL || mov->parent != moveTree->current)
		return false;

	moveTree->current = mov;
	enpasantable = NULL;

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
	//moveTree->current = moveTree->current->choices[0];
	return true; //return if successfully moved forward
}
bool Game::moveForward(int steps){
	for(int i = 0; i < steps; i++){
		if(!moveForward()) return false;
	}
	return true; //return if all successfully moved forward
}
//sets board to match the true board setup
bool Game::goActualLayout(){
	return move(moveTree->actual);
}
void Game::commitMove(){
	moveTree->actual = moveTree->current;
	nodes = 0;
	analysisQueue.clear();
	analysisQueue.push_back(moveTree->current);
	searchClock = clock();
}

double Game::evaluateBoard(){

	if(moveTree->current->evaluated)
		return moveTree->current->getScore();

	double score = 0.0;
	
	
	if(moveTree->current->pawnMove)
		if(moveTree->current->turn%2 == WHITE)
			score-=20;
		else
			score+=20;
	if(moveTree->current->capture != NULL)
		if(moveTree->actual->turn%2 == WHITE)
			score-=13*moveTree->current->capture->getValue();
		else
			score+=13*moveTree->current->capture->getValue();
	if(this->inCheckmate(this->whiteKing)){
		score -= 3900;
	}
	if(this->inCheckmate(this->blackKing)){
		score += 3900;
	}
			
	vector<Piece *> pieces;
	board->getPieces(WHITE, pieces);
	board->getPieces(BLACK, pieces);

	for(unsigned int i = 0; i < pieces.size(); i++){
		//vector<string> moves;
		//pieces[i]->getMoves(moves);
		//cout << pieces[i]->toString() << endl;
		double mute = pieces[i]->getColor() == WHITE?1:-1;
		//score += (mute * moves.size());
		score += (mute * (pieces[i]->getValue()) * 100);
		if(pieces[i] != pieces[i]->getGame()->getKing(pieces[i]->getColor())){
			score += mute * 50 *(Board::squareVal(pieces[i]->getX(), pieces[i]->getY()));
		}
	}

	moveTree->current->evaluated = true;
	return score;
}
void Game::stepAnalysis(){

	while(analysisQueue.size() > 0 && analysisQueue[0]->turn < moveTree->actual->turn)
		analysisQueue.erase(analysisQueue.begin());
	
	if(analysisQueue.size() == 0 || this->analysisQueue[0] == moveTree->actual){
		analysisQueue.clear();
		analysisQueue.push_back(moveTree->actual);
		move(moveTree->actual);
		findChoices(moveTree->actual);
		moveTree->actual->setScore(evaluateBoard());
		for(unsigned int i = 0; i < moveTree->actual->choices.size(); i++){
			analysisQueue.push_back(moveTree->actual->choices[i]);
		}
	}
	else
	{
		/*if(analysisQueue[0]->evaluated){
			nodes++;
			if(analysisQueue[0]->turn%2 == WHITE)
				for(unsigned int i = 1; i <= BREADTH && i <= analysisQueue[0]->choices.size(); i ++)
					analysisQueue.push_back(analysisQueue[0]->choices[analysisQueue[0]->choices.size()-i]);
			else		
				for(unsigned int i = 0; i < BREADTH && i < analysisQueue[0]->choices.size(); i ++)
					analysisQueue.push_back(analysisQueue[0]->choices[i]);
					analysisQueue.erase(analysisQueue.begin());
					return;
		}*/
		move(this->analysisQueue[0]);
		findChoices(this->analysisQueue[0]);
		analysisQueue[0]->sortScores();

		if(analysisQueue[0]->turn%2 == WHITE)
			for(unsigned int i = 1; i <= BREADTH && i <= analysisQueue[0]->choices.size(); i ++)
				analysisQueue.push_back(analysisQueue[0]->choices[analysisQueue[0]->choices.size()-i]);
		else		
			for(unsigned int i = 0; i < BREADTH && i < analysisQueue[0]->choices.size(); i ++)
				analysisQueue.push_back(analysisQueue[0]->choices[i]);
	
	}
	//if(playAs == moveTree->actual->turn%2 && moveTree->actual->getBest() != NULL && analysisQueue[0]->turn - moveTree->actual->turn >= DEPTH)
		//handleOutput("move " + moveTree->actual->getBest()->id);
	if(post && (analysisQueue.size() == 1 || analysisQueue[0]->turn<analysisQueue[1]->turn)){
		unsigned int depth = ((int)analysisQueue[0]->turn - moveTree->actual->turn);
		string think = "";
		think = think + to_string((long double)depth);
		think.append(" ");
		think.append(to_string((long double)floor((playAs!=BLACK?moveTree->actual->adjustedScore()+.5:-1*moveTree->actual->adjustedScore())+.5)));
		think.append(" ");
		double searchTime = ((clock() - searchClock) /CLOCKS_PER_SEC)*100;
		think.append(to_string((long double)searchTime));
		think.append(" ");
		think.append(to_string((long double)nodes));

		Move * best = moveTree->actual->getBest();
		//bool out = best->evaluated;
		
		while(best != NULL){
			think.append(" ");
			think.append(best->id);
			best = best->getBest();
		}
			
		//if(out)
		handleOutput(think);
	}
	analysisQueue.erase(analysisQueue.begin());

}

string Game::chooseMove(){
	if(playAs != getTurn())
		return "...---...";

	vector<Piece *> pieces;

	board->getPieces(playAs, pieces);

	vector<string> moves;

	for(unsigned int i = 0; i < pieces.size(); i++)
		pieces[i]->getMoves(moves);

	// for(unsigned int i = 0; i < moves.size(); i++)
	//   while( i < moves.size() && board->pieces[moves[i]/1000][(moves[i]%1000)/100]->causesCheck((moves[i]%100)/10, moves[i]%10))
	//     moves.erase(moves.begin() + i);

	if(moves.size() == 0)
		return "...---...";

	vector<string> legalMoves;

	for(unsigned int i = 0; i < moves.size(); i++){
		//cout << moves[i] << endl;
		if(move(moves[i])){
			legalMoves.push_back(moves[i]);
			moveBack();
		}
	}
	if(legalMoves.size() == 0)
		return "...---...";/*
	for(unsigned int i = 0; i < legalMoves.size(); i++)	
		cout << legalMoves[i] << endl;*/

	return legalMoves[(int)time(NULL)%legalMoves.size()];
	/*vector <vector<double> > scores(moves.size(), vector<double>(2));

	for(unsigned int i = 0; i < moves.size(); i++){
		int mov = Board::toInts(moves[i]);
		if(!( i < moves.size() && board->pieces[mov/1000][(mov%1000)/100]->causesCheck((mov%100)/10, mov%10))){
			//double[]* tmp = {(double) moves[i], 0.0};
			//scores.push_back(tmp);
			scores[i][0] = (double)mov;
		}
		else
			scores[i][0] = -1;
	}

	moves.clear();
	for(unsigned int i = 0; i < scores.size(); i++){
		if(scores[i][0] != -1)
			moves.push_back(Board::toStr(scores[i][0]));
	}

	return moves[time(NULL)%moves.size()];
	*/
}

void Game::findChoices(Move * mov){

	Move * curMove = moveTree->current;

	if(!move(mov)){
		return;
	}

	vector<Piece *> pieces;

	board->getPieces(mov->turn%2, pieces);

	vector<string> moves;

	for(unsigned int i = 0; i < pieces.size(); i++)
		pieces[i]->getMoves(moves);
	
	if(moves.size() == 0)
		return;

	for(unsigned int i = 0; i < moves.size(); i++){
		move(mov);
		if(move(moves[i])){			
			Move * t = mov->getChoice(moves[i]);
			nodes++;
			move(t);
			//if(!t->evaluated){
				t->setScore(evaluateBoard());
			/*Move * up = t;			
			while(up->parent != NULL){
				up->updateAdjuster();
				up = up->parent;
			}*/
		}
	}
	move(curMove);

}
bool Game::recordGameToBook(string result){
	int win = -99;
	if(result.find("0-1")==0)
		win=BLACK;
	else if(result.find("1-0")==0)
		win=WHITE;
	else if(result.find("1/2-1/2") == 0)
		win=NONE;
	else
		return false;
		

	
	stack<Move *> tree;
	tree.push(moveTree->actual);
	while(tree.top()->parent != NULL)
		tree.push(tree.top()->parent);

	int index = book.find("M");
	
	//only deals with single digit number at the moment
	while(index != -1 && tree.size() > 0 && tree.top()->turn < 10 && tree.top()->turn <= (unsigned int) book.at(index+3)-48){
		if(tree.top()->turn == book.at(index+1)-48 && tree.top()->id.find(book.substr(index+3,4))==0){
			bool updated = false;
			int st = book.find(":",index + 1);
			int en = book.find(":", st+1);
			st = en;
			en = book.find(":", st+1);
			string counts = book.substr(st+1, en-st-1);
			int countsi = stoi(counts);
			if(win==WHITE)
				book.replace(st+1, en-st-1, to_string((long double)countsi+1));
			st=book.find(":", st+1);
			en=book.find(":", st+1);
			counts = book.substr(st+1, en-st-1);
			countsi = stoi(counts);
			if(win==BLACK)
				book.replace(st+1, en-st-1, to_string((long double)countsi+1));
			st=book.find(":", st+1);
			en=book.find("M", st+1);
			counts = book.substr(st+1, en-st-1);
			countsi = stoi(counts);
			if(win==NONE)				
				book.replace(st+1, en-st-1, to_string((long double)countsi+1));
			//implement update to outcomes here
			tree.pop();
		}
		index = book.find("M", index +1);
	}
	while(tree.size() > 0 && tree.top()->turn < 10){
		string entry = "M";
		entry.append(to_string((long double)tree.top()->turn));
		entry.append(":");
		entry.append(tree.top()->id);
		entry.append(":");
		entry.append(to_string((long double)(win==WHITE?1:0)));
		entry.append(":");
		entry.append(to_string((long double)(win==BLACK?1:0)));
		entry.append(":");
		entry.append(to_string((long double)(win==NONE?1:0)));
		if(index == -1)
			book = book + entry;
		else
			book = book.substr(0,index) + entry + book.substr(index);
		tree.pop();
		if(index != -1)
			index = book.find("M", index +1);
	}

	//cout << bkstr << endl;
	ofstream booko;
	booko.open("book.fbk", ofstream::out | ofstream::trunc);
	if(booko.is_open()){
		booko << book << endl;
	}
	booko.close();
	return true;
}

bool Game::openBook(){
	return openBook("book.fbk");
}
bool Game::openBook(string loc){
	book = "";
	ifstream booki;
	booki.open(loc, ifstream::in);
	if(booki.is_open()){
		string tmp;
		getline(booki,tmp);
		book=book+tmp;
	}
	booki.close();
	return booki.is_open();
}

Move * Game::getBookMove(){	
	if(!onBook)
		return NULL;

	findChoices(moveTree->current);

	stack<Move *> tree;
	tree.push(moveTree->current);
	while(tree.top()->parent != NULL)
		tree.push(tree.top()->parent);

	int index = book.find("M");
	
	//only deals with single digit number at the moment
	while(index != -1 && tree.size() > 0 && tree.top()->turn < 10 && tree.top()->turn <= (unsigned int) book.at(index+3)-48){
		if(tree.top()->turn == book.at(index+1)-48 && tree.top()->id.find(book.substr(index+3,4))==0){
			if(tree.top() == moveTree->current)
				break;
			tree.pop();
		}
		index = book.find("M", index +1);
	}

	/*bkIndex = book.find("M", bkIndex+1);
	while(bkIndex >= 0 && book.at(bkIndex+1)-48 >= moveTree->current->turn && moveTree->current->id.find(book.substr(bkIndex+3,4)) != 0){
		bkIndex = book.find("M", bkIndex+1);
	}*/
	/*if(index < 0){
		onBook = false;
		return NULL;
	}*/

	index = book.find("M", index+1);
	while(index > 0 && book.at(index+1)-48 > moveTree->current->turn){
		if(book.at(index+1)-48-1 == moveTree->current->turn){
			Move * tmp = moveTree->current->getChoice(book.substr(index+3,4));
			if(tmp != NULL){

			int wwins = 0;
			int bwins = 0;
			int draws = 0;

			int st = book.find(":",index + 1);
			int en = book.find(":", st+1);
			st = en;
			en = book.find(":", st+1);
			string counts = book.substr(st+1, en-st-1);
			wwins = stoi(counts);
			st=book.find(":", st+1);
			en=book.find(":", st+1);
			counts = book.substr(st+1, en-st-1);
			bwins = stoi(counts);
			st=book.find(":", st+1);
			en=book.find("M", st+1);
			counts = book.substr(st+1, en-st-1);
			draws = stoi(counts);
			int total = wwins + bwins + draws;
			tmp->bookWins = .5*draws + (tmp->turn%2 == WHITE?bwins:wwins);
			tmp->bookTotal = total;
			moveTree->current->nxtBootTotal+=total;
			}
		}
		index = book.find("M", index+1);
	}

	if(moveTree->current->nxtBootTotal == 0)
		onBook = false;

	if(!onBook)
		return NULL;

	moveTree->current->sortBkScores();

	if(post){
		string think = "0 0 0 0 (";
		for(int i = 0; i < 5; i++){
			if(i > 0)
				think.append(", ");
			think.append(moveTree->current->choices[i]->id);
			think.append(" ");
			think.append(to_string((long double)((int)(.5+((long double) moveTree->current->choices[i]->bookWins/moveTree->current->nxtBootTotal * 1000)))/10));			
			think.append("%");
		}
		think.append(")");
		handleOutput(think);
	}

	return moveTree->current->choices[0];
}