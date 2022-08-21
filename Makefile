#!/bin/make

CC=gcc
SRCDIR=src
OUTDIR=out
OBJDIR=obj
TESTDIR=tests
LIB=-lncurses
LIBCRIT=-lcriterion
OPT=-O0
CFLAGS=-Wall -Wextra -std=c99 -pedantic $(OPT)

$(OUTDIR)/mayedit: $(SRCDIR)/mayedit.c
	$(CC) $(CFLAGS) -o $@ $^