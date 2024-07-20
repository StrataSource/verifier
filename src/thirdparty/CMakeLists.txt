# Disable shared libraries
set( BUILD_SHARED_LIBS OFF )

# argumentum
set( ARGUMENTUM_BUILD_STATIC_LIBS ON CACHE INTERNAL "" )
add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/argumentum" SYSTEM )

# fmt
add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/fmt" SYSTEM )

# sourcepp
add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/sourcepp" SYSTEM )
