all:
	g++ -O2 -o main main.cpp game.cpp board.cpp
	g++ -O2 -o perfttest perfttest.cpp game.cpp board.cpp
	g++ -O2 -o gui gui.cpp game.cpp board.cpp `pkg-config --cflags --libs gtk4`
main:
	g++ -O2 -o main main.cpp game.cpp board.cpp
perfttest:
	g++ -O2 -o perfttest perfttest.cpp game.cpp board.cpp
gui:
	g++ -O2 -o gui gui.cpp game.cpp board.cpp `pkg-config --cflags --libs gtk4`
clean:
	rm main perfttest gui

.PHONY: all main perfttest gui clean