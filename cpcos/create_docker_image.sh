#!/usr/bin/env bash

# toggle to debug the script
# options could be found by 'set help'
set -o errexit
#set -o xtrace
#set -o verbose

#### global variables ####
ABSOLUTE_FILENAME=`readlink -e "$0"`
PROEJCT_ROOT=$(dirname ${ABSOLUTE_FILENAME})

CONANFILE_CRC32=$(crc32 ${PROEJCT_ROOT}/conanfile.txt)

echo "crc32 of conanfile: ${CONANFILE_CRC32}"

docker build \
    --build-arg CONANFILE_CRC32=$(crc32 conanfile.txt) \
    -t cpcos \
    -f Dockerfile \
    .
