//main.cpp
//Fly By Knight - Chess Engine
//Created by Edward Sandor 2011-2012.
//Read the README for more.

//#define BOOST_THREAD_LIBRARY

#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <queue>
#ifdef BOOST_THREAD_LIBRARY
#include <boost/thread.hpp>
#else
#include <pthread.h>
#endif
#include <ctime>
#include "game.hpp"
#include "main.hpp"
#include "board.hpp"
#include "moveTree.hpp"

using namespace std;

int eColor = WHITE;
queue<string> inputQueue;
queue<string> pings;

Game * currentGame = new Game();
bool editMode = false;

bool ponder = false;
bool gameStarted = false;
bool analyze = false;
bool pondB4 = false;

#ifdef BOOST_THREAD_LIBRARY
void inputQueuer(){
#else
void *inputQueuer(void*){
#endif
	for(;;){
		string input;
		getline(cin, input);
		inputQueue.push(input);
	}
}

int main(int argc, char* argv[]){

	handleOutput("feature myname=\"Fly By Knight 0.4.2-dev"
#ifdef FBK_DEBUG_BUILD
" <debug "__DATE__ " " __TIME__">"
#endif
	"\" sigint=0 sigterm=0 ping=1 time=0 colors=0");

	#ifdef BOOST_THREAD_LIBRARY
	boost::thread inputQueuerThread(inputQueuer);
	#else
	pthread_t inputQueuerThread = {0};
	pthread_create(&inputQueuerThread, nullptr, inputQueuer, nullptr);
	#endif

	string input = "";
	string last = input;

	//currentGame->recordGameToBook();
	for(;;){
		currentGame->updateClocks();
		if(inputQueue.size() > 0){
			input = inputQueue.front();

			if(input.length() == 0)
				input = last;
			else
				last = input;
			inputQueue.pop();
		}
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
		if(input.size() == 0);
		else if(input.find("quit") == 0)                                  //exits program
			exit(0);
		else if(input.find("new") == 0)                         //reset the board play white
		{
			currentGame->resetGame();
			gameStarted = false;
		}
		else if(input.find("print") == 0)                         //draws a board
			currentGame->getBoard()->printBoard();
		else if(input.find("help") == 0)                          //dislays list of commands
			handleOutput("\nquit\nprint\nnew\ngo\nforce\nundo\nremove\nredo\nreplace\nhelp\n");
		else if(input.find("go") == 0){                             //move right now
			currentGame->playAs = currentGame->moveTree->actual->turn%2;
			currentGame->searchClock = clock();
			gameStarted = true;
		}
		else if(input.find("force") == 0 || input.find("result") == 0){                         //turn on force mode or stop play.
			currentGame->playAs = NONE;
			if(input.find("result") == 0)
				currentGame->recordGameToBook(input.substr(7));
		}
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
		else if(input.find("ping") == 0){
			if(input.length() > 5)
				pings.push(input.substr(5));
			else
				pings.push("");
		}
		else if(input.find("hard") == 0){
			ponder = true;
		}
		else if(input.find("easy") == 0){
			ponder = false;
		}
		else if(input.find("post") == 0){
			currentGame->post = true;
		}
		else if(input.find("nopost") == 0){
			currentGame->post = false;
		}
		else if(input.find("analyze") == 0){      
			analyze = true;
			pondB4 = ponder;
			ponder = true;
			currentGame->playAs = NONE;
		}
		else if(input.find("white") == 0){      
			if(currentGame->getTurn() == BLACK)
			{
				currentGame->changeTurn();
			}
			currentGame->playAs = BLACK;
		}
		else if(input.find("black") == 0){      
			if(currentGame->getTurn() == WHITE)
			{
				currentGame->changeTurn();
			}
			currentGame->playAs = WHITE;
		}
			else if(input.find("exit") == 0){                              //exits program or analyze
			if(!analyze)
				exit(0);
			else{
				ponder = pondB4;
				analyze = false;
			}
		}
		else if(input.find(".") == 0){  
				if(currentGame->analysisQueue.size() > 0){
					string stat01 = "stat01: 0 ";
					stat01.append(std::to_string((long double)currentGame->nodes));
					stat01.append(" ");
					int depth = currentGame->analysisQueue[0]->turn - currentGame->moveTree->actual->turn;				
					stat01.append(to_string((long double)depth));
					stat01.append(" ");
					int left = 0;
					for(unsigned int i = 0; i < currentGame->analysisQueue.size(); i++){
						if(currentGame->analysisQueue[0]->turn == currentGame->analysisQueue[i]->turn)
							left++;
					}
					stat01.append(to_string((long double)left));
					stat01.append(" ");
					stat01.append(to_string((long double)currentGame->analysisQueue.size()));
					stat01.append(" ");
					stat01.append(currentGame->analysisQueue[0]->id);
					handleOutput(stat01);
				}
		}
		else if(Board::moveFormat(input)){                   //move piece
			currentGame->goActualLayout();
			if(currentGame->move(input)){
				gameStarted = true;
				currentGame->commitMove();
				currentGame->endGame();
			}
			else{
				handleOutput("Illegal move: " + input); 
			}
			//currentGame->recordGameToBook();
		}
		else if(input.find("?") == 0 && currentGame->playAs == currentGame->moveTree->actual->turn%2){
			Move * tmp = currentGame->moveTree->actual->getBest();
				if(tmp != NULL){
					handleOutput("move " + tmp->id);
					currentGame->move(tmp);
					currentGame->commitMove();
				}
			currentGame->endGame();
		}
		else if(input.find("hint") == 0){
			Move * tmp = currentGame->moveTree->actual->getBest();
				if(tmp != NULL){
					handleOutput("Hint: " + tmp->id);
				}
		}
		else if(input.find("stepp") == 0){			
			currentGame->stepAnalysis();
			currentGame->getBoard()->printBoard();
		}
		else{                                                                     //handle unknown commands
			handleOutput("Error (unknown command): " + input);
		}
		
		if(currentGame->onBook && currentGame->playAs == currentGame->moveTree->actual->turn%2){
			currentGame->move(currentGame->moveTree->actual);
			Move * tmp = currentGame->getBookMove();
			if(tmp != NULL && tmp->bookTotal > 20){
				handleOutput("move " + tmp->id);
				currentGame->move(tmp);
				currentGame->commitMove();
			}
			else
				currentGame->onBook = false;
			currentGame->endGame();
		}

		if(ponder && !editMode && gameStarted)

			currentGame->stepAnalysis();
		else if(currentGame->playAs == currentGame->moveTree->actual->turn%2)
			currentGame->stepAnalysis();

		if(currentGame->playAs == currentGame->moveTree->actual->turn%2 && currentGame->analysisQueue.size() > 0 && currentGame->analysisQueue[0]->turn - currentGame->moveTree->actual->turn >= DEPTH){
			Move * tmp = currentGame->moveTree->actual->getBest();
			if(tmp != NULL){
				handleOutput("move " + tmp->id);
				currentGame->move(tmp);
				currentGame->commitMove();
			}
			currentGame->endGame();
		}
		if(currentGame->playAs != currentGame->moveTree->actual->turn%2){
			while(pings.size() > 0){
				handleOutput("pong " + pings.front());
				pings.pop();
			}
		}
		/*string str = currentGame->chooseMove();
		if(str.find("...---...") != 0){  
			currentGame->goActualLayout();
			currentGame->move(str);
			handleOutput("move " + str);
			currentGame->commitMove();
			currentGame->endGame();
		}*/
	}
	else{
		if(input.length() > 0)
		{
			if(input.find(".") == 0)
				editMode = false;
			else if(input.find("#") == 0)
				currentGame->clear();
			else if(input.find("print") == 0)
				currentGame->getBoard()->printBoard();
			else if(input.find("c") == 0)
				eColor = (eColor == WHITE)? BLACK:WHITE;
			else{          
				if(!currentGame->modSquare(input, eColor))
					handleOutput("Error (edit mode): " + input);
			}
		}
	}

	return true;
}
bool handleOutput(string output){ 
	//cout.flush();

	cout << output << endl;
	return true;
}


