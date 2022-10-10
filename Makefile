FLAGS = @g++ -Ofast

.PHONY: all alll main perfttest gui clean cleano

all: main perfttest gui

alll:
	$(FLAGS) -o main main.cpp game.cpp board.cpp movegen.cpp ai.cpp `pkg-config --cflags --libs gtk4`
	$(FLAGS) -o perfttest perfttest.cpp game.cpp board.cpp movegen.cpp ai.cpp `pkg-config --cflags --libs gtk4`
	$(FLAGS) -o gui gui.cpp game.cpp board.cpp movegen.cpp ai.cpp `pkg-config --cflags --libs gtk4`

main: main.o game.o board.o movegen.o ai.o
	$(FLAGS) -o main main.o game.o board.o movegen.o ai.o `pkg-config --cflags --libs gtk4`
perfttest: perfttest.o board.o movegen.o 
	$(FLAGS) -o perfttest perfttest.o board.o movegen.o 
gui: gui.o board.o movegen.o game.o ai.o
	$(FLAGS) -o gui gui.o board.o movegen.o game.o ai.o `pkg-config --cflags --libs gtk4`
clean: cleano
	rm main perfttest gui
cleano:
	rm *.o

main.o: main.cpp
	$(FLAGS) -c main.cpp `pkg-config --cflags --libs gtk4`
game.o: game.cpp
	$(FLAGS) -c game.cpp `pkg-config --cflags --libs gtk4`
board.o: board.cpp
	$(FLAGS) -c board.cpp
movegen.o: movegen.cpp
	$(FLAGS) -c movegen.cpp
perfttest.o: perfttest.cpp
	$(FLAGS) -c perfttest.cpp
ai.o: ai.cpp
	$(FLAGS) -c ai.cpp `pkg-config --cflags --libs gtk4`
gui.o: gui.cpp
	$(FLAGS) -c gui.cpp `pkg-config --cflags --libs gtk4`