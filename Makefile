CXXFLAGS = -std=c++17 -Wall
CC = g++
DIRS = build

all: build/menu	build/midi

build/menu: src/menu/main.cpp include/rterm.h include/rkeyboard.h
	$(CC) $(CXXFLAGS) -o build/menu src/menu/main.cpp

build/midi: src/midi/main.cpp include/rterm.h include/rkeyboard.h
	$(CC) $(CXXFLAGS) -o build/midi src/midi/main.cpp

clean:
	rm build/*

$(shell mkdir -p $(DIRS))
