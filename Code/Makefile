OBJS = nsh.o parser.o shell.o builtin.o global.o
CC = g++
CFLAGS = -Wall -c -std=c++11
LFLAGS = -Wall

nsh : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o nsh

nsh.o : global.h shell.h nsh.cpp
	$(CC) $(CFLAGS) nsh.cpp

parser.o : global.h shell.h parser.cpp
	$(CC) $(CFLAGS) parser.cpp

shell.o : global.h shell.h shell.cpp
	$(CC) $(CFLAGS) shell.cpp

builtin.o : global.h shell.h builtin.cpp
	$(CC) $(CFLAGS) builtin.cpp

global.o : global.h global.h global.cpp
	$(CC) $(CFLAGS) global.cpp

