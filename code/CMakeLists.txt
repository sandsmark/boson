project(boson)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" "${CMAKE_SOURCE_DIR}/cmake/modules/boson")

## support for Dart: http://public.kitware.com/dashboard.php?name=kde
#enable_testing()
#include(CTest)

# generate non-quiet makefiles
set(CMAKE_VERBOSE_MAKEFILE ON)


# TODO: make configurable
set(BOSON_LINK_STATIC 0)
set(USE_BO_PLUGINS 1)

# search packages used by KDE
find_package(Qt3 REQUIRED)
find_package(KDE3 REQUIRED)
find_package(Perl REQUIRED)

# packages required by boson
# note: we may have more deeper in the directory structure (e.g. python in
#       boson/script)
find_package(OpenGL REQUIRED)


include (MacroAdditionalCleanFiles)
include (MacroAddFileDependencies)


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

# TODO: compile bomemory conditionally
#add_subdirectory( bomemory )
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

