//moveTree.cpp
//flyByKnight - Chess Engine
//Created by Edward Sandor 2012.
//Read the README for more.

#include "moveTree.hpp"

MoveTree::MoveTree(Game * g){
	gm = g;
	root = new Move(NULL);
	current = root;
}
Move::Move(Move * p){
	foundChoices = false;
	parent = p;
	score = 0;
	turn = 0;	
	
}
Move::~Move(){
	changes.clear();
	choices.clear();
}
//to implement findChoices()
