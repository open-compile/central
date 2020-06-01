all: test

test: parser.tab.o scanner.o
	gcc -o $@ parser.tab.o scanner.o;

parser.tab.c parser.tab.h: parser.y
	bison -vd parser.y

scanner.c: scanner.l
	flex -o $@ $<

scanner.o: scanner.c parser.tab.h

.PYHONY: clean

clean:
	-@ rm parser.tab.c parser.tab.h scanner.c parser.output *.o

