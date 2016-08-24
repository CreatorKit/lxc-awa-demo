#!/bin/bash
export MIPS_SDK="/work/openwrt/OpenWrt-SDK-0.9.4/staging_dir/target-mipsel_mips32_musl-1.1.11"
#make clean
make \
CC=mipsel-openwrt-linux-musl-gcc \
LD=mipsel-openwrt-linux-musl-gcc \
CFLAGS=-I$MIPS_SDK/usr/include \
LDFLAGS="-L$MIPS_SDK/usr/lib -lawa -llxc -lcap"

