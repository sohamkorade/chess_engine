all:
	g++ -O2 -o main main.cpp game.cpp board.cpp
	g++ -O2 -o perfttest perfttest.cpp game.cpp board.cpp
clean:
	rm main perfttest