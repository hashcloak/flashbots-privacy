#!/usr/bin/env bash

function build
{
    echo ARCH = $1 >> CONFIG.mine
    echo GDEBUG = >> CONFIG.mine
    make clean
    rm -R static
    mkdir static
    make -j 4 static-release
    mkdir bin
    dest=bin/`uname`-$2
    rm -R $dest
    mv static $dest
    strip $dest/*
}

echo AVX_OT = 0 >> CONFIG.mine
build '-maes -mpclmul -DCHECK_AES -DCHECK_PCLMUL -DCHECK_AVX' amd64

echo AVX_OT = 1 >> CONFIG.mine
build '-msse4.1 -maes -mpclmul -mavx -mavx2 -mbmi2 -madx -DCHECK_ADX' avx2
