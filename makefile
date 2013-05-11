all: sandrich

sandrich: main.o board.o game.o piece.o moveTree.o
	g++ -I $(BOOST_ROOT) -std=c++11 -Wall -g board.o game.o piece.o main.o moveTree.o -o flyByKnight -lboost_thread

main.o: main.cpp
	g++ -I $(BOOST_ROOT) -std=c++11 -Wall -g -c main.cpp -lboost_thread 

board.o: board.cpp
	g++ -Wall -g -c board.cpp 

game.o: game.cpp
	g++ -std=c++11 -Wall -g -c game.cpp

piece.o: piece.cpp
	g++ -Wall -g -c piece.cpp

moveTree.o: moveTree.cpp
	g++ -std=c++11 -Wall -g -c moveTree.cpp

clean:
	rm -rf *o flyByKnight xboard.debug

run:
	./flyByKnight

runxboard:
	xboard -fcp ./flyByKnight -scp ./flyByKnight -debug
