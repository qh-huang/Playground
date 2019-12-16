#!/bin/bash

# toggle to debug the script
# options could be found by 'set help'
set -o errexit
set -o xtrace
set -o verbose

for f in repos.yaml ; do
  vcs-import --input ${f} .;
done

# update all repositories
find . -type d -name .git -exec git --git-dir={} --work-tree=$PWD/{}/.. pull --verbose \;
