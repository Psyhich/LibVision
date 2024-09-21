CPMAddPackage(
	NAME cxxopts
	GITHUB_REPOSITORY jarro2783/cxxopts
	VERSION 3.2.1
	OPTIONS
		"CXXOPTS_BUILD_EXAMPLES OFF"
		"CXXOPTS_BUILD_TESTS OFF"
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
