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

# ===========================================================================
# 二进制 IR dump / load round-trip 测试 (Step 11 / Step 12)
# ===========================================================================
echo
echo "Start to running IR dump/load round-trip tests";

irb_dir=/tmp/iririo_test
rm -rf "$irb_dir"
mkdir -p "$irb_dir"

# Layer 2: textual round-trip — dump 后 load+再 dump, 两份 .txt 必须一致
# Layer 3: .s 端到端 — 直接编译 vs. dump-then-load 编译, .s 必须字节相同
irb_total=0
irb_pass=0
irb_fail=0

run_irb_test() {
    local f="$1"
    local base=$(basename "$f" .sy)
    ((irb_total=irb_total + 1))

    # Layer 2: textual round-trip
    compiler --feonly --dump-ir-after=fe="$irb_dir/$base.fe.irb" \
             --dump-textual "$f" >/dev/null 2>&1
    if [ ! -s "$irb_dir/$base.fe.irb" ]; then
        echo "[IRB][$base] FAIL — first dump produced empty file";
        ((irb_fail=irb_fail + 1)); return;
    fi
    compiler --load-ir="$irb_dir/$base.fe.irb" --feonly \
             --dump-ir-after=fe="$irb_dir/$base.reload.irb" \
             --dump-textual "$f" >/dev/null 2>&1
    if ! diff -q "$irb_dir/$base.fe.irb.txt" "$irb_dir/$base.reload.irb.txt" >/dev/null 2>&1; then
        echo "[IRB][$base] FAIL — textual round-trip diff";
        ((irb_fail=irb_fail + 1)); return;
    fi

    # Layer 3: .s 端到端 diff
    compiler -S "$f" -o "$irb_dir/$base.A.s" >/dev/null 2>&1
    compiler --load-ir="$irb_dir/$base.fe.irb" -S "$f" \
             -o "$irb_dir/$base.B.s" >/dev/null 2>&1
    if [ ! -s "$irb_dir/$base.A.s" ] || [ ! -s "$irb_dir/$base.B.s" ]; then
        echo "[IRB][$base] FAIL — .s output missing (A=$(stat -f%z "$irb_dir/$base.A.s" 2>/dev/null || echo 0), B=$(stat -f%z "$irb_dir/$base.B.s" 2>/dev/null || echo 0))";
        ((irb_fail=irb_fail + 1)); return;
    fi
    if ! diff -q "$irb_dir/$base.A.s" "$irb_dir/$base.B.s" >/dev/null 2>&1; then
        echo "[IRB][$base] FAIL — .s diff between direct vs. load-ir";
        ((irb_fail=irb_fail + 1)); return;
    fi
    ((irb_pass=irb_pass + 1))
    echo "[IRB][$base] OK"
}

# 选取若干 .sy 用例做 round-trip
for f in add1.sy ; do
    if [ -f "$f" ]; then
        run_irb_test "$f"
    fi
done

echo "[IRB] Summary: $irb_pass/$irb_total passed, $irb_fail failed"
if [ $irb_fail -gt 0 ]; then
    exit 3;
fi
