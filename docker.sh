#!/usr/bin/env bash

set -e

docker run --rm -v `pwd`:`pwd` -w `pwd` synapsedev/gcc-arm-none-eabi make $@
