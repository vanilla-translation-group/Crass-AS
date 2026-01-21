cmake_minimum_required(VERSION 3.10)
project(Crass)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_compile_definitions(_UNICODE UNICODE)
add_compile_definitions(_CRT_NON_CONFORMING_SWPRINTFS)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

include_directories(${CMAKE_SOURCE_DIR}/common/SDK/include)
include_directories(${CMAKE_SOURCE_DIR}/common/include)

link_directories(${CMAKE_SOURCE_DIR}/common/lib)

add_library(utility_and_locale SHARED utility/utility.cpp locale/locale.cpp utility/bits.cpp utility/jcrage.cpp utility/my.cpp)
target_compile_definitions(utility_and_locale PRIVATE UTILITY_EXPORTS)
target_compile_definitions(utility_and_locale PRIVATE LOCALE_EXPORTS)
target_link_libraries(utility_and_locale shlwapi ws2_32)

add_library(io_request SHARED io_request/io_request.cpp)
target_compile_definitions(io_request PRIVATE IO_REQUEST_EXPORTS)
target_link_libraries(io_request utility_and_locale)

add_library(acui_core SHARED acui_core/cui_core.cpp acui_core/aui_core.cpp acui_core/acui_core.def)
target_compile_definitions(acui_core PRIVATE ACUI_CORE_EXPORTS)
target_link_libraries(acui_core utility_and_locale)

add_library(package_core SHARED package_core/package_core.cpp)
target_compile_definitions(package_core PRIVATE PACKAGE_CORE_EXPORTS)
target_link_libraries(package_core io_request)

add_library(resource_core SHARED resource_core/resource_core.cpp)
target_compile_definitions(resource_core PRIVATE RESOURCE_CORE_EXPORTS)
target_link_libraries(resource_core io_request)

add_subdirectory(Language)
add_subdirectory(cui)

add_executable(crage crage/crage.cpp)
target_link_libraries(crage acui_core package_core resource_core)

if (DEFINED CRASS_VERSION)
	target_compile_definitions(crage PRIVATE CRAGE_VERSION="${CRASS_VERSION}")
endif ()

string(TIMESTAMP release_time "%Y-%m-%d %H:%M")
target_compile_definitions(crage PRIVATE CRAGE_RELEASE_TIME="${release_time}")
