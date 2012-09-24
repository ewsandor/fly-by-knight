//main.cpp
//Fly By Knight - Chess Engine
//Created by Edward Sandor 2011-2012.
//Read the README for more.

#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include "game.hpp"
#include "main.hpp"
#include "board.hpp"
#include "moveTree.hpp"

using namespace std;

int eColor = WHITE;

Game * currentGame = new Game();
bool editMode = false;

int main(int argc, char* argv[]){

	handleOutput("feature myname=\"Fly By Knight 0.2.6\" sigint=0 sigterm=0");

	//currentGame->setupBoard();
	//currentGame->getBoard()->printBoard();

	string input = "";
	string last = input;

	for(;;){
		getline(cin, input);

		if(input.length() == 0)
			input = last;
		else
			last = input;

		handleInput(input);

		input = "";
	}


	return 0;
}

bool handleInput(string input){  


	//cin.rdbuf()->in_avail();
	// if(input.length() > 0 && input.at(input.length()-1) == '\n')
	//   input = input.substr(0,input.length()-1);

	if(!editMode){
		if(input.find("quit") == 0 || input.find("exit") == 0)                                  //exits program
			exit(0);
		else if(input.find("new") == 0)                         //reset the board play white
			currentGame->resetGame();
		else if(input.find("print") == 0)                         //draws a board
			currentGame->getBoard()->printBoard();
		else if(input.find("help") == 0)                          //dislays list of commands
			handleOutput("\nquit\nprint\nnew\ngo\nforce\nundo\nremove\nredo\nreplace\nhelp\n");
		else if(input.find("go") == 0)                             //move right now
			currentGame->playAs = currentGame->getTurn();
		else if(input.find("force") == 0 || input.find("result") == 0)                         //turn on force mode or stop play.
			currentGame->playAs = NONE;
		else if(input.find("undo") == 0){                        //go back 1 move; play same color
			currentGame->moveBack();
				currentGame->commitMove();
		}
		else if(input.find("remove") == 0){                    //go back 2 moves; play same color
			currentGame->moveBack(2);
				currentGame->commitMove();
		}
		else if(input.find("redo") == 0){                        //go foreward 1 move; play same color
			currentGame->moveForward();
				currentGame->commitMove();
		}
		else if(input.find("replace") == 0){                         //go foreward 2 move; play same color
			currentGame->moveForward(2);
				currentGame->commitMove();
		}
		else if(input.find("edit") == 0){
			currentGame->goActualLayout();
			editMode = true;
			eColor = WHITE;
		}
		else if(Board::moveFormat(input)){                   //move piece
			currentGame->goActualLayout();
			if(currentGame->move(input)){
				currentGame->commitMove();
				currentGame->endGame();
			}
			else{
				handleOutput("Illegal move: " + input); 
			}
		}
		else{                                                                     //handle unknown commands
			handleOutput("Error (unknown command): " + input);
		}
		string str = currentGame->chooseMove();
		if(str.find("...---...") != 0){  
			currentGame->goActualLayout();
			currentGame->move(str);
			handleOutput("move " + str);
			currentGame->commitMove();
			currentGame->endGame();
		}
	}
	else{
		if(input.find(".") == 0)
			editMode = false;
		else if(input.find("#") == 0)
			currentGame->clear();
		else if(input.find("print") == 0)
			currentGame->getBoard()->printBoard();
		else if(input.find("c") == 0)
			eColor = eColor == WHITE? BLACK:WHITE;
		else{          
			if(!currentGame->modSquare(input, eColor))
				handleOutput("Error (edit mode): " + input);
		}
	}

	return true;
}
bool handleOutput(string output){ 
	//cout.flush();

	cout << output << endl;
	return true;
}


