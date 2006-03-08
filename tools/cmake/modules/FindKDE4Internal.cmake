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
# KDE4_MEINPROC_EXECUTABLE
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
# KDE4_INSTALL_LIBTOOL_FILE
# KDE4_CREATE_FINAL_FILE
# KDE4_ADD_KPART
# KDE4_ADD_KLM
# KDE4_ADD_EXECUTABLE
#
# _KDE4_PLATFORM_INCLUDE_DIRS is used only internally
# _KDE4_PLATFORM_DEFINITIONS is used only internally


CMAKE_MINIMUM_REQUIRED(VERSION 2.3.3)

#this line includes FindQt.cmake, which searches the Qt library and headers
FIND_PACKAGE(Qt4 REQUIRED)

set(QT_AND_KDECORE_LIBS ${QT_QTCORE_LIBRARY} kdecore)

INCLUDE (MacroLibrary)

#add some KDE specific stuff


# the following are directories where stuff will be installed to
set(CONFIG_INSTALL_DIR       /share/config              CACHE STRING "The config file install dir")
set(DATA_INSTALL_DIR         /share/apps                CACHE STRING "The parent directory where applications can install their data")
set(HTML_INSTALL_DIR         /share/doc/HTML            CACHE STRING "The HTML install dir for documentation")
set(ICON_INSTALL_DIR         /share/icons               CACHE STRING "The icon install dir (default prefix/share/icons/)")
set(INFO_INSTALL_DIR         /info                      CACHE STRING "The kde info install dir (default prefix/info)")
set(KCFG_INSTALL_DIR         /share/config.kcfg         CACHE STRING "The install dir for kconfig files")
set(LIB_INSTALL_DIR          /lib                       CACHE STRING "The subdirectory relative to the install prefix where libraries will be installed (default is /lib)")
set(LOCALE_INSTALL_DIR       /share/locale              CACHE STRING "The install dir for translations")
set(MAN_INSTALL_DIR          /man                       CACHE STRING "The kde man install dir (default prefix/man/)")
set(MIME_INSTALL_DIR         /share/mimelnk             CACHE STRING "The install dir for the mimetype desktop files")
set(PLUGIN_INSTALL_DIR       "${LIB_INSTALL_DIR}/kde4"  CACHE STRING "The subdirectory relative to the install prefix where plugins will be installed (default is ${KDE4_LIB_INSTALL_DIR}/kde4)")
set(SERVICES_INSTALL_DIR     /share/services            CACHE STRING "The install dir for service (desktop, protocol, ...) files")
set(SERVICETYPES_INSTALL_DIR /share/servicetypes        CACHE STRING "The install dir for servicestypes desktop files")
set(SOUND_INSTALL_DIR        /share/sounds              CACHE STRING "The install dir for sound files")
set(TEMPLATES_INSTALL_DIR    /share/templates           CACHE STRING "The install dir for templates (Create new file...)")
set(WALLPAPER_INSTALL_DIR    /share/wallpapers          CACHE STRING "The install dir for wallpapers")
set(KCONF_UPDATE_INSTALL_DIR /share/apps/kconf_update/  CACHE STRING "The kconf_update install dir")
set(XDG_APPS_DIR             /share/applications/kde    CACHE STRING "The XDG apps dir")
set(XDG_DIRECTORY_DIR        /share/desktop-directories CACHE STRING "The XDG directory")

# seem to be unused:
set(LIBS_HTML_INSTALL_DIR    /share/doc/HTML            CACHE STRING "Is this still used ?")
set(APPLNK_INSTALL_DIR       /share/applnk              CACHE STRING "Is this still used ?")
#set(KDE4_SYSCONF_INSTALL_DIR "/etc" CACHE STRING "The kde sysconfig install dir (default /etc)")
# set(KDE4_DIR               ${CMAKE_INSTALL_PREFIX})


option(KDE4_ENABLE_FINAL "Enable final all-in-one compilation")
option(KDE4_BUILD_TESTS  "Build the tests")
option(KDE4_USE_QT_EMB   "link to Qt-embedded, don't use X")


