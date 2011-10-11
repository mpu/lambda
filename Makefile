# Lambda build system

# Configuration
EXE      = lambda
DBGFLAGS = -DNDEBUG
CFLAGS   = -Wall -Wextra -Winline -Wunused -std=c99 -pedantic -fstrict-aliasing -g -O0 $(DBGFLAGS)
LDFLAGS  =
CC       = gcc

OBJS = parse.o subst.o eval.o main.o

# Recipes

.PHONY: default depend clean

default: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

clean:
	rm -f *.o $(EXE)

depend:
	$(CC) -MM *.c > .depend

# Dot recipes

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

# Dependencies
include .depend
