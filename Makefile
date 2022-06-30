#!bin/bash
.PHONY: clean

CC := gcc
OBJDIR := objdir
OBJS := $(addprefix $(OBJDIR)/,buffer.o)
LIB := -lncurses

all: out/main

out/main:	obj/main.o obj/buffer.o
	$(CC) $^ -o out/main

obj/buffer.o:	src/buffer.c
	$(CC) -c $^ -o $@

obj/main.o: src/main.c obj/buffer.o
	$(CC) -c $^ $(LIB) -o $@

clean:
	rm out/*
	rm obj/*