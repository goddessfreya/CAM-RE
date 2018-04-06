# My version, use previous version at own risk
cmake_minimum_required(VERSION 3.10)

project(CAM)
set(CMAKE_CXX_STANDARD 17) # no explicit compiler flags if possible
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
	${CMAKE_SOURCE_DIR}/src/main.cpp
	${CMAKE_SOURCE_DIR}/src/Worker.cpp
	${CMAKE_SOURCE_DIR}/src/WorkerPool.cpp
	${CMAKE_SOURCE_DIR}/src/JobPool.cpp
)

#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -Wshadow -pthread -g -O3 -fexceptions -fstack-protector-all -flto -gsplit-dwarf -fPIC -march=native -fomit-frame-pointer")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -Wshadow -pthread -g -fexceptions -fstack-protector-all -flto -gsplit-dwarf -fPIC -march=native -fomit-frame-pointer")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -ldl -lstdc++ -lm -fuse-ld=gold")

add_executable(CAM ${SOURCES})
