#!/usr/bin/env bash

# toggle to debug the script
# options could be found by 'set help'
set -o errexit
#set -o xtrace
#set -o verbose

#### global variables ####
ABSOLUTE_FILENAME=`readlink -e "$0"`
PROJECT_ROOT=$(dirname ${ABSOLUTE_FILENAME})

BUILD_DIR=${PROJECT_ROOT}/build/local

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}
conan install ${PROJECT_ROOT} --build=missing
cmake -S ${PROJECT_ROOT} -B ${BUILD_DIR}
cmake --build ${BUILD_DIR}
cd ${PROJECT_ROOT}
