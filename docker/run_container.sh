#!/usr/bin/env bash

set -o errexit
# Uncomment to debug
set -o verbose
set -o xtrace


# pulled from gpal/ros_devel/xauth.sh
# Make sure processes in the container can connect to the x server
# Necessary so gazebo can create a context for OpenGL rendering (even headless)

function init_x11() {
  XAUTH=/tmp/.docker.xauth
  if [ -d $XAUTH ]
  then
      sudo rm -rf $XAUTH
  fi

  if [ ! -f $XAUTH ]
  then
      touch $XAUTH
      xauth_list=$(xauth nlist :0 | head -n 1 | sed -e 's/^..../ffff/')
      if [ ! -z "$xauth_list" ]
      then
          echo $xauth_list | xauth -f $XAUTH nmerge -
      fi
      chmod a+r $XAUTH
  fi

  XAUTH=/tmp/.docker.xauth
  if [ ! -f $XAUTH ]
  then
    xauth_list=$(xauth nlist :0 | head -n 1 | sed -e 's/^..../ffff/')
    if [ ! -z "$xauth_list" ]
    then
      echo $xauth_list | xauth -f $XAUTH nmerge -
    else
      touch $XAUTH
    fi
    chmod a+r $XAUTH
  fi

  # socat TCP-LISTEN:6000,reuseaddr,fork UNIX-CLIENT:\"$DISPLAY\" &
  xhost +
}

function prepare_docker_run_params() {
  case "$(uname -s)" in
    Darwin)
      DOCKER_RUN_PARAMETER_LIST="-it \
        --name=$CONTAINER_NAME \
        --tmpfs /tmp:exec \
        -h docker \
        -e DISPLAY=host.docker.internal:0 \
        -e QT_X11_NO_MITSHM=1 \
        -e XAUTHORITY=$XAUTH \
        -v /dataset:/dataset \
        -v /etc/group:/etc/group:ro \
        -v /private/etc/passwd:/etc/passwd:ro \
        -v /etc/shadow:/etc/shadow:ro \
        -v /private/etc/sudoers.d:/etc/sudoers.d:ro \
        -v ${HOME}:/root \
        -v /tmp/.X11-unix:/tmp/.X11-unix \
        -v $XAUTH:$XAUTH \
        -v /dev:/dev \
        --net=host \
        --privileged \
        --detach \
        --rm ${IMAGE_NAME} \
        /bin/bash"
        #-e XAUTHORITY=$XAUTH \
        #-v "$WORKSPACE_DIR":/home/gpal \
        #-v /etc/localtime:/etc/localtime:ro \
        #-v /run/udev:/run/udev:ro \
      ;;

    Linux)
      # Prepare target env
      CONTAINER_DISPLAY="0"
      # Get the DISPLAY slot
      DISPLAY_NUMBER=$(echo $DISPLAY | cut -d. -f1 | cut -d: -f2)
      # Extract current authentication cookie
      AUTH_COOKIE=$(xauth list | grep "^$(hostname)/unix:${DISPLAY_NUMBER} " | awk '{print $3}')

# Launch the container
      DOCKER_RUN_PARAMETER_LIST="-it \
        --name=$CONTAINER_NAME \
        --tmpfs /tmp:exec \
        -h docker \
        -e DISPLAY=:${CONTAINER_DISPLAY} \
        -e QT_X11_NO_MITSHM=1 \
        -e XAUTHORITY=$XAUTH \
        -v /dataset:/dataset \
        -v /etc/group:/etc/group:ro \
        -v /etc/passwd:/etc/passwd:ro \
        -v /etc/shadow:/etc/shadow:ro \
        -v /etc/sudoers.d:/etc/sudoers.d:ro \
        -v ${HOME}:/root \
        -v /tmp/.X11-unix:/tmp/.X11-unix \
        -v $XAUTH:$XAUTH \
        -v /dev:/dev \
        --net=host \
        --privileged \
        --detach \
        --rm ${IMAGE_NAME} \
        /bin/bash"
        #-e XAUTHORITY=$XAUTH \
        #-e DISPLAY=host.docker.internal:0 \        #-v "$WORKSPACE_DIR":/home/gpal \
        #-v /etc/localtime:/etc/localtime:ro \
        #-v /run/udev:/run/udev:ro \
      ;;
   CYGWIN*|MINGW32*|MSYS*)
     echo 'Currently not support MS Windows'
     ;;

   *)
     echo 'Unknown OS'
     ;;
esac
}


IMAGE_NAME=playground-env
CONTAINER_NAME=playground-env
if [ $(docker inspect -f '{{.State.Running}}' $CONTAINER_NAME) ]
then
  docker start $CONTAINER_NAME
  docker attach $CONTAINER_NAME
else
  prepare_docker_run_params
  echo $DOCKER_RUN_PARAMETER_LIST
  docker run $DOCKER_RUN_PARAMETER_LIST
  docker exec -it $CONTAINER_NAME /bin/bash
fi
