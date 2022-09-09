.PHONY: all alll main perfttest gui clean cleano

all: main perfttest gui

alll:
	g++ -O2 -o main main.cpp game.cpp board.cpp
	g++ -O2 -o perfttest perfttest.cpp game.cpp board.cpp
	g++ -O2 -o gui gui.cpp game.cpp board.cpp `pkg-config --cflags --libs gtk4`

main: main.o game.o board.o
	g++ -O2 -o main main.o game.o board.o
perfttest: perfttest.o board.o
	g++ -O2 -o perfttest perfttest.o board.o
gui: gui.o board.o game.o
	g++ -O2 -o gui gui.o board.o game.o `pkg-config --cflags --libs gtk4`
clean: cleano
	rm main perfttest gui
cleano:
	rm *.o

main.o: main.cpp
	g++ -O2 -c main.cpp
game.o: game.cpp
	g++ -O2 -c game.cpp
board.o: board.cpp
	g++ -O2 -c board.cpp
perfttest.o: perfttest.cpp
	g++ -O2 -c perfttest.cpp
gui.o: gui.cpp
	g++ -O2 -c gui.cpp `pkg-config --cflags --libs gtk4`