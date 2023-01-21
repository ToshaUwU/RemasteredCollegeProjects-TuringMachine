cmake_minimum_required(VERSION 3.11)

project(libtest)
add_executable(libtest ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

set_target_properties(libtest PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO

	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin/debug"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/release"

	OUTPUT_NAME "libtest"
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(libtest turingm)