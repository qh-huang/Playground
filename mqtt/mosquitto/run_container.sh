#!/usr/bin/env bash

set -o errexit
# Uncomment to debug
set -o verbose
set -o xtrace

BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

function prepare_docker_run_params() {
  case "$(uname -s)" in
    Darwin)
      # Launch the container
      DOCKER_RUN_PARAMETER_LIST="-it \
        -h mosquitto \
        -p 1883:1883 \
        -p 9001:9001 \
        -v ${BASE_DIR}/mosquitto.conf:/mosquitto/config/mosquitto.conf \
        mosquitto"
      ;;

    Linux)
      # Launch the container
      DOCKER_RUN_PARAMETER_LIST="-it \
        -h mosquitto \
        -p 1883:1883 \
        -p 9001:9001 \
        -v ${BASE_DIR}/mosquitto.conf:/mosquitto/config/mosquitto.conf \
        mosquitto"
      ;;
   CYGWIN*|MINGW32*|MSYS*)
     echo 'Currently not support MS Windows'
     ;;

   *)
     echo 'Unknown OS'
     ;;
esac
}


prepare_docker_run_params
echo $DOCKER_RUN_PARAMETER_LIST
docker run $DOCKER_RUN_PARAMETER_LIST
