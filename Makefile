# Merely a wrapper for cmake
.PHONY: all
all:
	-mkdir build
	cd ./build && ../configure
	cd ./build && ../build.sh

clean:
	cd ./build && make clean

install:
	cd build && make install