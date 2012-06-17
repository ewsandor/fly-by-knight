//moveTree.cpp
//flyByKnight - Chess Engine
//Created by Edward Sandor 2012.
//Read the README for more.

#include "moveTree.hpp"

MoveTree::Move::Move(Move * p){
	foundChoices = false;
	parent = p;
	score = 0;
	turn = 0;	
}
MoveTree::Move::~Move(){
	changes.clear();
	choices.clear();
}
