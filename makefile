all: sandrich

sandrich: main.o board.o game.o piece.o moveTree.o
	g++ -I $BOOST_ROOT -Wall -g board.o game.o piece.o main.o moveTree.o -o flyByKnight $BOOST_ROOT/stage/lib/libboost_thread-vc100-mt-1_51.lib

main.o: main.cpp
	g++ -I $BOOST_ROOT -Wall -g -c main.cpp $BOOST_ROOT/stage/lib/libboost_thread-vc100-mt-1_51.lib

board.o: board.cpp
	g++ -Wall -g -c board.cpp 

game.o: game.cpp
	g++ -Wall -g -c game.cpp

piece.o: piece.cpp
	g++ -Wall -g -c piece.cpp

moveTree.o: moveTree.cpp
	g++ -Wall -g -c moveTree.cpp

clean:
	rm -rf *o flyByKnight xboard.debug

run:
	./flyByKnight

runxboard:
	xboard -fcp ./flyByKnight -scp ./flyByKnight -debug
