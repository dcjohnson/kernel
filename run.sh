#!/usr/bin/env bash

set -e

docker run --rm -v `pwd`:`pwd` -w `pwd` synapsedev/gcc-arm-none-eabi make

qemu-system-arm -m 1024 -M raspi2 -serial stdio -kernel kernel.elf
