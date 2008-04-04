project(boson-data)

CMAKE_MINIMUM_REQUIRED(VERSION 2.3.5)

# Disallow in-source build
STRING(COMPARE EQUAL "${boson-data_SOURCE_DIR}" "${boson-data_BINARY_DIR}" insource)
if(insource)
  MESSAGE(FATAL_ERROR "boson requires an out of source build. Please create a separate build directory and run 'cmake path_to_boson [options]' there.")
endif(insource)


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/boson")

# generate non-quiet makefiles
set(CMAKE_VERBOSE_MAKEFILE ON)



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

set(BOSON_PLUGIN_INSTALL_DIR ${KDE3_PLUGIN_INSTALL_DIR}/plugins/boson)


# search packages used by KDE
# AB: FindKDE3.cmake adds compiler flags, but we don't want this.
set(orig_c_flags ${CMAKE_C_FLAGS})
set(orig_cxx_flags ${CMAKE_CXX_FLAGS})
find_package(Qt3 REQUIRED)
find_package(BosonKDE3 REQUIRED)
find_package(Perl REQUIRED)
set(CMAKE_C_FLAGS ${orig_c_flags})
set(CMAKE_CXX_FLAGS ${orig_cxx_flags})


add_definitions(${QT_DEFINITIONS} ${KDE3_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE3_LIB_DIR})

# AB: don't want this (KDE3_DEFINITIONS defines it)
add_definitions(-Wno-non-virtual-dtor)
remove_definitions(-Wnon-virtual-dtor)


if (UNIX)
   link_directories(/usr/local/lib)
   include_directories(/usr/local/include)
endif (UNIX)


add_subdirectory( doc )
add_subdirectory( fonts )
add_subdirectory( maps )
add_subdirectory( pics )
add_subdirectory( themes )


