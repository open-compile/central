echo "----------  Starting Locating DIR ----------"
ORIG_DIR=$(pwd)
echo "++ Setting JAVA_HOME ++ : /usr/lib/jvm/java-8-openjdk-amd64"
export JAVA_HOME="/usr/lib/jvm/java-8-openjdk-amd64"
echo "++ Setting CLANG_HOME ++ : /usr/lib/clangfe/clang-prebuilt/release"
export CLANG_HOME=/usr/lib/clangfe/clang-prebuilt/release
MAKE_PROTECT_PARAMS=""
DEBUG_OR_DEFAULT="DEBUG"

goto_correct_dir() {

    if test -f ./BASE_DIR
    then
	LASTTIME_DIR=$(cat BASE_DIR)
	echo "FOUND BASE_DIR : $LASTTIME_DIR"
    fi

    ./ci/setup-env.sh

    #Determine Time
    TZ='Asia/Chongqing'
    DT_PFX=$(date +"%F")
    DT_PTP=$(date +"%H-%M-%S")

    echo " ----------  Creating new dir ------------"
    if [[ "$LASTTIME_DIR" != "" ]]
    then
	cd "$LASTTIME_DIR"
	echo " ------- USING $LASTTIME_DIR --------"
    else
	# dir : ~/occ-build/
	echo " ----  Creating New Dir Based On Time %F/%H_%M_%S --------"
	mkdir ~/occ-build
	mkdir ~/occ-build/"$DT_PFX"
	mkdir ~/occ-build/"$DT_PFX/$DT_PTP"
	cd ~/occ-build/"$DT_PFX/$DT_PTP"
	pwd > $ORIG_DIR/BASE_DIR
	pwd > ./BASE_DIR
	LASTTIME_DIR=$(pwd)
	cp -rf $ORIG_DIR/ci $LASTTIME_DIR/ci # This will copy only util related files
    cp -rf $ORIG_DIR $LASTTIME_DIR/occ # This will copy all files
    fi

    echo " ============= Working  DIR =================="
    pwd
    OCC_INSTALL=$(pwd)/install
    echo " ============= INSTALL DIR ==================="
    echo ${OCC_INSTALL}
}

set_special_modes () {
    if [ -z ${BUILD_TYPE} ] ; then
	echo "[Set-Special-Modes] No mode is present, assuming on-push"
	## TODO: Identify the pushed branch
	return;
    fi
    
    if [ "RELEASE" = ${BUILD_TYPE} ] ; then
	echo "-- Running Core Build (Release) --"
	DEBUG_OR_DEFAULT="DEFAULT";
    elif [ "NIGHTLY" = ${BUILD_TYPE} ] ; then
	echo "NIGHTLY" > ./NIGHTLY
	echo "-- Running Core.Nightly Build --"
	echo "-- Writing to ./NIGHTLY -- "
	echo "-- Removing all Cache (Nightly Build) --------"
	rm -rf ./build
    elif [ "PRO_DFA" = ${BUILD_TYPE} ] ; then
	MAKE_PROTECT_PARAMS="BUILD_WITH_PROTECT=YES"
	echo "== Setting up MAKE_PROTECT_PARAMS =="
	mkdir ./install
	echo "== Writing to ./install/WITH_PROTECT"
	echo "PROTECT" > ./install/WITH_PROTECT
    elif [ "PRO_ON" = ${BUILD_TYPE} ] ; then
	MAKE_PROTECT_PARAMS="BUILD_WITH_PROTECT_VSA=YES"
	echo "== Setting up MAKE_PROTECT_PARAMS =="
	mkdir ./install
	echo "== Writing to ./install/WITH_PROTECT"
	echo "PROTECT" > ./install/WITH_PROTECT
    elif [ "PRO_RELEASE" = ${BUILD_TYPE} ] ; then
	MAKE_PROTECT_PARAMS="BUILD_WITH_PROTECT_VSA=YES"
	echo "== Setting up MAKE_PROTECT_PARAMS = YES ==="
	echo "== Setting up DEFAULT_OR_DEBUG to DEFAULT"
	DEBUG_OR_DEFAULT="DEFAULT";
	mkdir ./install
	echo "== Writing to ./install/WITH_PROTECT"
	echo "PROTECT" > ./install/WITH_PROTECT
    elif [ "JFEPLUGIN" = ${BUILD_TYPE} ] ; then
	echo "JFEPLUGIN" > ./JFEPLUGIN
	echo "-- [Set-Special-Modes] Mode is set to JFEPLUGIN --"
	echo "-- Wrting to ./NIGHTLY --"
	echo "-- Removing all Cache (Nightly Build) --------"
	rm -rf ./build
    else
	rm -rf ./build
	echo "Removing all caches"
	echo "[Set-Special-Modes] Mode is unrecognizable ${BUILD_TYPE}, assuming on-push"
	## TODO: Identify the pushed branch
	return;
    fi


}

