option(USE_MOLD "Use mold linker to speed up build" ON)


if (USE_MOLD)
	find_program(MOLD_PROGRAM mold)
	if (MOLD_PROGRAM-NOTFOUND)
		message(FATAL_ERROR "No mold linker found")
	else()
		set(CMAKE_LINKER_TYPE MOLD)
		message("Using mold linker")
	endif()
endif()
