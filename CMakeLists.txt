# My version, use previous version at own risk
cmake_minimum_required(VERSION 3.9)

project(CAM)
set(CMAKE_CXX_STANDARD 17) # no explicit compiler flags if possible
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

include_directories(
	${Vulkan_INCLUDE_DIRS}
	${GLM_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIRS}
)

if (EXISTS ${CMAKE_SOURCE_DIR}/.git)
	find_package(Git)
	if (GIT_FOUND)
		execute_process(
			COMMAND git rev-list start..HEAD --count
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE CAM_RE_COMMIT_NUMBER
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		execute_process(
			COMMAND git rev-parse HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE CAM_RE_VERSION
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		execute_process(
			COMMAND ${CMAKE_SOURCE_DIR}/VersionIsDirty
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE CAM_RE_IS_DIRTY
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

	if (${CAM_RE_IS_DIRTY} MATCHES 1)
			set(CAM_RE_IS_DIRTY "_DIRTY")
			MATH(EXPR CAM_RE_COMMIT_NUMBER "${CAM_RE_COMMIT_NUMBER}+1")
		else (${CAM_RE_IS_DIRTY} MATCHES 1)
			set(CAM_RE_IS_DIRTY "_CLEAN")
		endif (${CAM_RE_IS_DIRTY} MATCHES 1)
	else (GIT_FOUND)
		set(CAM_RE_VERSION 0)
		set(CAM_RE_IS_DIRTY "")
	endif (GIT_FOUND)
endif (EXISTS ${CMAKE_SOURCE_DIR}/.git)

configure_file(
	${CMAKE_SOURCE_DIR}/src/Utils/VersionNumber.hpp.in
	${CMAKE_SOURCE_DIR}/src/Utils/VersionNumber.hpp
)

set(SOURCES
	${CMAKE_SOURCE_DIR}/src/Main.cpp
	${CMAKE_SOURCE_DIR}/src/Jobs/Worker.cpp
	${CMAKE_SOURCE_DIR}/src/Jobs/WorkerPool.cpp
	${CMAKE_SOURCE_DIR}/src/Jobs/JobPool.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/Renderer.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/SDLWindow.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/VKDevice.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/VKFence.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/VKImage.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/VKImageView.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/VKInstance.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/VKQueue.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/VKSemaphore.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/VKSurface.cpp
	${CMAKE_SOURCE_DIR}/src/Renderer/VKSwapchain.cpp
	${CMAKE_SOURCE_DIR}/src/Utils/File.cpp
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -pthread -g -fexceptions -fstack-protector-all -gsplit-dwarf -fPIC -fomit-frame-pointer")

option(CAM_RE_OPTIMIZED_BUILD "Build an optimized build" ON)
option(CAM_RE_NATIVE_ARCH "Build with -march=native" ON)
option(CAM_RE_RESTRICTED_ARCH_NATIVE "Build with some features disabled to stop valgrind from breaking" ON)

if(CAM_RE_OPTIMIZED_BUILD)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -flto")
endif(CAM_RE_OPTIMIZED_BUILD)

if (CAM_RE_NATIVE_ARCH)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

	if (CAM_RE_RESTRICTED_ARCH_NATIVE)
		# breaks valgrind on Piledriver, Steamroller and Excavator AMD CPUS. See: 
		#	 https://bugs.kde.org/show_bug.cgi?id=328357
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-tbm")

		# breaks valgrind on AMD CPUs which support some AMD-specific XOP or FMA4 commands. See:
		#	 https://bugs.kde.org/show_bug.cgi?id=369053
		#	 https://bugs.kde.org/show_bug.cgi?id=369000
		#	 https://bugs.kde.org/show_bug.cgi?id=339596
		#	 https://bugs.kde.org/show_bug.cgi?id=323431
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-xop -mno-fma4")
	endif (CAM_RE_RESTRICTED_ARCH_NATIVE)
endif (CAM_RE_NATIVE_ARCH)

SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -ldl -lstdc++ -lm -fuse-ld=gold")

add_executable(CAM ${SOURCES})
target_link_libraries(CAM
	stdc++fs
	${Vulkan_LIBRARIES}
	${SDL2_LIBRARIES}
)
target_compile_options(CAM PUBLIC ${SDL2_CFLAGS_OTHER})
