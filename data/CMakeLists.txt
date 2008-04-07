project(boson-data)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.8)

# Disallow in-source build
STRING(COMPARE EQUAL "${boson-data_SOURCE_DIR}" "${boson-data_BINARY_DIR}" insource)
if(insource)
  MESSAGE(FATAL_ERROR "boson requires an out of source build. Please create a separate build directory and run 'cmake path_to_boson [options]' there.")
endif(insource)


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/boson")

# generate non-quiet makefiles
set(CMAKE_VERBOSE_MAKEFILE ON)


# search packages used by KDE
set(QT_MIN_VERSION "4.4.0")
find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)

set(BOSON_PLUGIN_INSTALL_DIR ${PLUGIN_INSTALL_DIR}/plugins/boson)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})


if (UNIX)
   link_directories(/usr/local/lib)
   include_directories(/usr/local/include)
endif (UNIX)


add_subdirectory( doc )
add_subdirectory( fonts )
add_subdirectory( maps )
add_subdirectory( pics )
add_subdirectory( themes )


