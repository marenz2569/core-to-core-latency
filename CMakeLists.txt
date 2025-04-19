cmake_minimum_required(VERSION 3.22)
project(core-to-core-latency)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

include(cmake/GitSubmoduleUpdate.cmake)

# set core-to-core-latency version
find_package(Git)
if(Git_FOUND)
	_is_git(${CMAKE_CURRENT_SOURCE_DIR} IN_GIT)
endif()

SET (_CORE_TO_CORE_LATENCY_VERSION_TEMPERED OFF)

SET(_CORE_TO_CORE_LATENCY_VERSION_STRING "default" CACHE STRING "_CORE_TO_CORE_LATENCY_VERSION_STRING should not be used if you do not know what to do.")
if("${_CORE_TO_CORE_LATENCY_VERSION_STRING}" STREQUAL "default")
	if (IN_GIT)
		execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --tags --dirty
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			OUTPUT_VARIABLE _CORE_TO_CORE_LATENCY_VERSION_STRING
			)
		string(STRIP ${_CORE_TO_CORE_LATENCY_VERSION_STRING} _CORE_TO_CORE_LATENCY_VERSION_STRING)
	else()
		message(AUTHOR_WARNING "Cannot detect core-to-core-latency version.")
		SET(_CORE_TO_CORE_LATENCY_VERSION_STRING "unknown")
	endif()
endif()
add_compile_definitions(_CORE_TO_CORE_LATENCY_VERSION_STRING="${_CORE_TO_CORE_LATENCY_VERSION_STRING}")

SET(_CORE_TO_CORE_LATENCY_BUILD_YEAR "default" CACHE STRING "_CORE_TO_CORE_LATENCY_BUILD_YEAR should not be used if you do not know what to do.")
if("${_CORE_TO_CORE_LATENCY_BUILD_YEAR}" STREQUAL "default")
	if(IN_GIT)
		execute_process(COMMAND ${GIT_EXECUTABLE} show -s --format=%ci HEAD
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			OUTPUT_VARIABLE _CORE_TO_CORE_LATENCY_BUILD_YEAR
			)
		string(SUBSTRING ${_CORE_TO_CORE_LATENCY_BUILD_YEAR} 0 4 _CORE_TO_CORE_LATENCY_BUILD_YEAR)
	else()
		message(AUTHOR_WARNING "Cannot detect core-to-core-latency build year.")
		SET(_CORE_TO_CORE_LATENCY_BUILD_YEAR "unknown")
	endif()
endif()
add_compile_definitions(_CORE_TO_CORE_LATENCY_BUILD_YEAR="${_CORE_TO_CORE_LATENCY_BUILD_YEAR}")

# init git submodules
include(cmake/GitSubmoduleUpdate.cmake)
git_submodule_update()

if(NOT DEFINED ASMJIT_STATIC)
	set(ASMJIT_STATIC TRUE)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(lib/asmjit)
add_subdirectory(lib/FIRESTARTER)

include_directories(include)
include_directories(lib/cxxopts/include)

add_subdirectory(src)

add_subdirectory(test)