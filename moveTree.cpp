//moveTree.cpp
//flyByKnight - Chess Engine
//Created by Edward Sandor 2012.
//Read the README for more.

#include <string>
#include "moveTree.hpp"

MoveTree::MoveTree(Game * g){
	gm  = g;
	root = NULL;
	current = NULL;
	actual = NULL;
}
Move::Move(Move * p){
	id="NULL";
	foundChoices = false;
	parent = p;
	score = 0;
	turn = parent==NULL?0:parent->turn+1;	
	
}
Move::Move(){
	id="NULL";
	foundChoices = false;
	parent = NULL;
	score = 0;
	turn = 0;
}
Move::~Move(){
	changes.clear();
	choices.clear();
}
//to implement findChoices()
