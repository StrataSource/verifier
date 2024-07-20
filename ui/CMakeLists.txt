cmake_minimum_required(VERSION 3.24)
project( verifier_ui
	DESCRIPTION "A UI for the verifier."
	VERSION 0.1.0
)

# RPath for Linux
set( CMAKE_SKIP_BUILD_RPATH FALSE )
set( CMAKE_BUILD_RPATH_USE_ORIGIN TRUE )
set( CMAKE_INSTALL_RPATH $ORIGIN )

# Qt
if(WIN32 AND NOT DEFINED QT_BASEDIR)
	message(FATAL_ERROR "Please define your QT install dir with -DQT_BASEDIR=\"C:/your/qt6/here\"")
endif()
if(DEFINED QT_BASEDIR)
	string(REPLACE "\\" "/" QT_BASEDIR "${QT_BASEDIR}")

	# Add it to the prefix path so find_package can find it
	list(APPEND CMAKE_PREFIX_PATH "${QT_BASEDIR}")
	set(QT_INCLUDE "${QT_BASEDIR}/include")
	message(STATUS "Using ${QT_INCLUDE} as the Qt include directory")
endif()

# CMake has an odd policy that links a special link lib for Qt on newer versions of CMake
cmake_policy(SET CMP0020 NEW)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add sources and create executable
list(APPEND ${PROJECT_NAME}_SOURCES
	"${CMAKE_CURRENT_LIST_DIR}/res/res.qrc"
	"${CMAKE_CURRENT_LIST_DIR}/src/main.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/MainWindow.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/MainWindow.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/ReportTableModel.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/src/ReportTableModel.hpp"
)

if(WIN32)
	list(APPEND ${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_LIST_DIR}/res/icon.rc"
	)
endif()

add_executable(
	${PROJECT_NAME} WIN32
		${${PROJECT_NAME}_SOURCES}
)
target_compile_definitions( ${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG> $<$<BOOL:UNIX>:UNIX> )
set_target_properties( ${PROJECT_NAME}
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Final Qt setup
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core Qt::Gui Qt::Widgets)
target_include_directories(
	${PROJECT_NAME} PRIVATE
		"${QT_INCLUDE}"
		"${QT_INCLUDE}/QtCore"
		"${QT_INCLUDE}/QtGui"
		"${QT_INCLUDE}/QtWidgets"
)
if(WIN32 AND MSVC)
	target_link_options(
		${PROJECT_NAME} PRIVATE
			"/ENTRY:mainCRTStartup"
	)
endif()
