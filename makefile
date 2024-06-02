CXX=gcc
CXXFLAGS= -Wall -g
LDFLAGS= -lcurses

HEADERS=shell.h
OBJECTS=shell.o
SOURCES=shell.c

myshell: shell.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean

all: myshell

clean:
	rm -f *.o myshell
