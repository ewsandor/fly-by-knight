all: sandrich

sandrich: main.o board.o game.o piece.o
	g++ -Wall -g board.o game.o piece.o main.o -o flyByKnight

main.o: main.cpp
	g++ -Wall -g -c main.cpp

board.o: board.cpp
	g++ -Wall -g -c board.cpp

game.o: game.cpp
	g++ -Wall -g -c game.cpp

piece.o: piece.cpp
	g++ -Wall -g -c piece.cpp

clean:
	rm -rf *o flyByKnight

run:
	./flyByKnight

runxboard:
	xboard -fcp ./flyByKnight -scp ./flyByKnight -debug