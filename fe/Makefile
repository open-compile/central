# Make file for generating the .h files from bison / flex
all: test

test: parser.tab.o scanner.o
	gcc -o $@ parser.tab.o scanner.o;

parser.tab.c parser.tab.h: parser.y
	/usr/local/opt/bison/bin/bison -vd parser.y

scanner.c: scanner.l
	flex -o $@ $<

scanner.o: scanner.c parser.tab.h

.PHONY: clean

clean:
	-@ rm parser.tab.c parser.tab.h scanner.c parser.output *.o

