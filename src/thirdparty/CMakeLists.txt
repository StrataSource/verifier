# Disable shared libraries
set( BUILD_SHARED_LIBS OFF )

# argumentum
set( ARGUMENTUM_BUILD_STATIC_LIBS ON CACHE INTERNAL "" )
add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/argumentum" SYSTEM )

# fmt
add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/fmt" SYSTEM )

# sourcepp
set( SOURCEPP_LIBS_START_ENABLED OFF CACHE INTERNAL "" FORCE )
set( SOURCEPP_USE_KVPP ON CACHE INTERNAL "" FORCE )
set( SOURCEPP_USE_VPKPP ON CACHE INTERNAL "" FORCE )
# We don't really need support for this, get rid of the bloat!
set( SOURCEPP_VPKPP_SUPPORT_VPK_V54 OFF CACHE INTERNAL "" FORCE )
add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/sourcepp" SYSTEM )
