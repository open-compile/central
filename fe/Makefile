# Make file for generating the .h files from bison / flex
all: grammar.cpp grammar.hpp token.cpp
BISON = /usr/local/opt/bison/bin/bison

grammar.cpp: grammar.y
	bison -d -o $@ $<

grammar.hpp: grammar.cpp

token.cpp: token.l grammar.hpp
	flex -o $@ $<

.PHONY: clean

clean:
	-@ rm grammar.cpp grammar.hpp

