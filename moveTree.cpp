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
	pawnMove=false;
	capture=NULL;
	bookWins = 0;
	bookTotal = 0;
	nxtBootTotal = 0;
}
Move::Move(){
	id="NULL";
	evaluated = false;
	parent = NULL;
	score = 0;
	adjuster = 0;
	turn = 0;
	pawnMove=false;
	capture=NULL;
	bookWins = 0;
	bookTotal = 0;
	nxtBootTotal = 0;
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
	
	if(choices.size() != 0){
	//double weight = (turn%2)==WHITE?1.0:0.0;
	//double wInc = (1.0/choices.size()) * (turn%2)==WHITE?-1.0:1.0;
	//double wInc = 0;

	sortScores();

	for(unsigned int i = 0; i < choices.size(); i++){
		//if(!adjust && choices[i]->getBest() != NULL && choices[i]->getBest()->evaluated)
			//adjust = true;
		//if(choices[i]->evaluated){
			double weight;
			weight = pow(2.5, (turn%2)==WHITE?(int) i:(int)(choices.size()-i-1));
			c+=weight;
			t += weight*(choices[i]->adjustedScore());
		//}
	}
	//cout << turn << " " << t << "  " << c << " " << t/c << endl;
	}
	if(c<0)c*=-1;
	if(c == 0)
		adjuster = score;
	else
		adjuster = t/c;
	if(parent != NULL)
		parent->updateAdjuster();
}
double Move::adjustedScore(){
	return score*.2 + adjuster*.8;
}
void Move::setScore(double s){
	score = s;
	updateAdjuster();
}
double Move::getScore(){
	return score;
}
void Move::sortBkScores(){
	for(unsigned int i = 1; i < choices.size(); i ++){
		for(unsigned int j = i; j >= 1 && choices[j]->bookWins > choices[j - 1]->bookWins; j--){
			Move * tmp = choices[j-1];
			choices[j-1] = choices[j];
			choices[j] = tmp;
		}
	}
}