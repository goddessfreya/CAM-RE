# My version, use previous version at own risk
cmake_minimum_required(VERSION 3.9)

project(CAM)
set(CMAKE_CXX_STANDARD 17) # no explicit compiler flags if possible
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(
	${Vulkan_INCLUDE_DIRS}
	${GLM_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIRS}
)

set(SOURCES
	${CMAKE_SOURCE_DIR}/src/Main.cpp
	${CMAKE_SOURCE_DIR}/src/Jobs/Worker.cpp
	${CMAKE_SOURCE_DIR}/src/Jobs/WorkerPool.cpp
	${CMAKE_SOURCE_DIR}/src/Jobs/JobPool.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/Renderer.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/SDLWindow.cpp
	${CMAKE_SOURCE_DIR}/src/Utils/File.cpp
)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -pthread -g -fexceptions -fstack-protector-all -flto -gsplit-dwarf -fPIC -march=native -fomit-frame-pointer")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-tbm") # breaks valgrind on Piledriver, Steamroller and Excavator AMD CPUS, see https://bugs.kde.org/show_bug.cgi?id=328357
#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O3")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -ldl -lstdc++ -lm -fuse-ld=gold")

add_executable(CAM ${SOURCES})
target_link_libraries(CAM 
	stdc++fs
	${Vulkan_LIBRARIES}
	${SDL2_LIBRARIES}
)
