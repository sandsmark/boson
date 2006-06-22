# This file contains the following macros:
# BOSON_ADD_BOUI_FILES
# BOSON_READ_STATIC_DEPENDENCIES_FROM_LA
# BOSON_USE_STATIC_QT_AND_KDE
# BOSON_TARGET_LINK_LIBRARIES
#
# This file defines the following variables:
# QT_MT_REQUIRED (to TRUE)
# BOUIC_EXECUTABLE (to our bouic binary)
# X11_XMU_LIB
# X11_XRANDR_LIB


# AB: I used cmake 2.3 to write this, so let's require this. probably would work
# with lower versions too
CMAKE_MINIMUM_REQUIRED(VERSION 2.3)

SET(QT_MT_REQUIRED TRUE)
#SET(QT_MIN_VERSION "3.0.0")

FIND_LIBRARY(X11_XMU_LIB Xmu ${X11_LIB_SEARCH_PATH})
FIND_LIBRARY(X11_XRANDR_LIB Xrandr ${X11_LIB_SEARCH_PATH})



SET(BOUIC_EXECUTABLE ${CMAKE_BINARY_DIR}/boson/boufo/bouic/bouic)

# similar to KDE3_ADD_UI_FILES()
# usage: BOSON_ADD_BOUI_FILES(foo_SRCS ${boui_files})
MACRO ( BOSON_ADD_BOUI_FILES _sources )
   FOREACH (_current_FILE ${ARGN})
     GET_FILENAME_COMPONENT(_tmp_FILE ${_current_FILE} ABSOLUTE)

     GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)

     SET(_src ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
     SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
     SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)

     # AB: note: bouic creates both, .cpp AND .h files!
     ADD_CUSTOM_COMMAND(OUTPUT ${_header}
        COMMAND ${BOUIC_EXECUTABLE}
        ARGS --input ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE} --output ${CMAKE_CURRENT_BINARY_DIR}/${_basename} --addinclude klocale.h
        DEPENDS ${_tmp_FILE} ${BOUIC_EXECUTABLE}
     )

     # dummy command
     ADD_CUSTOM_COMMAND(OUTPUT ${_src}
        COMMAND
        DEPENDS ${_tmp_FILE} ${_header}
     )

     ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
        COMMAND ${QT_MOC_EXECUTABLE}
        ARGS ${_header} -o ${_moc}
        DEPENDS ${_header}
     )
     SET_SOURCE_FILES_PROPERTIES(${_src} PROPERTIES SKIP_AUTOMOC true)


     SET(${_sources} ${${_sources}} ${_src})

   ENDFOREACH (_current_FILE)
ENDMACRO (BOSON_ADD_BOUI_FILES)


# reads the .la _la_file of a library and places the libraries that it depends on in the dependencies variable
MACRO(BOSON_READ_STATIC_DEPENDENCIES_FROM_LA _la_file dependencies)
   set(_dependencies "")
   if (EXISTS "${_la_file}")
      file(READ ${_la_file} _la_data)
      string(REGEX MATCH "dependency_libs='.*" _dependencies "${_la_data}")
      string(REGEX REPLACE "' *\n.*" "" _dependencies "${_dependencies}")
      string(REGEX REPLACE "dependency_libs='" "" _dependencies "${_dependencies}")

      # turn into cmake list
      string(REGEX REPLACE " " ";" _dependencies ${_dependencies})

      set(_dependencies_tmp "${_dependencies}")
      set(_dependencies "")
      foreach (element ${_dependencies_tmp})
         # we don't use -R at all in boson.
         # certainly not with static libs.
         if (${element} MATCHES "^ *-R")
            set(element "")
         endif (${element} MATCHES "^ *-R")

         # "libfoo.la" -> "libfoo"
         string(REGEX REPLACE "\\.la" "" element "${element}")

         # "/path/to/libs/libfoo" -> "-lfoo"
         string(REGEX REPLACE "^ */.*/lib" "-l" element "${element}")

         if (element)
            set(_dependencies "${_dependencies};${element}")
         endif (element)
      endforeach(element)
   else (EXISTS "${_la_file}")
      message(STATUS "${_la_file} does not exist. Need a .la file to find dependencies of static library. Linking will probably fail")
   endif (EXISTS "${_la_file}")
   set(${dependencies} "${_dependencies}")
ENDMACRO(BOSON_READ_STATIC_DEPENDENCIES_FROM_LA)

MACRO(BOSON_USE_STATIC_QT_AND_KDE)
   MESSAGE(STATUS "TODO: use proper path to *.la files")
   # TODO: use KDEDIR/QTDIR or so (i.e. use path to real libfoo.a and replace
   #       ".a" by ".la")
   SET(_qt_mt_la "/home/andi/kde/boson/static/qt-copy/lib/libqt-mt.la")
   SET(_kdecore_la "/home/andi/kde/boson/static/kdedir/lib/libkdecore.la")

   BOSON_READ_STATIC_DEPENDENCIES_FROM_LA(${_qt_mt_la} _static_qt_dependencies)
   BOSON_READ_STATIC_DEPENDENCIES_FROM_LA(${_kdecore_la} _static_kdecore_dependencies)

   STRING(REGEX REPLACE " " ";" QT_LIBRARIES "${QT_LIBRARIES}")
   STRING(REGEX REPLACE " " ";" QT_AND_KDECORE_LIBS "${QT_AND_KDECORE_LIBS}")
   SET(QT_LIBRARIES "${QT_LIBRARIES};${_static_qt_dependencies}")
   SET(QT_AND_KDECORE_LIBS "${QT_AND_KDECORE_LIBS};${_static_kdecore_dependencies};${_static_qt_dependencies}")


   # we don't want these libs linked statically atm.
   SET(link_libs_dynamic "dl;pthread")

   # NEVER EVER link libGL statically
   SET(link_libs_dynamic "${link_libs_dynamic};GL")
   FOREACH (lib ${link_libs_dynamic})
      IF ("${QT_AND_KDECORE_LIBS}" MATCHES "-l${lib}")
         STRING(REGEX REPLACE "-l${lib}" "" QT_AND_KDECORE_LIBS "${QT_AND_KDECORE_LIBS}")
         SET(QT_AND_KDECORE_LIBS "${QT_AND_KDECORE_LIBS};-Wl,-Bdynamic;-l${lib};-Wl,-Bstatic")
      ENDIF ("${QT_AND_KDECORE_LIBS}" MATCHES "-l${lib}")

      IF ("${QT_LIBRARIES}" MATCHES "-l${lib}")
         STRING(REGEX REPLACE "-l${lib}" "" QT_LIBRARIES "${QT_LIBRARIES}")
         SET(QT_LIBRARIES "${QT_LIBRARIES};-Wl,-Bdynamic;-l${lib};-Wl,-Bstatic")
      ENDIF ("${QT_LIBRARIES}" MATCHES "-l${lib}")
   ENDFOREACH (lib)
ENDMACRO(BOSON_USE_STATIC_QT_AND_KDE)


# behaves like TARGET_LINK_LIBRARIES, but in addition provides semi-static builds
# if BOSON_LINK_STATIC is set
MACRO(BOSON_TARGET_LINK_LIBRARIES target)
   set(args "${ARGN}")
   IF (BOSON_LINK_STATIC)
      SET(args "-Wl,-Bstatic;${args};-Wl,-Bdynamic")
   ENDIF (BOSON_LINK_STATIC)
   TARGET_LINK_LIBRARIES(${target} ${args})
ENDMACRO(BOSON_TARGET_LINK_LIBRARIES)


# vim: et sw=3 textwidth=0
