//moveTree.hpp
//Fly By Knight - Chess Engine
//Created by Edward Sandor 2012.

#ifndef _MOVETREE_INCLUDED_
#define _MOVETREE_INCLUDED_

#include <stdlib.h>
#include <vector>

class Piece;
class Game;

struct change_t{
	Piece * moded;
	int oldLoc;
	int newLoc;
	bool captured;
	bool firstMove;
	bool ep;
};
class Move{
			private:
				bool foundChoices;
			public:
				int turn;
				double score;
				Move * parent;
				std::vector<change_t> changes;
				std::vector<Move *> choices;

				Move(Move * p);
				~Move();
				void findChoices();
};

class MoveTree{
	public: 
				MoveTree(Game * g);

		Move * root;
		Move * current;
		Game * gm;
};
#endif
