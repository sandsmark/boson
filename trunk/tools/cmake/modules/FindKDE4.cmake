# - Find the KDE4 include and library dirs, KDE preprocessors and define a some macros
#
# KDE4_DEFINITIONS
# KDE4_INCLUDE_DIR
# KDE4_INCLUDE_DIRS
# KDE4_LIB_DIR
# KDE4_SERVICETYPES_DIR
# KDE4_DCOPIDL_EXECUTABLE
# KDE4_DCOPIDL2CPP_EXECUTABLE
# KDE4_KCFGC_EXECUTABLE
# KDE4_FOUND
# it also adds the following macros (from KDE4Macros.cmake)
# ADD_FILE_DEPENDANCY
# KDE4_ADD_DCOP_SKELS
# KDE4_ADD_DCOP_STUBS
# KDE4_ADD_MOC_FILES
# KDE4_ADD_UI_FILES
# KDE4_ADD_KCFG_FILES
# KDE4_AUTOMOC
# KDE4_PLACEHOLDER
# KDE4_CREATE_LIBTOOL_FILE
# KDE4_CREATE_FINAL_FILE
# KDE4_ADD_KPART
# KDE4_ADD_KLM
# KDE4_ADD_EXECUTABLE
#
# _KDE4_PLATFORM_INCLUDE_DIRS is used only internally
# _KDE4_PLATFORM_DEFINITIONS is used only internally


CMAKE_MINIMUM_REQUIRED(VERSION 2.2)

#this line includes FindQt.cmake, which searches the Qt library and headers
FIND_PACKAGE(Qt4 REQUIRED)

SET(QT_AND_KDECORE_LIBS ${QT_QTCORE_LIBRARY} kdecore)

INCLUDE (MacroLibrary)

#add some KDE specific stuff


SET(KDE4_DIR               ${CMAKE_INSTALL_PREFIX})
SET(KDE4_APPS_DIR          /share/applnk)
SET(KDE4_CONFIG_DIR        /share/config)
SET(KDE4_DATA_DIR          /share/apps)
SET(KDE4_HTML_DIR          /share/doc/HTML)
SET(KDE4_ICON_DIR          /share/icons)
SET(KDE4_KCFG_DIR          /share/config.kcfg)
SET(KDE4_LIBS_HTML_DIR     /share/doc/HTML)
SET(KDE4_LOCALE_DIR        /share/locale)
SET(KDE4_MIME_DIR          /share/mimelnk)
SET(KDE4_SERVICES_DIR      /share/services)
SET(KDE4_SERVICETYPES_DIR  /share/servicetypes)
SET(KDE4_SOUND_DIR         /share/sounds)
SET(KDE4_TEMPLATES_DIR     /share/templates)
SET(KDE4_WALLPAPER_DIR     /share/wallpapers)
SET(KDE4_KCONF_UPDATE_DIR	/share/apps/kconf_update/ )
SET(XDG_APPS_DIR           /share/applications/kde)
SET(XDG_DIRECTORY_DIR      /share/desktop-directories)


# the following are directories where stuff will be installed to
#SET(KDE4_SYSCONF_INSTALL_DIR "/etc" CACHE STRING "The kde sysconfig install dir (default /etc)")
SET(KDE4_MAN_INSTALL_DIR "/man" CACHE STRING "The kde man install dir (default prefix/man/)")
SET(KDE4_INFO_INSTALL_DIR "/info" CACHE STRING "The kde info install dir (default prefix/info)")
SET(KDE4_LIB_INSTALL_DIR "/lib" CACHE STRING "The subdirectory relative to the install prefix where libraries will be installed (default is /lib)")




#now try to find some kde stuff

#are we trying to compile kdelibs ?
#then enter bootstrap mode
IF(EXISTS ${CMAKE_SOURCE_DIR}/kdecore/kglobal.h)

  MESSAGE(STATUS "Building kdelibs...")

  SET(KDE4_INCLUDE_DIR ${CMAKE_SOURCE_DIR})

  SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin )
  
  IF (WIN32)
     SET(KDE4_DCOPIDL_EXECUTABLE call ${CMAKE_SOURCE_DIR}/dcop/dcopidlng/dcopidl.bat )
     SET(LIBRARY_OUTPUT_PATH  ${EXECUTABLE_OUTPUT_PATH} )
