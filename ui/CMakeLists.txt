cmake_minimum_required(VERSION 3.24)
project(verifier_ui
		DESCRIPTION "A UI for the verifier."
		VERSION 0.1.0
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# RPath for Linux
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN)

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

if(WIN32)
	if(CMAKE_BUILD_TYPE MATCHES Debug)
		set(QT_LIB_SUFFIX "d" CACHE STRING "" FORCE)
	else()
		set(QT_LIB_SUFFIX "" CACHE STRING "" FORCE)
	endif()
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
		${${PROJECT_NAME}_SOURCES})

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

# Copy these so the user doesn't have to
if(WIN32)
	configure_file("${QT_BASEDIR}/bin/Qt6Core${QT_LIB_SUFFIX}.dll"    "${CMAKE_BINARY_DIR}/Qt6Core${QT_LIB_SUFFIX}.dll"    COPYONLY)
	configure_file("${QT_BASEDIR}/bin/Qt6Gui${QT_LIB_SUFFIX}.dll"     "${CMAKE_BINARY_DIR}/Qt6Gui${QT_LIB_SUFFIX}.dll"     COPYONLY)
	configure_file("${QT_BASEDIR}/bin/Qt6Widgets${QT_LIB_SUFFIX}.dll" "${CMAKE_BINARY_DIR}/Qt6Widgets${QT_LIB_SUFFIX}.dll" COPYONLY)

	configure_file("${QT_BASEDIR}/plugins/platforms/qwindows${QT_LIB_SUFFIX}.dll" "${CMAKE_BINARY_DIR}/platforms/qwindows${QT_LIB_SUFFIX}.dll" COPYONLY)
	configure_file("${QT_BASEDIR}/plugins/styles/qwindowsvistastyle${QT_LIB_SUFFIX}.dll" "${CMAKE_BINARY_DIR}/styles/qwindowsvistastyle${QT_LIB_SUFFIX}.dll" COPYONLY)
elseif(UNIX)
	configure_file("${QT_BASEDIR}/lib/libQt6Core.so.6"    "${CMAKE_BINARY_DIR}/libQt6Core.so.6"    COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libQt6Gui.so.6"     "${CMAKE_BINARY_DIR}/libQt6Gui.so.6"     COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libQt6Widgets.so.6" "${CMAKE_BINARY_DIR}/libQt6Widgets.so.6" COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libQt6DBus.so.6"    "${CMAKE_BINARY_DIR}/libQt6DBus.so.6"    COPYONLY)

	configure_file("${QT_BASEDIR}/lib/libQt6WaylandClient.so.6" "${CMAKE_BINARY_DIR}/libQt6WaylandClient.so.6" COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libQt6XcbQpa.so.6"        "${CMAKE_BINARY_DIR}/libQt6XcbQpa.so.6"        COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libicui18n.so.56"         "${CMAKE_BINARY_DIR}/libicui18n.so.56"         COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libicuuc.so.56"           "${CMAKE_BINARY_DIR}/libicuuc.so.56"           COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libicudata.so.56"         "${CMAKE_BINARY_DIR}/libicudata.so.56"         COPYONLY)

	configure_file("${QT_BASEDIR}/plugins/platforms/libqwayland-generic.so"          "${CMAKE_BINARY_DIR}/platforms/libqwayland-generic.so"          COPYONLY)
	configure_file("${QT_BASEDIR}/plugins/platforms/libqminimal.so"                  "${CMAKE_BINARY_DIR}/platforms/libqminimal.so"                  COPYONLY)
	configure_file("${QT_BASEDIR}/plugins/platforms/libqxcb.so"                      "${CMAKE_BINARY_DIR}/platforms/libqxcb.so"                      COPYONLY)
	configure_file("${QT_BASEDIR}/plugins/platformthemes/libqgtk3.so"                "${CMAKE_BINARY_DIR}/platformthemes/libqgtk3.so"                COPYONLY)
	configure_file("${QT_BASEDIR}/plugins/wayland-shell-integration/libxdg-shell.so" "${CMAKE_BINARY_DIR}/wayland-shell-integration/libxdg-shell.so" COPYONLY)
	configure_file("${QT_BASEDIR}/plugins/wayland-shell-integration/libqt-shell.so"  "${CMAKE_BINARY_DIR}/wayland-shell-integration/libqt-shell.so"  COPYONLY)
endif()
