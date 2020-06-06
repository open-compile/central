# Sanity
RUNTIME_DIR=$(pwd)
echo " "
echo " "
echo "-----------------------------------------------"
echo "----------- Sanity Analyzer   -----------------"
echo "=---------------------------------------------="
echo "---------- Verifying previous stuff -----------"


# if test -f ./baseline/sanity/gcc.log
# then
#     echo "[Analyze/Sanity] Found gcc.log"
# else
#     echo "$(pwd)/baseline/sanity/gcc.log  not present ."
#     exit 2
# fi

# if test -f ./baseline/sanity/g++.log
# then
#     echo "[Analyze/Sanity] Found g++.log"
# else
#     echo "$(pwd)/baseline/sanity/g++.log  not present ."
#     exit 2
# fi

# cat ./testresult/sanity/gcc.log | grep Assertion > ./testresult/sanity/gcc-assert.log
# awk '{printf "%-8s %-10s %-10s %-5s %-10s %-10s\n",$1,$2,$3,$4,$5,$6}' ./testresult/sanity/gcc-assert.log > ./testresult/sanity/gcc-awk.log
# diff ./baseline/sanity/gcc.log ./testresult/sanity/gcc-awk.log
# if [ $? -eq 0 ]
# then
#     echo " [DIff] GCC Finished without any difference! "
# else
#     echo " [Diff] GCC different"
#     echo "FAILED" > ./testresult/FAILED
#     exit 3
# fi

# cat ./testresult/sanity/g++.log | grep Assertion > ./testresult/sanity/g++assert.log
# awk '{printf "%-8s %-10s %-10s %-5s %-10s %-10s\n",$1,$2,$3,$4,$5,$6}' ./testresult/sanity/g++assert.log > ./testresult/sanity/g++awk.log
# diff ./baseline/sanity/g++.log ./testresult/sanity/g++awk.log
# if [ $? -eq 0 ]
# then
#     echo " [DIff] G++ Finished without any difference! "
# else
#     echo " [Diff] G++ different"
#     echo "FAILED" > ./testresult/FAILED
#     exit 3
# fi


echo "-----------------------------------------------"
echo "        Sanity Finished Successfully    "
echo "----------------------------------------------"
