#!/usr/bin/env sh

if test `uname` = "Linux"; then
    flags='cat /proc/cpuinfo'
elif test `uname` = Darwin; then
    if ! type brew; then
	echo Do you want me to install Homebrew?
	echo Press RETURN to continue or any other key to abort
	read ans
	if test "$ans"; then
	    echo Aborting
	    exit 1
	else
	    /usr/bin/env ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
	fi
    fi
    make tldr
else
    echo OS unknown
    exit 1
fi

if test "$flags"; then
    if $flags | grep -q avx2; then
	cpu=avx2
    else
	cpu=amd64
    fi

    cp -av bin/`uname`-$cpu/* .
fi

mkdir Player-Data 2> /dev/null
