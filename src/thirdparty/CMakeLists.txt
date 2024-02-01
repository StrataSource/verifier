# Disable shared libraries
set( BUILD_SHARED_LIBS OFF )

# argumentum
set( ARGUMENTUM_BUILD_STATIC_LIBS ON CACHE INTERNAL "" )
add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/argumentum" )

# fmt
add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/fmt" )

# hash-library
file( GLOB HASH_LIBRARY_SOURCES "${CMAKE_CURRENT_LIST_DIR}/hash-library/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/hash-library/*.h" )
add_library( hash-library STATIC ${HASH_LIBRARY_SOURCES} )
target_include_directories( hash-library PUBLIC "${CMAKE_CURRENT_LIST_DIR}/hash-library" )
