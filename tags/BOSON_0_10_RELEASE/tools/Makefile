# AB: this is an extremely simple Makefile, as these are development tools only.
# I don't want to use autoconf/automake here (-->slows me down)

CC = gcc
CXX = g++

all: checktextures fixtexturesize modelstatistics stripifier gentexfont

checktextures: checktextures.c
	$(CC) checktextures.c -o checktextures -l3ds -lm

modelstatistics: modelstatistics.c
	$(CC) modelstatistics.c -o modelstatistics -l3ds -lm

fixtexturesize: fixtexturesize.cpp
	$(CXX) fixtexturesize.cpp -o fixtexturesize -I$(QTDIR)/include -L$(QTDIR)/lib -lqt-mt

stripifier: stripifier.c
	$(CC) stripifier.c -o stripifier -l3ds -lm

gentexfont: gentexfont.c
	$(CC) -o gentexfont gentexfont.c -lX11 -L/usr/X11R6/lib
