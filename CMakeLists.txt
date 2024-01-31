cmake_minimum_required(VERSION 3.24)

project( verifier
	DESCRIPTION "A tool used to verify a game's install."
	VERSION 0.1.1
)
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# RPath for Linux
set( CMAKE_SKIP_BUILD_RPATH FALSE )
set( CMAKE_BUILD_RPATH_USE_ORIGIN TRUE )
set( CMAKE_INSTALL_RPATH $ORIGIN )

# Add sources and create executable
list( APPEND ${PROJECT_NAME}_SOURCES
	"${CMAKE_CURRENT_LIST_DIR}/src/main.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/output/Output.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/output/Output.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/output/CsvOutput.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/output/CsvOutput.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/output/JsonOutput.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/output/JsonOutput.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/output/SimpleOutput.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/output/SimpleOutput.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/output/RsvOutput.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/output/RsvOutput.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/create.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/create.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/verify.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/verify.hpp"
)

add_executable( ${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} )
target_compile_definitions( ${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG> "CLI_VERSION=\"${PROJECT_VERSION}\"" )
target_compile_options( ${PROJECT_NAME} PRIVATE -flto -static )
target_link_options( ${PROJECT_NAME} PRIVATE -flto -static )

find_package( Argumentum CONFIG REQUIRED )
find_package( fmt CONFIG REQUIRED )
find_package( unofficial-hash-library CONFIG REQUIRED )
target_link_libraries( ${PROJECT_NAME} PRIVATE Argumentum::argumentum fmt::fmt unofficial::hash-library )

add_subdirectory( ui )