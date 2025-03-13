cmake_minimum_required(VERSION 3.24)

project( verifier
	DESCRIPTION "A tool used to verify a game's install."
	VERSION 0.3.3
)
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# Options
option( VERIFIER_BUILD_GUI "Build the verifier GUI application" OFF )

# RPath for Linux
set( CMAKE_SKIP_BUILD_RPATH FALSE )
set( CMAKE_BUILD_RPATH_USE_ORIGIN TRUE )
set( CMAKE_INSTALL_RPATH $ORIGIN )

# Enable PIE
set( CMAKE_POSITION_INDEPENDENT_CODE ON )

# Enable IPO/LTO if supported
include( CheckIPOSupported )
check_ipo_supported( RESULT VERIFIER_IPO_SUPPORTED OUTPUT VERIFIER_IPO_ERROR )
if( VERIFIER_IPO_SUPPORTED )
	set( CMAKE_INTERPROCEDURAL_OPTIMIZATION ON )
else()
	message( WARNING "IPO/LTO not supported! (${VERIFIER_IPO_ERROR})" )
endif()

# Add sources for CLI executable
list( APPEND ${PROJECT_NAME}_SOURCES
	"${CMAKE_CURRENT_LIST_DIR}/src/main.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/create.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/create.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/log.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/log.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/verify.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/verify.hpp"
)

# Create CLI executable
add_executable( ${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} )
target_compile_definitions( ${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG> "VERIFIER_VERSION=\"${PROJECT_VERSION}\"" )

# Link to CLI dependencies
include( "${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/CMakeLists.txt" )
target_link_libraries( ${PROJECT_NAME} PRIVATE Argumentum::argumentum cryptopp::cryptopp fmt::fmt sourcepp::kvpp sourcepp::vpkpp)

if (UNIX)
	set_target_properties(
		verifier PROPERTIES
			LINK_FLAGS "-static-libgcc -static-libstdc++ -static"
	)
endif()

# Create GUI executable
if( VERIFIER_BUILD_GUI )
	add_subdirectory( ui )
endif()
