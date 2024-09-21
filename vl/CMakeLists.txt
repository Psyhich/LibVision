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
	src/operations.cpp
)
target_include_directories(vision
	PUBLIC
		include/
)
target_link_libraries(vision
	PRIVATE
		png_static
	PUBLIC
		fmt
)
set_property(TARGET vision
	PROPERTY CXX_STANDARD 23
)
if (USE_ARCH_OPTIMIZATION)
	target_compile_options(vision
		PRIVATE
			-march=native
	)
endif()