envsave() {
    mkdir ./logs
    printenv > ./logs/printenv.log
}

# ======================================
#  Setting up special modes for
#  1. Core-Nightly
#  2. Rule-based
#  3. Protection (Compile-time on, Runtime-on)
#  4. Protection (Compile-time on, Runtime-off)
# ======================================
checkout_correct_branch() {
    cd ./occ
    export CMT_ID=$(git log --format=%h -n 1)
    export CMT_USER=$(git log --format=%an -n 1)
    export CMT_TIME=$(git log --format=%aI -n 1)
    git status > ../logs/git.status.log
    git log --oneline --graph > ../logs/git.history.log
    cd ..
}

run_build_and_install() {

    echo "======= Starting to Build the OCC ========"
    mkdir ./build
    cd ./build
    echo "=======   Running Configure  =========="
    echo "[CMD] $LASTTIME_DIR/occ/configure ${OCC_INSTALL} 1>../logs/configure.log 2>../logs/configure.err.log"
    $LASTTIME_DIR/occ/configure ${OCC_INSTALL} 1>../logs/configure.log 2>../logs/configure.err.log
    echo "==========   Running Make   =========="
    echo " =+= [CMD] make -j 8 ${MAKE_PROTECT_PARAMS} --silent build 2>../logs/make.err.log 1>../logs/make.log "
    make -j 8 ${MAKE_PROTECT_PARAMS} --silent all 2>../logs/make.err.log 1>../logs/make.log
    RET=$?
    if [ ${RET} -ne 0 ] ; then
	echo "Build the project end with ${RET}";
	exit 1
    fi
    pwd
    echo "======= Displaying All Files =========="
    du . >../logs/du.log
    echo " ------------- Find . ---------------"
    find . >../logs/find.log
    echo "======= Running Make Install =========="
    echo "[CMD] make install 1>../logs/make.install.log 2>../logs/make.install.err.log "
    make install 1>../logs/make.install.log 2>../logs/make.install.err.log
    RET=$?
    if [ ${RET} -ne 0 ] ; then
	echo "Install the project end with ${RET}";
	exit 1
    fi
    echo "Make install returned zero, continue..."
    archive_and_upload;
    echo "======= Make Install Finished ========="
    echo "BUILD_FINISHED" > $LASTTIME_DIR/BUILD_FINISHED
    rm -rf $LASTTIME_DIR/build
    
}

archive_and_upload() {
    echo "======== Start archiving the install dir ==========="
    saved_dir=$(pwd)
    mkdir -p ~/occ-build/latest/ 2>/dev/null
    cd ~/occ-build/latest/
    LATEST_BUILD_STORAGE_DIR=$(pwd)
    cd ${saved_dir}
    work_base_dir=$(pwd)
    echo "Inside build dir, exiting to workdir"
    cd ..
    echo "Under $(pwd)"
    result_tarball_name="${work_base_dir}/xvsa-$(date +%F)-${CMT_ID}.tgz"
    echo "Taring to ${result_tarball_name}"
    tar zcvf "${result_tarball_name}" install
    rm -f ${LATEST_BUILD_STORAGE_DIR}/xvsa-*.tgz
    mv ${result_tarball_name} ${LATEST_BUILD_STORAGE_DIR}
    cd ${work_base_dir}
    echo "======== Finished archiving the install dir ==========="
}

final_tips() {
    echo "======= Build Script Finished! ========"
    echo " For Debugging, enter the data dir     "
    echo " cd $LASTTIME_DIR                      "
    echo " To redo the build process             "
    echo " ./ci/debug.sh                      "
    echo " To add the installed opencc to PATH   "
    echo " ./ci/reuse.sh                      "
    echo " To run sanity tests                   "
    echo " ./ci/sanity.sh                     "
    echo " To run cti tests                      "
    echo " ./ci/cti.sh                        "
    echo " See the running results via HTTP at   "
    echo " http://compiler.anitago.com/occ-build/"
    echo " See the testing results via HTTP at   "
    echo " http://cti.anitago.com/CTI/"
    echo "---------------------------------------"
}



echo "Starting to run "$(date)
goto_correct_dir;
envsave;
set_special_modes;
checkout_correct_branch;
run_build_and_install;
echo "Building finished " $(date)
final_tips;
