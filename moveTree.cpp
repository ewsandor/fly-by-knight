//moveTree.cpp
//flyByKnight - Chess Engine
//Created by Edward Sandor 2012.
//Read the README for more.

#include <string>
#include <vector>
#include <iostream>
#include "moveTree.hpp"
#include "game.hpp"
using namespace std;

MoveTree::MoveTree(Game * g){
	gm  = g;
	root = NULL;
	current = NULL;
	actual = NULL;
}
Move::Move(Move * p){
	id="NULL";
	evaluated = false;
	parent = p;
	score = 0;
	adjuster = 0;
	turn = parent==NULL?0:parent->turn+1;	
	
}
Move::Move(){
	id="NULL";
	evaluated = false;
	parent = NULL;
	score = 0;
	adjuster = 0;
	turn = 0;
}
Move::~Move(){
	for(unsigned int i = 0; i < choices.size(); i++)
		delete choices[i];
	changes.clear();
	choices.clear();
}
Move * Move::getChoice(string mid){
	for(unsigned int i = 0; i < choices.size(); i++)
		if(mid.compare(choices[i]->id) == 0)
			return choices[i];
	return NULL;
}
void Move::sortScores(){
	for(unsigned int i = 1; i < choices.size(); i ++){
		for(unsigned int j = i; j >= 1 && choices[j]->adjustedScore() < choices[j - 1]->adjustedScore(); j--){
			Move * tmp = choices[j-1];
			choices[j-1] = choices[j];
			choices[j] = tmp;
		}
	}
}
Move * Move::getBest(){
	if(choices.size() == 0)
		return NULL;
	sortScores();
	if(turn%2 == WHITE)
		return choices[choices.size()-1];
	else
		return choices[0];
}
void Move::updateAdjuster(){
	double c = 0.0;
	double t = 0.0;
	for(unsigned int i = 0; i < choices.size(); i++){
		//if(!adjust && choices[i]->getBest() != NULL && choices[i]->getBest()->evaluated)
			//adjust = true;
		if(choices[i]->evaluated){
			c++;
			t += choices[i]->adjustedScore();
		}
	}
	//cout << turn << " " << t << "  " << c << " " << t/c << endl;
	if(c == 0)
		adjuster = score;
	else
		adjuster = t/c;
	if(parent != NULL)
		parent->updateAdjuster();
}
double Move::adjustedScore(){
	return score*.5 + adjuster*.5;
}
void Move::setScore(double s){
	score = s;
	updateAdjuster();
}
double Move::getScore(){
	return score;
}