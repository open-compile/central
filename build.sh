#!/usr/bin/env bash
cd $(dirname $0)
echo "Workdir : $(pwd)"

cd build
make $*