#!/usr/bin/env bash
script_dir=$(dirname $(realpath $0))
echo "Workdir=$(pwd)";

OPENCC_PREFIX=$(pwd)/install
MASTIFF_DIR=$(pwd)/mastiff

if [ -z ${DEBUG_OR_DEFAULT} ] ; then
    echo "Because no env.variable DEBUG_OR_DEFAULT found, using DEBUG mode.";
    DEBUG_OR_DEFAULT="DEBUG";
else
    echo "Using DEBUG_OR_DEFAULT=${DEBUG_OR_DEFAULT} mode.";
fi

if [ ! -d ${MASTIFF_DIR} ] ; then
    echo "Cannot locate mastiff under ../mastiff";
    echo "Which is: ${MASTIFF_DIR}";
    exit 1;
fi

if [ -n ${BUILD_TYPE} ] && [ "PRO_ON" = ${BUILD_TYPE} ] ; then
  MAKE_PROTECT_PARAMS="BUILD_WITH_PROTECT_VSA=YES"
  echo "== Setting up MAKE_PROTECT_PARAMS =="

  # This should be the protect_test artifact used in mastiff executable creation
  which protect_test &> /dev/null;
  if [ $? -ne 0 ] ; then
      echo "executable 'protect_test' is not found, you need to download it and add to PATH";
  fi

  ## This is the protect-patch-tool project cloned.
  if [ ! -d ./prerelease ] ; then
      echo "Trying ssh -T git@git.compiler.anitago.com; "
      ssh -T git@git.compiler.anitago.com;
      if [ $? -ne 0 ] ; then
	  echo "Cannot connect to git.compiler.anitago.com with ssh -T for cloning the project, and yet prerelease folder does not exist, please make sure one way is working.";
	  exit 2;
      fi
  fi
fi

echo "--------------------------------------"
echo "   install to :  $OPENCC_PREFIX       "
echo "--------------------------------------"
mkdir ./build;
cd ./build;
mkdir ./install;
echo "=======   Running Configure  =========="
echo " Silently running configure , output is in "$(pwd)"/../configure.log"
OUTPUT_CONFIG=$(${MASTIFF_DIR}/configure --host=x86_64-linux-gnu --target=x86_64-linux-gnu --disable-fortran --with-build-optimize=${DEBUG_OR_DEFAULT} --disable-multilib --prefix=$OPENCC_PREFIX 1>../configure.log 2>../configure.error.log) || exit 2
echo " Silently running make , output is in "$(pwd)"/../make.log"
make -j 8 ${MAKE_PROTECT_PARAMS} --silent 1>./make.log 2>./make.error.log
pwd
echo "======= Displaying All Files =========="
du . > du.after.make.log
echo " ------------- Find . ---------------"
find . > find.after.make.log
echo "======= Running Make Install =========="
echo " Silently running make install , output is in "$(pwd)"/../make.install.log"
make install 1>./make.install.log 2>./make.install.error.log
echo "======================================="
echo " ====== Make Install Finished ========="
echo "======================================="

if [ -n ${BUILD_TYPE} ] && [ "PRO_ON" = ${BUILD_TYPE} ] ; then
    echo "======================================="
    echo " ====== Protect patch started ========="
    echo "======================================="

    bash -x ${script_dir}/protect-test.sh;
    echo "..........."
    echo "======================================="
    echo " ====== Protect patch finished ========="
    echo "======================================="
    
fi

echo "======================================="
echo "======= Build Script Finished! ========"
echo "======================================="
echo "      "
echo "   make log is preserved in files      "
echo "   error/warn log is in build dir      "
echo "                   make.log      "
echo "                   make.error.log"
echo "                   configure.log"
echo "                   configure.error.log"
echo "                   make.install.log"
echo "                   make.install.error.log"
echo "       "
echo "---------------------------------------"
