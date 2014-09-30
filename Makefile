# Makefile for Project 2

all: 	awget.cc awget.h ss.cc
	g++ -Wall -g -I.awget.cc -o awget
	g++ -Wall -g -I. ss.cc -o ss 

awget: 	awget.cc awget.h
	g++ -Wall -g -I.awget.cc -o awget 

ss:	ss.cc
	g++ -Wall -g -I. ss.cc -o ss

clean:
	rm -f awget
	rm -f ss
