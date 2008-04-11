project(boson)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.8)

# Disallow in-source build
STRING(COMPARE EQUAL "${boson_SOURCE_DIR}" "${boson_BINARY_DIR}" insource)
if(insource)
  MESSAGE(FATAL_ERROR "boson requires an out of source build. Please create a separate build directory and run 'cmake path_to_boson [options]' there.")
endif(insource)


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" "${CMAKE_SOURCE_DIR}/cmake/modules/boson")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/boson")

set(CMAKE_BUILD_WITH_INSTALL_RPATH 1)

# Use colored output
set(CMAKE_COLOR_MAKEFILE ON)

# AB: this is for the boson-all release package. code is a subdirectory of it
#     the rest of this file (and CMakeLists.txt in subdirs) assume that
#     CMAKE_SOURCE_DIR==boson_SOURCE_DIR.
set(CMAKE_SOURCE_DIR ${boson_SOURCE_DIR})
set(CMAKE_BINARY_DIR ${boson_BINARY_DIR})

include(BosonMisc)
include(BosonMacros)



# TODO: mark as advanced
option(BOSON_LINK_STATIC
  "link statically to most libraries, like KDE and Qt"
  OFF
)
option(BOSON_USE_DEBUG_PLUGINS
  "use debugging plugins that allow to use new code without restarting boson.  only useful to developers."
  OFF
)
option(BOSON_USE_BOMEMORY
  "debug memory usage. this is a very experimental debugging option!"
  OFF
)
option(BOGL_DO_DLOPEN
  "dlopen() the OpenGL libraries instead of linking directly to them if enabled."
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
if(BOGL_DO_DLOPEN)
  set(BOGL_DO_DLOPEN 1)
else(BOGL_DO_DLOPEN)
  set(BOGL_DO_DLOPEN 0)
endif(BOGL_DO_DLOPEN)


# search packages used by KDE
## AB: FindKDE3.cmake adds compiler flags, but we don't want this.
#set(orig_c_flags ${CMAKE_C_FLAGS})
#set(orig_cxx_flags ${CMAKE_CXX_FLAGS})
#find_package(Qt3 REQUIRED)
#find_package(BosonKDE3 REQUIRED)
#find_package(Perl REQUIRED)
#set(CMAKE_C_FLAGS ${orig_c_flags})
#set(CMAKE_CXX_FLAGS ${orig_cxx_flags})
set(QT_MIN_VERSION "4.4.0")
find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)



set(BOSON_PLUGIN_INSTALL_DIR ${PLUGIN_INSTALL_DIR}/plugins/boson)




# packages required by boson
# note: we may have more deeper in the directory structure (e.g. python in
#       boson/script)
find_package(OpenGL REQUIRED)

#find_package(Lib3ds REQUIRED)
set(LIB3DS_INCLUDE_DIR "${boson_SOURCE_DIR}/lib3ds")
set(LIB3DS_LIBRARY_DIR "${boson_BINARY_DIR}/lib3ds")
set(LIB3DS_LIBRARY "lib3ds")

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})


include_directories(
  # AB: kgame MUST be the first include! otherwise an installed libkdegames
  #     version might be used
  #     -> same about CMAKE_CURRENT_SOURCE_DIR here: in case we use
  #        #include <kgame/header.h> instead of #include <header.h>
  ${CMAKE_CURRENT_SOURCE_DIR}/kgame
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE4_INCLUDES} # KDE4_INCLUDE_DIR, Qt includes, ...
)


# TODO: fix for Qt4 port
#if(BOSON_LINK_STATIC)
#  boson_use_static_qt_and_kde()
#endif(BOSON_LINK_STATIC)


# create config.h
#include(ConfigureChecks.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )


if (UNIX)
   link_directories(/usr/local/lib)
   include_directories(/usr/local/include)
endif (UNIX)


# TODO: fix for Qt4 port
#
#add_custom_target(messages)
## AB: this MUST be the FIRST target that is executed by "make messages" !
#add_custom_target(messages-recreate-boson_pot
#   COMMAND echo "" > ${CMAKE_BINARY_DIR}/po/boson.pot
#)
#add_dependencies(messages messages-recreate-boson_pot)


add_subdirectory( lib3ds )
add_subdirectory( bomemory )
add_subdirectory( math )
add_subdirectory( bobmfconverter )
add_subdirectory( bodebug )
add_subdirectory( bogl )
add_subdirectory( kgame )
add_subdirectory( ufo )
add_subdirectory( boufo )
add_subdirectory( boson )
#add_subdirectory( server )


########### install files ###############

#install_files( /include FILES kde.pot )
#install_files( /include FILES kdelibs_export.h kdemacros.h )