#     SET(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib )
     # todo: copy Dlls only to ${CMAKE_BINARY_DIR}/lib
  ELSE (WIN32)
     SET(KDE4_DCOPIDL_EXECUTABLE ${CMAKE_SOURCE_DIR}/dcop/dcopidlng/dcopidl )
     SET(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib )
  ENDIF (WIN32)

  SET(KDE4_LIB_DIR ${LIBRARY_OUTPUT_PATH})
  SET(KDE4_KALYPTUS_DIR ${CMAKE_SOURCE_DIR}/dcop/dcopidlng/ )
  
  # CMAKE_CFG_INTDIR is the output subdirectory created e.g. by XCode and MSVC
  SET(KDE4_DCOPIDL2CPP_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/dcopidl2cpp )
  SET(KDE4_KCFGC_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/kconfig_compiler )

ELSE(EXISTS ${CMAKE_SOURCE_DIR}/kdecore/kglobal.h)

  # at first the KDE include direcory
  # this should better check for a header which didn't exist in KDE < 4
  FIND_PATH(KDE4_INCLUDE_DIR kurl.h
    $ENV{KDEDIR}/include
    /opt/kde/include
    /opt/kde4/include
    /usr/local/include
    /usr/include/
    /usr/include/kde
    /usr/local/include/kde
  )

  # now the KDE library directory, kxmlcore is new with KDE4
  FIND_LIBRARY(KDE4_XMLCORE_LIBRARY NAMES kxmlcore
  PATHS
    $ENV{KDEDIR}/lib
    /opt/kde/lib
    /opt/kde4/lib
    /usr/lib
    /usr/local/lib
  )

  GET_FILENAME_COMPONENT(KDE4_LIB_DIR ${KDE4_XMLCORE_LIBRARY} PATH )

  #now search for the dcop utilities
  FIND_PROGRAM(KDE4_DCOPIDL_EXECUTABLE NAME dcopidl PATHS
    $ENV{KDEDIR}/bin
    /opt/kde/bin
    /opt/kde4/bin
  )

  FIND_PATH(KDE4_KALYPTUS_DIR kalyptus
    $ENV{KDEDIR}/share/apps/dcopidl
    /opt/kde/share/apps/dcopidl
    /opt/kde4/share/apps/dcopidl
  )

  FIND_PROGRAM(KDE4_DCOPIDL2CPP_EXECUTABLE NAME dcopidl2cpp PATHS
    $ENV{KDEDIR}/bin
    /opt/kde/bin
    /opt/kde4/bin
  )

  FIND_PROGRAM(KDE4_KCFGC_EXECUTABLE NAME kconfig_compiler PATHS
    $ENV{KDEDIR}/bin
    /opt/kde/bin
    /opt/kde4/bin
  )

ENDIF(EXISTS ${CMAKE_SOURCE_DIR}/kdecore/kglobal.h)


# this is  already in cmake cvs and can be removed once we require it
IF (WIN32)
   GET_FILENAME_COMPONENT(_tmp_COMPILER_NAME ${CMAKE_CXX_COMPILER} NAME_WE)
   IF ( _tmp_COMPILER_NAME MATCHES cl )
      SET(MSVC TRUE)
   ENDIF ( _tmp_COMPILER_NAME MATCHES cl )
ENDIF (WIN32)

#####################  and now the platform specific stuff  ############################


IF(UNIX AND NOT APPLE)
   FIND_PACKAGE(X11 REQUIRED)
   SET(_KDE4_PLATFORM_INCLUDE_DIRS ${X11_INCLUDE_DIR} )
ENDIF(UNIX AND NOT APPLE)



