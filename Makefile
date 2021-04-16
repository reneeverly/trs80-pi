CXXFLAGS = -std=c++17 -Wall -Wextra
LIBRARYFLAGS = -lpthread

# check for if lstdc++fs is needed
CHECKSTDCPPFS=$(find / -name libstdc++fs*)
ifneq (,$(CHECKSTDCPPFS))
   LIBRARYFLAGS = $(LIBRARYFLAGS) -lstdc++fs
endif

CC = g++
DIRS = build

all: build/menu	build/midi

build/menu: src/menu/main.cpp include/rterm.h include/rkeyboard.h
	$(CC) $(CXXFLAGS) -o build/menu src/menu/main.cpp $(LIBRARYFLAGS)

build/midi: src/midi/main.cpp include/rterm.h include/rkeyboard.h
	$(CC) $(CXXFLAGS) -o build/midi src/midi/main.cpp $(LIBRARYFLAGS)

clean:
	rm build/*

$(shell mkdir -p $(DIRS))
