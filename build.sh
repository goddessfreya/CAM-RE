#!/bin/sh

cd "${0%/*}"

cd builddir

export CC="ccache clang"
export CXX="ccache clang++"

cmake -G Ninja .. -D_CMAKE_TOOLCHAIN_PREFIX=llvm- && ninja -j4 "$@" || exit

cd ..
