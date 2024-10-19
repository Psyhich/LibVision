find_package(PNG REQUIRED)

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
		PNG::PNG
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
