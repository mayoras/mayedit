#!bin/bash

CC=gcc
SRCDIR=src
OBJDIR=obj
OBJS=$(addprefix $(OBJDIR)/,buffer.o main.o)
LIB=-lncurses
INCS=-I./
OPT=-O0
CFLAGS=-Wall -Wextra -g $(OPT) $(LIB) $(INCS)

BINARY=out/main.out

all: $(BINARY)

$(BINARY):	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o:$(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm $(BINARY)
	rm $(OBJS)

.PHONY= clean