# RPATH handling
set(KDE4_NEED_WRAPPER_SCRIPTS FALSE)
if (UNIX)

   set(KDE4_NEED_WRAPPER_SCRIPTS TRUE)

   # to disable RPATH completely, set CMAKE_SKIP_RPATH to TRUE

   option(KDE4_RPATH_TO_BUILD_DIR "Compile executables with RPATH set to both the builddir and the installdir, otherwise RPATH will be set to the install dir. You can disable RPATH completely by setting CMAKE_SKIP_RPATH to ON" ON)

   # this avoids relinking during installation
   set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
   set(CMAKE_SKIP_BUILD_RPATH TRUE)
   # this is the list of RPATHs, start empty

   # optionally add the builddir
   if (KDE4_RPATH_TO_BUILD_DIR)
      set(KDE4_NEED_WRAPPER_SCRIPTS FALSE)
   endif (KDE4_RPATH_TO_BUILD_DIR)

   if (CMAKE_SKIP_RPATH)
      set(KDE4_NEED_WRAPPER_SCRIPTS TRUE)
   endif (CMAKE_SKIP_RPATH)

endif (UNIX)

# set it to false again until the next kde release of cmake is required
set(KDE4_NEED_WRAPPER_SCRIPTS FALSE)


#now try to find some kde stuff

#are we trying to compile kdelibs ?
#then enter bootstrap mode
if(EXISTS ${CMAKE_SOURCE_DIR}/kdecore/kglobal.h)

   message(STATUS "Building kdelibs...")

   set(KDE4_INCLUDE_DIR ${CMAKE_SOURCE_DIR})

   set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin )
  
   # adjust dcopidl and the library output path depending on the platform
   if (WIN32)
      # under windows dcopidl.bat has to be used, except when using MSYS, then the perl script has to be used, Alex
      if ("${CMAKE_GENERATOR}" MATCHES "MSYS")
         set(KDE4_DCOPIDL_EXECUTABLE ${CMAKE_SOURCE_DIR}/dcop/dcopidlng/dcopidl )
      else ("${CMAKE_GENERATOR}" MATCHES "MSYS")
         set(KDE4_DCOPIDL_EXECUTABLE call ${CMAKE_SOURCE_DIR}/dcop/dcopidlng/dcopidl.bat )
      endif ("${CMAKE_GENERATOR}" MATCHES "MSYS")
  
      set(LIBRARY_OUTPUT_PATH  ${EXECUTABLE_OUTPUT_PATH} )
   else (WIN32)
      set(KDE4_DCOPIDL_EXECUTABLE ${CMAKE_SOURCE_DIR}/dcop/dcopidlng/dcopidl )
      set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib )
   endif (WIN32)

   set(KDE4_LIB_DIR ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})
   set(KDE4_KALYPTUS_DIR ${CMAKE_SOURCE_DIR}/dcop/dcopidlng/ )
  
   # CMAKE_CFG_INTDIR is the output subdirectory created e.g. by XCode and MSVC
   if (KDE4_NEED_WRAPPER_SCRIPTS)
      set(KDE4_DCOPIDL2CPP_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/dcopidl2cpp.sh )
      set(KDE4_KCFGC_EXECUTABLE       ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/kconfig_compiler.sh )
      set(KDE4_MEINPROC_EXECUTABLE    ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/meinproc.sh )
   else (KDE4_NEED_WRAPPER_SCRIPTS)
      set(KDE4_DCOPIDL2CPP_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/dcopidl2cpp )
      set(KDE4_KCFGC_EXECUTABLE       ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/kconfig_compiler )
      set(KDE4_MEINPROC_EXECUTABLE    ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/meinproc )
   endif (KDE4_NEED_WRAPPER_SCRIPTS)

   # when building kdelibs, make the dcop and kcfg rules depend on the binaries...
   set( _KDE4_DCOPIDL2CPP_DEP dcopidl2cpp)
   set( _KDE4_KCONFIG_COMPILER_DEP kconfig_compiler)
  
