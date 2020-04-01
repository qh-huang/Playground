#!/bin/bash

# toggle to debug the script
# options could be found by 'set help'
set -o errexit
set -o xtrace
set -o verbose

REPO_URL="git://git.archive.openwrt.org/15.05/openwrt.git" 

git clone ${REPO_URL}

