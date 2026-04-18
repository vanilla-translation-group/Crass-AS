set(CMAKE_SHARED_LIBRARY_SUFFIX ".cui")

file(GLOB dirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

foreach (dir ${dirs})
	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${dir}.cpp")
		message(STATUS "cui: ${dir}")
		file(GLOB_RECURSE sources "${dir}/*.cpp" "${dir}/*.def")
		string(REPLACE " " "_" dir "${dir}")
		string(REPLACE "-" "_" dir "${dir}")
		add_library("cui_${dir}" SHARED ${sources})
		set_target_properties("cui_${dir}"PROPERTIES OUTPUT_NAME "${dir}")
		target_link_libraries("cui_${dir}" utility_and_locale)
		target_link_libraries("cui_${dir}" libeay32 libz libbz2)
	endif ()
endforeach ()

# extra dependencies

target_link_libraries(cui_EntisGLS comctl32 winmm)

target_link_libraries(cui_Lucifen package_core)

target_link_libraries(cui_MainProgramHoep lz32)

target_link_libraries(cui_Majiro_Script_Engine package_core)
