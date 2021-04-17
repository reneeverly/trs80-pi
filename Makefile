CXXFLAGS = -std=c++17 -Wall -Wextra

# check for if lstdc++fs is needed
CHECKSTDCPPFS=$(shell find /usr/lib -name libstdc++fs* 2>/dev/null)
ifneq (,$(CHECKSTDCPPFS))
   FSFLAG = -lstdc++fs
endif

LIBRARYFLAGS = -lpthread $(FSFLAG)

CC = g++
DIRS = build

all: build/menu	build/midi

build/menu: src/menu/main.cpp include/rterm.h include/rkeyboard.h include/rtui.h
	$(CC) $(CXXFLAGS) -o build/menu src/menu/main.cpp $(LIBRARYFLAGS)

build/midi: src/midi/main.cpp include/rterm.h include/rkeyboard.h include/rtui.h
	$(CC) $(CXXFLAGS) -o build/midi src/midi/main.cpp $(LIBRARYFLAGS)

clean:
	rm build/*

$(shell mkdir -p $(DIRS))
