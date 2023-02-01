cmake_minimum_required(VERSION 3.11)

project(TuringMachine)
add_library(turingm STATIC)

# Set some useful variables
set(SOURCES_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Set properties of project
set_target_properties(turingm PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO

	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}/debug"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}/release"

	DEBUG_POSTFIX "-d"
	OUTPUT_NAME "turingm"
)

# Global compiler defines
if(CMAKE_BUILD_TYPE STREQUAL Debug)
	target_compile_definitions(turingm PRIVATE DEBUG)
endif()

# Enable maximum compiler warnings
if(MSVC)
	target_compile_options(turingm PRIVATE /W4)
else()
	target_compile_options(turingm PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Lib sources and includes
target_include_directories(turingm PUBLIC ${SOURCES_DIRECTORY})

target_sources(turingm
	PRIVATE ${SOURCES_DIRECTORY}/Tape.cpp
	PRIVATE ${SOURCES_DIRECTORY}/Program.cpp
	PRIVATE ${SOURCES_DIRECTORY}/TuringMachine.cpp
)