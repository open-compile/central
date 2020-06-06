#!/usr/bin/env bash
cd $(dirname $0)
echo "Workdir : $(pwd)"
main_dir=$(pwd)

if [ ! -z $1 ] && [ $1 = "nobuild" ] ; then
    echo "Skipping building ...";
else
    echo "Starting to build ...";
    ./configure
    ./build.sh
    if [ $? -ne 0 ] ; then
        echo "Building failed";
        exit 2;
    fi
    export PATH=$PATH:${main_dir}/build/install/bin
fi
echo "Start to running sanity tests";
cd ./testcase/simple
echo "Workdir : $(pwd)"
set -x;
compiler -h &> test.log 
if [ $? -ne 0 ] ; then
    echo "Testing failed";
    exit 2;
fi
compiler -S -O2 &> test.log 
if [ $? -eq 0 ] ; then
    echo "No file testing reported success";
    exit 2;
fi
compiler -S a.c &> test.log 
if [ $? -ne 0 ] ; then
    echo "[1/2] A.c testing reported error";
    exit 2;
fi
compiler b.sy -O2 -S -o b.s &> test.log 
if [ $? -ne 0 ] ; then
    echo "[2/2] b.sy testing reported error";
    exit 2;
fi
echo "[2/2] Testing completed."
