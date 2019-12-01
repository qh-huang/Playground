#!/usr/bin/env bash

MATCHES=$(docker ps -a --filter "name=^/$CONTAINER_NAME$" --format '{{.Names}}')
if [ "$MATCHES" == "$CONTAINER_NAME" ]; then
  docker stop $CONTAINER_NAME
  docker rm -f $CONTAINER_NAME
fi
