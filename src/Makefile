FLAGS = @g++ -Wall -Ofast -pg

GIT_SHA = $(shell git rev-parse --short HEAD 2>/dev/null)
TIMESTAMP = $(shell date +%Y%m%d_%H%M)

.PHONY: all alll main runtest gui clean cleano archive test

all: main runtest gui archive

test: all
	./test perft

archive:
	@cp main ../archive/main_$(TIMESTAMP)_$(GIT_SHA)
	@cp runtest ../archive/runtest_$(TIMESTAMP)_$(GIT_SHA)
	@cp gui ../archive/gui_$(TIMESTAMP)_$(GIT_SHA)

alll:
	$(FLAGS) -o main main.cpp game.cpp board.cpp movegen.cpp ai.cpp `pkg-config --cflags --libs gtk4`
	$(FLAGS) -o runtest runtest.cpp game.cpp board.cpp movegen.cpp ai.cpp `pkg-config --cflags --libs gtk4`
	$(FLAGS) -o gui gui.cpp game.cpp board.cpp movegen.cpp ai.cpp `pkg-config --cflags --libs gtk4`

main: main.o game.o board.o movegen.o ai.o
	$(FLAGS) -o main main.o game.o board.o movegen.o ai.o `pkg-config --cflags --libs gtk4`
runtest: runtest.o board.o movegen.o ai.o
	$(FLAGS) -o runtest runtest.o board.o movegen.o ai.o `pkg-config --cflags --libs gtk4`
gui: gui.o board.o movegen.o game.o ai.o
	$(FLAGS) -o gui gui.o board.o movegen.o game.o ai.o `pkg-config --cflags --libs gtk4`
clean: cleano
	rm main runtest gui
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
runtest.o: runtest.cpp
	$(FLAGS) -c runtest.cpp -o runtest.o 
ai.o: ai.cpp
	$(FLAGS) -c ai.cpp `pkg-config --cflags --libs gtk4`
gui.o: gui.cpp
	$(FLAGS) -c gui.cpp `pkg-config --cflags --libs gtk4`