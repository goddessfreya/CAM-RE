#!/bin/sh

generator=$1
toolchain=$2

if [ "$generator" = "" ]; then
	echo Please use "./build.sh GENERATOR [TOOLCHAIN]"
	echo Examples:
	echo For GCC and Make: ./build.sh "Unix Makefiles"
	echo For GCC and Ninja: ./build.sh Ninja
	echo For LLVM and Make: ./build.sh "Unix Makefiles" llvm
	echo For LLVM and Ninja: ./build.sh Ninja llvm
fi

toolchainCmds=
if [ "$toolchain" = "llvm" ]; then
	echo Using LLVM
	export CC="clang"
	export CXX="clang++"
	toolchainCmds="-D_CMAKE_TOOLCHAIN_PREFIX=llvm-"
fi

cd "${0%/*}"

mkdir -p "builddir-$generator-$toolchain"
cd "builddir-$generator-$toolchain"
# from: https://stackoverflow.com/questions/6481005/how-to-obtain-the-number-of-cpus-cores-in-linux-from-the-command-line
cores=$([[ $(uname) = 'Darwin' ]] && sysctl -n hw.logicalcpu_max || lscpu -p | egrep -v '^#' | wc -l)

cmake -G "${generator}" .. ${toolchainCmds} && ninja -j${cores} || exit

cd ..
