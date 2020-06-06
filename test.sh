#!/usr/bin/env bash
cd $(dirname $0)
echo "Workdir : $(pwd)"
main_dir=$(pwd)

echo "Starting to build ...";
./configure
./build.sh
if [ $? -ne 0 ] ; then
    echo "Building failed";
    exit 2;
fi
echo "Start to running sanity tests";
cd ./testcase/simple
echo "Workdir : $(pwd)"
set -x;
${main_dir}/build/driver/driver -h &> test.log 
if [ $? -ne 0 ] ; then
    echo "Testing failed";
    exit 2;
fi
${main_dir}/build/driver/driver -S -O2 &> test.log 
if [ $? -eq 0 ] ; then
    echo "No file testing reported success";
    exit 2;
fi
${main_dir}/build/driver/driver -S a.c &> test.log 
if [ $? -ne 0 ] ; then
    echo "[1/2] A.c testing reported error";
    exit 2;
fi
${main_dir}/build/driver/driver b.sy -O2 -S -o b.s &> test.log 
if [ $? -ne 0 ] ; then
    echo "[2/2] b.sy testing reported error";
    exit 2;
fi
echo "[2/2] Testing completed."
