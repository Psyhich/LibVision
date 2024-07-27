cmake_minimum_required(VERSION 3.29)

project(filtering)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(cmake/CPM.cmake)

include(cmake/AutoCCache.cmake)
include(cmake/UseMold.cmake)

CPMAddPackage("gh:fmtlib/fmt#11.0.2")

add_executable("${PROJECT_NAME}"
	main.cpp
)
target_link_libraries("${PROJECT_NAME}"
	PRIVATE
		fmt
)
set_property(TARGET "${PROJECT_NAME}"
	PROPERTY CXX_STANDARD 23
)