else(EXISTS ${CMAKE_SOURCE_DIR}/kdecore/kglobal.h)

   get_filename_component( kde_cmake_module_dir  ${CMAKE_CURRENT_LIST_FILE} PATH)
   # this file contains all dependencies of all libraries of kdelibs, Alex
   include(${kde_cmake_module_dir}/KDELibsDependencies.cmake)

# the following variables should be named KDE4_KIO_LIBRARIES etc
# Alex

   set(LIB_KDECORE ${QT_AND_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${X11_X11_LIB} DCOP ${ZLIB_LIBRARY})

   # kdeui_LIB_DEPENDS comes from KDELibsDependencies.cmake, Alex
   set(LIB_KDEUI ${kdeui_LIB_DEPENDS} kdeui)

message(STATUS "kdeui: ${LIB_KDEUI}")

   set(LIB_KIO ${LIB_KDEUI} kio)

   set(LIB_KPARTS ${LIB_KIO} kparts)

   set(LIB_KUTILS ${LIB_KPARTS} kutils)

   set(LIB_KDE3SUPPORT ${QT_QT3SUPPORT_LIBRARY} ${LIB_KUTILS} kde3support)



  # ... but NOT otherwise
  set( _KDE4_DCOPIDL2CPP_DEP )
  set( _KDE4_KCONFIG_COMPILER_DEP)
	set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib )

  # at first the KDE include direcory
  # kpassworddialog.h is new with KDE4
  FIND_PATH(KDE4_INCLUDE_DIR kpassworddialog.h
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

  FIND_PROGRAM(KDE4_MEINPROC_EXECUTABLE NAME meinproc PATHS
    $ENV{KDEDIR}/bin
    /opt/kde/bin
    /opt/kde4/bin
  )

endif(EXISTS ${CMAKE_SOURCE_DIR}/kdecore/kglobal.h)


#####################  and now the platform specific stuff  ############################

# Set a default build type for single-configuration
# CMake generators if no build type is set.
IF (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)


if (WIN32)

   if(CYGWIN)
      message(FATAL_ERROR "Support for Cygwin NOT yet implemented, please edit FindKDE4.cmake to enable it")
   endif(CYGWIN)

   FIND_PACKAGE(KDEWIN32 REQUIRED)
   
   set( _KDE4_PLATFORM_INCLUDE_DIRS ${KDEWIN32_INCLUDES})
   set( QT_AND_KDECORE_LIBS ${QT_AND_KDECORE_LIBS} ${KDEWIN32_LIBRARIES} )
     
   # windows, mingw
   if(MINGW)
   #hmmm, something special to do here ?
   endif(MINGW)
   
   # windows, microsoft compiler
   if(MSVC)
      set( _KDE4_PLATFORM_DEFINITIONS -DKDE_FULL_TEMPLATE_EXPORT_INSTANTIATION -DWIN32_LEAN_AND_MEAN -DUNICODE )
      IF(CMAKE_COMPILER_2005)
         ADD_DEFINITIONS( -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE )
         SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4661" )
      ENDIF(CMAKE_COMPILER_2005)
   endif(MSVC)

endif (WIN32)


# also use /usr/local by default under UNIX, including Mac OS X
if (UNIX)
   link_directories(/usr/local/lib)
   include_directories(/usr/local/include)

   # build the install RPATH
   set(CMAKE_INSTALL_RPATH)

   # optionally add the builddir
   if (KDE4_RPATH_TO_BUILD_DIR)
      set(CMAKE_INSTALL_RPATH ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/ )
   endif (KDE4_RPATH_TO_BUILD_DIR)

   # add the library install dir
   set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${CMAKE_INSTALL_PREFIX}${LIB_INSTALL_DIR})
   # add the Qt library dir
   set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${QT_LIBRARY_DIR})

   # building something else than kdelibs/ ?
   # then add the dir where the kde libraries are installed
   if (NOT EXISTS ${CMAKE_SOURCE_DIR}/kdecore/kglobal.h)
      set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${KDE4_LIB_DIR} )
   endif (NOT EXISTS ${CMAKE_SOURCE_DIR}/kdecore/kglobal.h)

