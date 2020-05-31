# Merely a wrapper for cmake
.PHONY: all
all:
	./configure
	./build.sh

clean:
	./clean.sh
