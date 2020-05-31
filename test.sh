#!/usr/bin/env bash
cd $(dirname $0)
echo "Workdir : $(pwd)"

./configure

./build.sh

./build/driver/driver
echo "Testing completed."
