.PHONY: all alll main perfttest gui clean cleano

all: main perfttest gui

alll:
	g++ -O2 -o main main.cpp game.cpp board.cpp
	g++ -O2 -o perfttest perfttest.cpp game.cpp board.cpp
	g++ -O2 -o gui gui.cpp game.cpp board.cpp `pkg-config --cflags --libs gtk4`

main: main.o game.o board.o ai.o
	g++ -O2 -o main main.o game.o board.o ai.o `pkg-config --cflags --libs gtk4`
perfttest: perfttest.o board.o
	g++ -O2 -o perfttest perfttest.o board.o
gui: gui.o board.o game.o ai.o
	g++ -O2 -o gui gui.o board.o game.o ai.o `pkg-config --cflags --libs gtk4`
clean: cleano
	rm main perfttest gui
cleano:
	rm *.o

main.o: main.cpp
	g++ -O2 -c main.cpp
game.o: game.cpp
	g++ -O2 -c game.cpp `pkg-config --cflags --libs gtk4`
board.o: board.cpp
	g++ -O2 -c board.cpp
perfttest.o: perfttest.cpp
	g++ -O2 -c perfttest.cpp
ai.o: ai.cpp
	g++ -O2 -c ai.cpp `pkg-config --cflags --libs gtk4`
gui.o: gui.cpp
	g++ -O2 -c gui.cpp `pkg-config --cflags --libs gtk4`