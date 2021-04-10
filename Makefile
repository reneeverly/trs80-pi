CXXFLAGS = -std=c++11 -Wall
CC = g++
DIRS = build

all: build/menu	

build/menu: src/menu/main.cpp include/rterm.h include/rkeyboard.h
	$(CC) $(CXXFLAGS) -o build/menu src/menu/main.cpp

clean:
	rm build/*

$(shell mkdir -p $(DIRS))
