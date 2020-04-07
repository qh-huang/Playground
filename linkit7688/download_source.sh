#!/bin/bash

# Bash Strict Mode by Aaron Maxell
# options could be found by 'set help'
set -o errexit # -e
set -o nounset # -u
set -o pipefail
IFS=$'\n\t'

# toggle to debug the script
set -o xtrace
set -o verbose

umask 0022

REPO_URL="https://git.openwrt.org/openwrt/openwrt.git" 
VERSION=v19.07.2

git clone ${REPO_URL} -b ${VERSION}
