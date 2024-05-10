#!/bin/bash

# set -x

emsdk_root=`readlink -f $HOME/emsdk`
echo "Using EMSDK ${emsdk_root}"
. ${emsdk_root}/emsdk_env.sh

# export EM_CONFIG=$HOME/.emscripten

#
# emcc --generate-config
# emcc --clear-cache
# emcc --clear-ports
#

