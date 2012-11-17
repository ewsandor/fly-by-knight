//game.hpp
//Fly By Knight - Chess Engine
//Created by Edward Sandor 2011-2012.

#ifndef _GAME_INCLUDED_
#define _GAME_INCLUDED_

#define WHITE 0
#define BLACK 1
#define NONE -1 

#define BREADTH  4
#define DEPTH    5

#include <stdlib.h>
#include <vector>
#include <string>
#include <ctime>

class Board;
class Piece;
class MoveTree;
class Move;
struct change_t;


class Game{
	private:  
		Piece * whiteKing;
		Piece * blackKing;
		Board * board;

	public:
		//std::vector<std::vector<change_t> > changes; //to be replaced with moveTree
		bool post;
		int nodes;
		Piece * enpasantable;
		int playAs;
		MoveTree * moveTree;
		std::vector<Move *> analysisQueue;
		int searchClock;

		Game();

		void clear();
		void resetGame();
		void setupBoard();
		void changeTurn();
		void updateClocks();
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
		void moveRoot();
		bool moveBack();
		bool moveBack(int steps);
		bool moveForward();
		bool moveForward(Move * mov);
		bool moveForward(int steps);
		bool move(Move * mov);
		bool goActualLayout();
		void commitMove();
		void findChoices(Move * mov);
		void stepAnalysis();
	private:
		bool inMate(Piece * matey);
};
#endif
