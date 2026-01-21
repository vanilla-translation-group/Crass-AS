set(CMAKE_SHARED_LIBRARY_SUFFIX ".loc")

file(GLOB dirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

foreach (dir ${dirs})
	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${dir}.cpp")
		message(STATUS "Language: ${dir}")
		file(GLOB_RECURSE sources "${dir}/*.cpp" "${dir}/*.def")
		add_library("${dir}" SHARED ${sources})
	endif ()
endforeach ()
