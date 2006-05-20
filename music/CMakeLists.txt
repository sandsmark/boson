project(boson-music)

CMAKE_MINIMUM_REQUIRED(VERSION 2.3.5)

# Disallow in-source build
STRING(COMPARE EQUAL "${boson-music_SOURCE_DIR}" "${boson-music_BINARY_DIR}" insource)
if(insource)
  MESSAGE(FATAL_ERROR "boson requires an out of source build. Please create a separate build directory and run 'cmake path_to_boson [options]' there.")
endif(insource)


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/boson")


# AB: TODO: include in official cmake files?
set(KDE3_XDG_APPS_DIR /share/applications)
set(KDE3_DATA_INSTALL_DIR /share/apps)
set(KDE3_LIB_INSTALL_DIR /lib)
set(KDE3_PLUGIN_INSTALL_DIR ${KDE3_LIB_INSTALL_DIR}/kde3)

# AB: we need this for bodebug. TODO: we should probably use
#                                     KDE3_DATA_INSTALL_DIR/boson/bodebug
#                                     instead
set(KDE3_CONFIG_INSTALL_DIR /share/config)
# AB: TODO: include in official cmake files? (end)



add_subdirectory( music )

