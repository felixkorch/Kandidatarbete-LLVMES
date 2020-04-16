#!/bin/sh

if ! $(docker images | grep -e "^${1}\b" >/dev/null); then
    echo "Image not found: $1"
    exit 1
fi

git_tree=$(git rev-parse --show-toplevel)

docker run \
       --mount src=${git_tree},target=/root/llvmes,type=bind \
       --security-opt seccomp=unconfined \
       --workdir '/root/llvmes' \
       -it $1 \
       /bin/bash
