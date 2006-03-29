project(boson)

CMAKE_MINIMUM_REQUIRED(VERSION 2.3.5)

# Disallow in-source build
STRING(COMPARE EQUAL "${boson_SOURCE_DIR}" "${boson_BINARY_DIR}" insource)
if(insource)
  MESSAGE(FATAL_ERROR "boson requires an out of source build. Please create a separate build directory and run 'cmake path_to_boson [options]' there.")
endif(insource)


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" "${CMAKE_SOURCE_DIR}/cmake/modules/boson")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/boson")

# generate non-quiet makefiles
set(CMAKE_VERBOSE_MAKEFILE ON)



option(BOSON_LINK_STATIC
  "link statically to most libraries, like KDE and Qt"
  OFF
)
option(BOSON_USE_DEBUG_PLUGINS
  "use debugging plugins that allow to use new code without restarting boson.  only useful to developers."
  ON
)
option(BOSON_USE_BOMEMORY
  "debug memory usage. this is a very experimental debugging option!"
  OFF
)

if(BOSON_LINK_STATIC)
  set(BOSON_USE_DEBUG_PLUGINS OFF)
endif(BOSON_LINK_STATIC)


# AB: use "0" and "1" in defines, not "OFF" and "ON"
if(BOSON_LINK_STATIC)
  set(BOSON_LINK_STATIC 1)
else(BOSON_LINK_STATIC)
  set(BOSON_LINK_STATIC 0)
endif(BOSON_LINK_STATIC)
if(BOSON_USE_DEBUG_PLUGINS)
  set(BOSON_USE_DEBUG_PLUGINS 1)
else(BOSON_USE_DEBUG_PLUGINS)
  set(BOSON_USE_DEBUG_PLUGINS 0)
endif(BOSON_USE_DEBUG_PLUGINS)


# search packages used by KDE
find_package(Qt3 REQUIRED)
find_package(KDE3 REQUIRED)
find_package(Perl REQUIRED)

# packages required by boson
# note: we may have more deeper in the directory structure (e.g. python in
#       boson/script)
find_package(OpenGL REQUIRED)


add_definitions(${QT_DEFINITIONS} ${KDE3_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE3_LIB_DIR})

# AB: don't want this (KDE3_DEFINITIONS defines it)
add_definitions(-Wno-non-virtual-dtor)
remove_definitions(-Wnon-virtual-dtor)


include_directories(
  # AB: kgame MUST be the first include! otherwise an installed libkdegames
  #     version might be used
  #     -> same about CMAKE_CURRENT_SOURCE_DIR here: in case we use
  #        #include <kgame/header.h> instead of #include <header.h>
  ${CMAKE_CURRENT_SOURCE_DIR}/kgame
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE3_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


# create config.h
#include(ConfigureChecks.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )


if (UNIX)
   link_directories(/usr/local/lib)
   include_directories(/usr/local/include)
endif (UNIX)

add_subdirectory( bomemory )
add_subdirectory( math )
add_subdirectory( bobmfconverter )
add_subdirectory( bodebug )
add_subdirectory( bogl )
add_subdirectory( kgame )
add_subdirectory( ufo )
add_subdirectory( boson )
add_subdirectory( server )


########### install files ###############

#install_files( /include FILES kde.pot )
#install_files( /include FILES kdelibs_export.h kdemacros.h )
