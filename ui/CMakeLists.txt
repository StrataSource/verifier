cmake_minimum_required(VERSION 3.24)
project(
	verifier-ui
		LANGUAGES CXX
		VERSION 0.1.0
		DESCRIPTION "An UI for the verifier"
)
set( CMAKE_CXX_STANDARD 23 )  # sets the C++ version we use
set( CMAKE_CXX_STANDARD_REQUIRED ON )  # makes so we _require_ the declared C++ version

# if we weren't provided an install dir, set the it inside the repo dir
if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
	set( CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "..." FORCE )
endif()

# platform-specific setup
if( UNIX )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC" )
	set( QT_INCLUDE "/usr/include/x86_64-linux-gnu/qt6" )

	if( DEFINED QT_BASEDIR )
		set( CMAKE_PREFIX_PATH "${QT_BASEDIR}/lib/cmake" )
	else()
		# On Unix, the install dir may not be specified, as it's standardized
		set( QT_BASEDIR "$ENV{HOME}/Qt/6.4.1/gcc_64/" )
	endif()
elseif( WIN32 )
	if( NOT DEFINED QT_BASEDIR )
		# On Windows, the install dir must be specified, as it's not standardized
		message( FATAL_ERROR "--!@ Please define your QT install dir with -DQT_BASEDIR=C:/your/qt6/here" )
	endif()

	# Add `QT_BASEDIR` to the prefix path so `find_package` can find it
	list( APPEND CMAKE_PREFIX_PATH ${QT_BASEDIR} )
	set( QT_INCLUDE ${QT_BASEDIR}/include )
	# CMake has an odd policy that links a special link lib for Qt on newer versions of CMake.
	cmake_policy( SET CMP0020 NEW )
endif()

set( QT_LIBS Core Gui Widgets )  # the Qt6 modules that we use
find_package( Qt6 COMPONENTS ${QT_LIBS} REQUIRED )  # find the Qt6 package and provide the buildscript with its defines
set( CMAKE_AUTOMOC ON )  # required for Qt to generate Q_OBJECT meta methods
set( CMAKE_AUTORCC ON )  # required to compile resources, might not be needed, will see
set( CMAKE_AUTOUIC ON )  # required to compile .ui files, might not be needed, will see

# the executable
add_executable( verifier-ui
	src/main.cpp
)

# setup libraries we link to
foreach( QT_LIB ${QT_LIBS} )
	target_link_libraries( verifier-ui PRIVATE Qt::${QT_LIB} )
endforeach()
target_include_directories( verifier-ui PUBLIC include )  # the directories that will be searched for files
qt_generate_moc( TARGET verifier-ui )  # make qt generate the meta methods

# setup Qt6 files for runtime, only needed for windows
if( WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE )
	set( DEBUG_SUFFIX )
	# only set the debug suffix if we're in debug and using MSVC
	if( MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug" )
		set( DEBUG_SUFFIX "d" )
	endif()

	# copy all libraries to the binaries directory
	if( EXISTS "${QT_BASEDIR}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll" )
		add_custom_command(
			TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/"
		)
		add_custom_command(
			TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy "${QT_BASEDIR}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/"
		)
	endif()
	foreach( QT_LIB ${QT_LIBS} OpenGLWidgets OpenGL )
		add_custom_command(
			TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy "${QT_BASEDIR}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
		)
	endforeach()
endif()

# setup the resource directory, so we can use the files within it
add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/resources/" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)