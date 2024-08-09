cmake_minimum_required(VERSION 3.29)

project(LibVision)

include(cmake/CPM.cmake)

include(cmake/AutoCCache.cmake)
include(cmake/UseMold.cmake)

CPMAddPackage("gh:fmtlib/fmt#11.0.2")
CPMAddPackage(
	NAME cxxopts
	GITHUB_REPOSITORY jarro2783/cxxopts
	VERSION 3.2.1
	OPTIONS
		"CXXOPTS_BUILD_EXAMPLES OFF"
		"CXXOPTS_BUILD_TESTS OFF"
)
CPMAddPackage(
	NAME libpng
	GITHUB_REPOSITORY pnggroup/libpng
	VERSION 1.6.43
	OPTIONS
		"PNG_SHARED OFF"
		"PNG_TESTS OFF"
		"PNG_TOOLS OFF"
)

add_library(vision
	src/filters.cpp
	src/image.cpp
	src/image_io.cpp
	src/math.cpp
)
target_link_libraries(vision
	PRIVATE
		fmt
		png_static
)
target_compile_definitions(vision
	PRIVATE
		BOUND_CHECK=${USE_BOUND_CHECK}
)
set_property(TARGET vision
	PROPERTY CXX_STANDARD 23
)

add_executable(vision_tool
	src/main.cpp
)
target_link_libraries(vision_tool
	PRIVATE
		fmt
		cxxopts
		vision
)
set_property(TARGET vision_tool
	PROPERTY CXX_STANDARD 23
)
