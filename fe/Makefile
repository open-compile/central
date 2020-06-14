# Make file for generating the .h files from bison / flex
all: scanner.o grammar.cpp grammar.hpp token.cpp
BISON = /usr/local/opt/bison/bin/bison

#test: parser.tab.o scanner.o
#	gcc -o $@ parser.tab.o scanner.o;
scanner.o: scanner.c parser.tab.h

parser.tab.c parser.tab.h: parser.y
	$(BISON) -vd parser.y

scanner.c: scanner.l
	flex -o $@ $<


grammar.cpp: grammar.y
	$(BISON) -d -o $@ $<

grammar.hpp: grammar.cpp

token.cpp: token.l grammar.hpp
	flex -o $@ $<

.PHONY: clean

clean:
	-@ rm parser.tab.c parser.tab.h scanner.c parser.output *.o

