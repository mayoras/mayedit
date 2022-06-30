#!bin/bash
.PHONY: clean

CC := gcc
OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/,buffer.o main.o)
LIB := -lncurses

all: out/main

out/main:	$(OBJS)
	$(CC) $^ -o out/main

obj/buffer.o:	src/buffer.c
	$(CC) -c $^ -o $@

obj/main.o: src/main.c
	$(CC) -c $< $(LIB) -o $@

clean:
	rm out/*
	rm obj/*