IF (WIN32)



   IF(CYGWIN)
      MESSAGE(FATAL_ERROR "Support for Cygwin not yet implemented, please edit FindKDE4.cmake to enable it")
   ENDIF(CYGWIN)

   
   # at first find the kdewin32 library, this has to be compiled and installed before kdelibs/
   FIND_LIBRARY(KDEWIN32_LIBRARY NAMES kdewin32 PATHS )
   FIND_PATH(KDEWIN32_INCLUDE_DIR winposix_export.h )
     
   # kdelibs/win/ has to be built before the rest of kdelibs/
   # eventually it will be moved out from kdelibs/
   IF (NOT KDEWIN32_LIBRARY OR NOT KDEWIN32_INCLUDE_DIR)
      MESSAGE(FATAL_ERROR "Could not find kdewin32 library, build and install kdelibs/win/ before building kdelibs/")
   ENDIF (NOT KDEWIN32_LIBRARY OR NOT KDEWIN32_INCLUDE_DIR)

   # add the winsock2 library, using find_library or something like this would probably be better
   SET(KDEWIN32_LIBRARY ${KDEWIN32_LIBRARY} ws2_32)
   
   IF(MINGW)
      #mingw compiler
      SET(KDEWIN32_INCLUDES ${KDEWIN32_INCLUDE_DIR} ${KDEWIN32_INCLUDE_DIR}/mingw ${QT_INCLUDES})
   ELSE(MINGW)
      # msvc compiler
      SET(KDEWIN32_INCLUDES ${KDEWIN32_INCLUDE_DIR} ${KDEWIN32_INCLUDE_DIR}/msvc  ${QT_INCLUDES})

      # add the MS SDK include directory if available
      SET(MS_SDK_DIR $ENV{MSSdk})
      IF (MS_SDK_DIR)
         SET(KDEWIN32_INCLUDES ${KDEWIN32_INCLUDES} ${MS_SDK_DIR}/include )
      ENDIF (MS_SDK_DIR)
      
   ENDIF(MINGW)
   
   SET( _KDE4_PLATFORM_INCLUDE_DIRS ${KDEWIN32_INCLUDES})
   SET( QT_AND_KDECORE_LIBS ${QT_AND_KDECORE_LIBS} ${KDEWIN32_LIBRARY} )
     
   # windows, mingw
   IF(MINGW)
   #hmmm, something special to do here ?
   ENDIF(MINGW)
   
   # windows, microsoft compiler
   IF(MSVC)
      SET( _KDE4_PLATFORM_DEFINITIONS -DKDE_FULL_TEMPLATE_EXPORT_INSTANTIATION -DWIN32_LEAN_AND_MEAN -DUNICODE )
   ENDIF(MSVC)

ENDIF (WIN32)


