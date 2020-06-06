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

run_test_fail() {
    ((ongoing=ongoing + 1))
    opts="$*"
    echo "[${ongoing}/${total}] Running to-fail test >> [compiler ${opts}]";
    compiler ${opts} &> test.log 
    ret=$?
    if [ $ret -eq 0 ] ; then
        echo "[${ongoing}/${total}] Testing of running 'compiler $opts' failed with $ret, it should have reported error";
        exit 2;
    fi
}

run_test_pass() {
    opts="$*"
    ((ongoing=ongoing + 1))
    echo "[${ongoing}/${total}] Running test >> [compiler ${opts}]";
    compiler ${opts} &> test.log 
    ret=$?
    if [ $ret -ne 0 ] ; then
        echo "[${ongoing}/${total}] Testing of running 'compiler $opts' failed with $ret, it should have returned 0";
        exit 2;
    fi
}

total=11;
ongoing=0;
run_test_pass -h;
run_test_fail -S -O2;
run_test_pass -S a.c;
run_test_pass a.c -O2 -S -o b.s;
run_test_pass -S a.c -m32;
run_test_fail -S a.c -c;
run_test_pass -S a.c -x c++;
run_test_pass -S a.c -x c;
run_test_pass -S a.c -x java;
run_test_pass -S a.c -o a.s -x c -m32;
run_test_pass -S a.c -o a.s -x c -m32 -I. -I/a/b/c;

echo "[${ongoing}/${total}] Testing completed."
