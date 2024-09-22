cmake_minimum_required(VERSION 3.29)

project(LibVision)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
	set(MAIN_PROJECT TRUE)
else()
	set(MAIN_PROJECT FALSE)
endif()

option(USE_ARCH_OPTIMIZATION "Use current hardware optimiztions" OFF)
option(BUILD_BINDINGS "Build python bindings" OFF)
option(BUILD_TOOLS "Build lib vision tools" ${MAIN_PROJECT})
option(USE_SANITIZER "Use address sanitizer" OFF)

if (USE_SANITIZER)
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
endif()

if (BUILD_BINDINGS)
	set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
endif()

include(cmake/CPM.cmake)

include(cmake/AutoCCache.cmake)
include(cmake/UseMold.cmake)

CPMAddPackage(
	NAME fmt
	GITHUB_REPOSITORY fmtlib/fmt
	GIT_TAG 11.0.2
)

add_subdirectory(vl)

if (BUILD_TOOLS)
	add_subdirectory(tools)
endif()
