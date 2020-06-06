#!/usr/bin/env bash
cd $(dirname $0)
echo "Workdir : $(pwd)"
workdir=$(pwd)

cd build || exit 2
../configure
make
make install
