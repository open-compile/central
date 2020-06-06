DBAR="-------------------------------------------"
echo $DBAR
echo "--------- Starting Analyzes ----------"
PREVDIR=$(pwd)
if test -f ./BASE_DIR
then
    LASTTIME_DIR=$(cat BASE_DIR)
    echo "FOUND BASE_DIR : $LASTTIME_DIR"
else
    exit 2
fi
cd $LASTTIME_DIR

echo $DBAR
echo "-- Inside : $(pwd)"
# mkdir testreport
git clone http://git.compiler.anitago.com/git/xc5-sz/mastiff-baseline baseline
if test -d baseline
then
    cd baseline
    git pull
    cd ..
    echo "Found baseline"
else
    echo "Failed to clone baseline, quitting"
#    exit 3
fi

# Sanity
echo $DBAR
rm -rf ./mastiff
rm -rf ./build

echo " ---------  Sanity Starting  ----------------"
./utils/analyze/sanity.sh

if [ $? -ne 0 ]
then
    echo " --- Sanity failed, previous command:$? ! ----"
    #    exit 2
fi

if test -f ./testresult/FAILED
then
   echo " --- Sanity failed ! FAILED exists ---"
   #   exit 2
fi  

echo $DBAR
echo " ---------  CTI Starting  ----------------"
./utils/analyze/cti.sh

if test -f ./testresult/FAILED
then   
    echo " --- Sanity failed ! ---"
#    exit 2
fi  

# AliOS
echo $DBAR
echo " ---------  AliOS Starting  ----------------"
# ./utils/analyze/alios.sh
echo "Skipping ..."
if [ $? -ne 0 ]
then
    echo " --- failed at AliOS -- non-zero return ! ---"
    # exit 2
fi


if test -f ./testresult/FAILED
then   
    echo " --- AliOS failed -- FAILED file exist ! ---"
    # exit 2
fi  

echo "========== Cleanup Starting ==========="
rm -rf ./testing
rm -rf ./baseline
rm -rf ./install
rm -rf ./testresult/.xcalibyte
tail -n 100 ./logs/make.err.log > ./make.err.log.tail
rm -rf ./logs
# rm ./logs/make.err.log
echo "========== Cleanup Finished ==========="
echo "TEST_ANALYZE FINISHED" > ./STATUS
echo "TEST_ANALYZE FINISHED" > $PREVDIR/STATUS
echo " To see the testresults, goto :           "
echo " cd $LASTTIME_DIR/testresult            "
echo " find                                   "
echo "----------------------------------------"