endif (UNIX)


# UNIX, except OS X
if (UNIX AND NOT APPLE)
   find_package(X11 REQUIRED)
   set(_KDE4_PLATFORM_INCLUDE_DIRS ${X11_INCLUDE_DIR} )
endif (UNIX AND NOT APPLE)


# This will need to be modified later to support either Qt/X11 or Qt/Mac builds
if (APPLE)

  set ( _KDE4_PLATFORM_DEFINITIONS -D__APPLE_KDE__ )

  # we need to set MACOSX_DEPLOYMENT_TARGET to (I believe) at least 10.2 or maybe 10.3 to allow
  # -undefined dynamic_lookup; in the future we should do this programmatically
  # hmm... why doesn't this work?
  set (ENV{MACOSX_DEPLOYMENT_TARGET} 10.3)

  # "-undefined dynamic_lookup" means we don't care about missing symbols at link-time by default
  # this is bad, but unavoidable until there is the equivalent of libtool -no-undefined implemented
  # or perhaps it already is, and I just don't know where to look  ;)

  set (CMAKE_SHARED_LINKER_FLAGS "-single_module -multiply_defined suppress")
  set (CMAKE_MODULE_LINKER_FLAGS "-multiply_defined suppress")
  #set(CMAKE_SHARED_LINKER_FLAGS "-single_module -undefined dynamic_lookup -multiply_defined suppress")
  #set(CMAKE_MODULE_LINKER_FLAGS "-undefined dynamic_lookup -multiply_defined suppress")

  # removed -Os, was there a special reason for using -Os instead of -O2 ?, Alex
  # optimization flags are set below for the various build types
  set (CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -fno-common")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common")
endif (APPLE)


