#!/bin/sh

# Stolen from: https://stackoverflow.com/questions/192249/how-do-i-parse-command-line-arguments-in-bash
POSITIONAL=()
CMAKE_OPTIONS=()
while [[ $# -gt 0 ]]; do
	key="$1"

	case $key in
		-g|--generator)
			GENERATOR="$2"
			shift # past argument
			shift # past value
		;;
		-t|--toolchain)
			TOOLCHAIN="$2"
			shift # past argument
			shift # past value
		;;
		-c|--cmake-option)
			CMAKE_OPTIONS+=("$2")
			shift # past argument
			shift # past value
		;;
		*) # unknown option
			POSITIONAL+=("$1") # save it in an array for later
			printf "Ignoring option \`$1\`\n"
			shift # past argument
		;;
	esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

if [ "$GENERATOR" = "" ]; then
	printf "Please use \"./build.sh [MANDITORY OPTIONS] [OPTIONS]\"\n"
	printf "\n"
	printf "Manditory Options:\n"
	printf "\t-g [GEN], --generator [GEN]\t\tChoose a generator (Ninja, make, ect)\n"
	printf "\n"
	printf "Options:\n"
	printf "\t-t [TOOL], --toolchain [TOOL]\t\tChoose a toolchain (gcc, llvm, ect)\n"
	printf "\t-c [OPTION], --cmake-option [OPTION]\t\tAdd an option to pass to CMAKE\n"
	printf "\n"
	printf "Examples:\n"
	printf "\tFor GCC and Make:\t./build.sh -g \"Unix Makefiles\"\n"
	printf "\tFor GCC and Ninja:\t./build.sh -g Ninja\n"
	printf "\tFor LLVM and Make:\t./build.sh -g \"Unix Makefiles\" -t llvm\n"
	printf "\tFor LLVM and Ninja:\t./build.sh -g Ninja -t llvm\n"
	exit 1
fi

COMPILE_APP=
if [ "$GENERATOR" = "Ninja" ]; then
	COMPILE_APP="ninja"
elif [ "$GENERATOR" = "Unix Makefiles" ]; then
	COMPILE_APP="make"
else
	echo Compile manually please.
fi

TOOLCHAIN_CMDS=
if [ "$TOOLCHAIN" = "llvm" ]; then
	echo Using LLVM
	export CC="clang"
	export CXX="clang++"
	TOOLCHAIN_CMDS="-D_CMAKE_TOOLCHAIN_PREFIX=llvm-"
fi

cd "${0%/*}"

# from: https://stackoverflow.com/questions/1527049/join-elements-of-an-array
function join_by { local IFS="$1"; shift; echo "$*"; }

mkdir -p "builddir-$GENERATOR-$TOOLCHAIN"
cd "builddir-$GENERATOR-$TOOLCHAIN"
# from: https://stackoverflow.com/questions/6481005/how-to-obtain-the-number-of-cpus-cores-in-linux-from-the-command-line
CORES=$(($([[ $(uname) = 'Darwin' ]] && sysctl -n hw.logicalcpu_max || lscpu -p | egrep -v '^#' | wc -l) * 2 + 1))

cmake -G "${GENERATOR}" "$(join_by " " ${CMAKE_OPTIONS[@]})" .. ${TOOLCHAIN_CMDS} && ${COMPILE_APP} -j${CORES} || exit

cd ..