# only on linux, but not e.g. on FreeBSD:
IF(CMAKE_SYSTEM_NAME MATCHES Linux)
  SET ( _KDE4_PLATFORM_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_GNU_SOURCE)
  SET ( CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -avoid-version -Wl,--no-undefined -lc")
  SET ( CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings -avoid-version -Wl,--no-undefined -lc")
  SET ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-long-long -ansi -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -O2 -Wformat-security -Wmissing-format-attribute -fno-common")
  SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -O2 -Wformat-security -fno-exceptions -fno-check-new -fno-common")
ENDIF(CMAKE_SYSTEM_NAME MATCHES Linux)


# works on FreeBSD, not tested on NetBSD and OpenBSD
IF(CMAKE_SYSTEM_NAME MATCHES BSD)
  SET ( _KDE4_PLATFORM_DEFINITIONS -D_GNU_SOURCE )
  SET ( CMAKE_SHARED_LINKER_FLAGS "-avoid-version -lc")
  SET ( CMAKE_MODULE_LINKER_FLAGS "-avoid-version -lc")
  SET ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-long-long -ansi -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -O2 -Wformat-security -Wmissing-format-attribute -fno-common")
  SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -O2 -Wformat-security -Wmissing-format-attribute -fno-exceptions -fno-check-new -fno-common")
ENDIF(CMAKE_SYSTEM_NAME MATCHES BSD)


# This will need to be modified later to support either Qt/X11 or Qt/Mac builds
IF(APPLE)

  SET ( _KDE4_PLATFORM_DEFINITIONS -D__APPLE_KDE__ )

  # we need to set MACOSX_DEPLOYMENT_TARGET to (I believe) at least 10.2 or maybe 10.3 to allow
  # -undefined dynamic_lookup; in the future we should do this programmatically
  # hmm... why doesn't this work?
  SET (ENV{MACOSX_DEPLOYMENT_TARGET} 10.3)

  # "-undefined dynamic_lookup" means we don't care about missing symbols at link-time by default
  # this is bad, but unavoidable until there is the equivalent of libtool -no-undefined implemented
  # or perhaps it already is, and I just don't know where to look  ;)

  SET (CMAKE_SHARED_LINKER_FLAGS "-single_module -multiply_defined suppress")
  SET (CMAKE_MODULE_LINKER_FLAGS "-multiply_defined suppress")
  #SET(CMAKE_SHARED_LINKER_FLAGS "-single_module -undefined dynamic_lookup -multiply_defined suppress")
  #SET(CMAKE_MODULE_LINKER_FLAGS "-undefined dynamic_lookup -multiply_defined suppress")

  SET (CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -fno-common -Os")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common -Os")
ENDIF(APPLE)

###########    end of platform specific stuff  ##########################


# KDE4Macros.cmake contains all the KDE specific macros
INCLUDE(KDE4Macros)


# decide whether KDE4 has been found
IF (KDE4_INCLUDE_DIR AND KDE4_LIB_DIR AND KDE4_SERVICETYPES_DIR AND KDE4_DCOPIDL_EXECUTABLE AND KDE4_DCOPIDL2CPP_EXECUTABLE AND KDE4_KCFGC_EXECUTABLE)
   SET(KDE4_FOUND TRUE)
ELSE (KDE4_INCLUDE_DIR AND KDE4_LIB_DIR AND KDE4_SERVICETYPES_DIR AND KDE4_DCOPIDL_EXECUTABLE AND KDE4_DCOPIDL2CPP_EXECUTABLE AND KDE4_KCFGC_EXECUTABLE)
   SET(KDE4_FOUND FALSE)
ENDIF (KDE4_INCLUDE_DIR AND KDE4_LIB_DIR AND KDE4_SERVICETYPES_DIR AND KDE4_DCOPIDL_EXECUTABLE AND KDE4_DCOPIDL2CPP_EXECUTABLE AND KDE4_KCFGC_EXECUTABLE)


MACRO (KDE4_PRINT_RESULTS)
   IF(KDE4_INCLUDE_DIR)
      MESSAGE(STATUS "Found KDE4 include dir: ${KDE4_INCLUDE_DIR}")
   ELSE(KDE4_INCLUDE_DIR)
      MESSAGE(STATUS "Didn't find KDE4 headers")
   ENDIF(KDE4_INCLUDE_DIR)

   IF(KDE4_LIB_DIR)
      MESSAGE(STATUS "Found KDE4 library dir: ${KDE4_LIB_DIR}")
   ELSE(KDE4_LIB_DIR)
      MESSAGE(STATUS "Didn't find KDE4 core library")
   ENDIF(KDE4_LIB_DIR)

   IF(KDE4_DCOPIDL_EXECUTABLE)
      MESSAGE(STATUS "Found KDE4 dcopidl preprocessor: ${KDE4_DCOPIDL_EXECUTABLE}")
   ELSE(KDE4_DCOPIDL_EXECUTABLE)
      MESSAGE(STATUS "Didn't find the KDE4 dcopidl preprocessor")
   ENDIF(KDE4_DCOPIDL_EXECUTABLE)

   IF(KDE4_DCOPIDL2CPP_EXECUTABLE)
      MESSAGE(STATUS "Found KDE4 dcopidl2cpp preprocessor: ${KDE4_DCOPIDL2CPP_EXECUTABLE}")
   ELSE(KDE4_DCOPIDL2CPP_EXECUTABLE)
      MESSAGE(STATUS "Didn't find the KDE4 dcopidl2cpp preprocessor")
   ENDIF(KDE4_DCOPIDL2CPP_EXECUTABLE)

   IF(KDE4_KCFGC_EXECUTABLE)
      MESSAGE(STATUS "Found KDE4 kconfig_compiler preprocessor: ${KDE4_KCFGC_EXECUTABLE}")
   ELSE(KDE4_KCFGC_EXECUTABLE)
      MESSAGE(STATUS "Didn't find the KDE4 kconfig_compiler preprocessor")
   ENDIF(KDE4_KCFGC_EXECUTABLE)
ENDMACRO (KDE4_PRINT_RESULTS)


IF (KDE4_FIND_REQUIRED AND NOT KDE4_FOUND)
   #bail out if something wasn't found
   KDE4_PRINT_RESULTS()
   MESSAGE(FATAL_ERROR "Could not find everything required for compiling KDE 4 programs")
ENDIF (KDE4_FIND_REQUIRED AND NOT KDE4_FOUND)


IF (NOT KDE4_FIND_QUIETLY)
   KDE4_PRINT_RESULTS()
ENDIF (NOT KDE4_FIND_QUIETLY)

#add the found Qt and KDE include directories to the current include path
SET(KDE4_INCLUDE_DIRS ${QT_INCLUDES} ${KDE4_INCLUDE_DIR} ${_KDE4_PLATFORM_INCLUDE_DIRS} )

# not used in Qt4: QT_NO_COMPAT, QT_CLEAN_NAMESPACE, QT_THREAD_SUPPORT
SET(KDE4_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} -DQT3_SUPPORT -DQT_NO_STL -DQT_NO_CAST_TO_ASCII -D_REENTRANT )