# only on linux, but NOT e.g. on FreeBSD:
if (CMAKE_SYSTEM_NAME MATCHES Linux)
   set ( _KDE4_PLATFORM_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_GNU_SOURCE)
   set ( CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -avoid-version -Wl,--no-undefined -lc")
   set ( CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings -avoid-version -Wl,--no-undefined -lc")
   # optimization flags are set further below for the various build types
   set ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-long-long -ansi -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-common")
   set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -fno-exceptions -fno-check-new -fno-common")
endif (CMAKE_SYSTEM_NAME MATCHES Linux)


# works on FreeBSD, NOT tested on NetBSD and OpenBSD
if (CMAKE_SYSTEM_NAME MATCHES BSD)
   set ( _KDE4_PLATFORM_DEFINITIONS -D_GNU_SOURCE )
   set ( CMAKE_SHARED_LINKER_FLAGS "-avoid-version -lc")
   set ( CMAKE_MODULE_LINKER_FLAGS "-avoid-version -lc")
   # optimization flags are set further below for the various build types
   set ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-long-long -ansi -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-common")
   set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-exceptions -fno-check-new -fno-common")
endif (CMAKE_SYSTEM_NAME MATCHES BSD)


# compiler specific stuff, maybe this should be done differently, Alex

if (MSVC)
   set (KDE4_ENABLE_EXCEPTIONS -EHsc)
endif(MSVC)

if (CMAKE_COMPILER_IS_GNUCXX)
   set (KDE4_ENABLE_EXCEPTIONS -fexceptions)
   # Select flags.
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
   set(CMAKE_CXX_FLAGS_RELEASE        "-O2")
   set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
   set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g")
   set(CMAKE_C_FLAGS_RELEASE          "-O2")
   set(CMAKE_C_FLAGS_DEBUG            "-O0 -g")
endif (CMAKE_COMPILER_IS_GNUCXX)

# it seems we prefer not to use a different postfix for debug libs, Alex
# SET(CMAKE_DEBUG_POSTFIX "_debug")

###########    end of platform specific stuff  ##########################


# KDE4Macros.cmake contains all the KDE specific macros
INCLUDE(KDE4Macros)


# decide whether KDE4 has been found
if (KDE4_INCLUDE_DIR AND KDE4_LIB_DIR AND KDE4_DCOPIDL_EXECUTABLE AND KDE4_DCOPIDL2CPP_EXECUTABLE AND KDE4_KCFGC_EXECUTABLE)
   set(KDE4_FOUND TRUE)
else (KDE4_INCLUDE_DIR AND KDE4_LIB_DIR AND KDE4_DCOPIDL_EXECUTABLE AND KDE4_DCOPIDL2CPP_EXECUTABLE AND KDE4_KCFGC_EXECUTABLE)
   set(KDE4_FOUND FALSE)
endif (KDE4_INCLUDE_DIR AND KDE4_LIB_DIR AND KDE4_DCOPIDL_EXECUTABLE AND KDE4_DCOPIDL2CPP_EXECUTABLE AND KDE4_KCFGC_EXECUTABLE)


MACRO (KDE4_PRINT_RESULTS)
   if(KDE4_INCLUDE_DIR)
      message(STATUS "Found KDE4 include dir: ${KDE4_INCLUDE_DIR}")
   else(KDE4_INCLUDE_DIR)
      message(STATUS "Didn't find KDE4 headers")
   endif(KDE4_INCLUDE_DIR)

   if(KDE4_LIB_DIR)
      message(STATUS "Found KDE4 library dir: ${KDE4_LIB_DIR}")
   else(KDE4_LIB_DIR)
      message(STATUS "Didn't find KDE4 core library")
   endif(KDE4_LIB_DIR)

   if(KDE4_DCOPIDL_EXECUTABLE)
      message(STATUS "Found KDE4 dcopidl preprocessor: ${KDE4_DCOPIDL_EXECUTABLE}")
   else(KDE4_DCOPIDL_EXECUTABLE)
      message(STATUS "Didn't find the KDE4 dcopidl preprocessor")
   endif(KDE4_DCOPIDL_EXECUTABLE)

   if(KDE4_DCOPIDL2CPP_EXECUTABLE)
      message(STATUS "Found KDE4 dcopidl2cpp preprocessor: ${KDE4_DCOPIDL2CPP_EXECUTABLE}")
   else(KDE4_DCOPIDL2CPP_EXECUTABLE)
      message(STATUS "Didn't find the KDE4 dcopidl2cpp preprocessor")
   endif(KDE4_DCOPIDL2CPP_EXECUTABLE)

   if(KDE4_KCFGC_EXECUTABLE)
      message(STATUS "Found KDE4 kconfig_compiler preprocessor: ${KDE4_KCFGC_EXECUTABLE}")
   else(KDE4_KCFGC_EXECUTABLE)
      message(STATUS "Didn't find the KDE4 kconfig_compiler preprocessor")
   endif(KDE4_KCFGC_EXECUTABLE)
ENDMACRO (KDE4_PRINT_RESULTS)


if (KDE4Internal_FIND_REQUIRED AND NOT KDE4_FOUND)
   #bail out if something wasn't found
   KDE4_PRINT_RESULTS()
   message(FATAL_ERROR "Could NOT find everything required for compiling KDE 4 programs")
endif (KDE4Internal_FIND_REQUIRED AND NOT KDE4_FOUND)


if (NOT KDE4Internal_FIND_QUIETLY)
   KDE4_PRINT_RESULTS()
endif (NOT KDE4Internal_FIND_QUIETLY)

#add the found Qt and KDE include directories to the current include path
set(KDE4_INCLUDE_DIRS ${QT_INCLUDES} ${KDE4_INCLUDE_DIR} ${_KDE4_PLATFORM_INCLUDE_DIRS} )

# NOT used in Qt4: QT_NO_COMPAT, QT_CLEAN_NAMESPACE, QT_THREAD_SUPPORT
set(KDE4_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} -DQT3_SUPPORT -DQT_NO_STL -DQT_NO_CAST_TO_ASCII -D_REENTRANT -DQT3_SUPPORT_WARNINGS )
