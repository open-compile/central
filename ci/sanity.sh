echo "----------------------------------------"
echo "------- Cloning   into  sanity  --------"
RUNTIME_DIR=$(pwd)
export PATH=$TMCI_SANITY_PATH # Binary dirs, multiple dirs
mkdir testing
cd testing
TESTING_DIR=$(pwd)

echo "===== Running test.sh ====="
../occ/test.sh nobuild
echo "================================="
cd ..
echo " running copying from $(pwd)"
mkdir testresult/sanity
cp -r ./testing/open64-sanity/test/*.log ./testresult/sanity/
echo "FINISHED" > ./testresult/SANITY_FINISHED
echo "================================"
echo "--- Sanity Test Finished ------"
echo "================================"
