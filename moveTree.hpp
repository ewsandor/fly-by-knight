//moveTree.hpp
//Fly By Knight - Chess Engine
//Created by Edward Sandor 2012.

#ifndef _MOVETREE_INCLUDED_
#define _MOVETREE_INCLUDED_

struct change_t{
	Piece * moded;
	int oldLoc;
	int newLoc;
	bool captured;
	bool firstMove;
	bool ep;
};

class moveTree{
private: 
}
#endif
