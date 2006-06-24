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


MACRO(_REMOVE_LIBS_FROM_LIST list have_removed remove_libs)
   SET(_newlist "")
   SET(${have_removed} "")
   FOREACH (current_lib ${${list}})
      SET(_match FALSE)
      FOREACH (remove_lib ${remove_libs})
         IF (${current_lib} MATCHES "^ *(-l)?${remove_lib}")
            SET(_match TRUE)
         ENDIF (${current_lib} MATCHES "^ *(-l)?${remove_lib}")
      ENDFOREACH (remove_lib)
      IF (_match)
         SET(${have_removed} "${${have_removed}};${current_lib}")
      ELSE (_match)
         SET(_newlist "${_newlist};${current_lib}")
      ENDIF (_match)
   ENDFOREACH (current_lib)
   SET(${list} ${_newlist})
ENDMACRO(_REMOVE_LIBS_FROM_LIST)

MACRO(BOSON_USE_STATIC_QT_AND_KDE)
   GET_FILENAME_COMPONENT(_qt_lib_dir ${QT_QT_LIBRARY} PATH)
   SET(_qt_mt_la "${_qt_lib_dir}/libqt-mt.la")
   SET(_kdecore_la "${KDE3_LIB_DIR}/libkdecore.la")
   SET(_kdeui_la "${KDE3_LIB_DIR}/libkdeui.la")
   SET(_kio_la "${KDE3_LIB_DIR}/libkio.la")

   BOSON_READ_STATIC_DEPENDENCIES_FROM_LA(${_qt_mt_la} _static_qt_dependencies)
   BOSON_READ_STATIC_DEPENDENCIES_FROM_LA(${_kdecore_la} _static_kdecore_dependencies)
   BOSON_READ_STATIC_DEPENDENCIES_FROM_LA(${_kdeui_la} _static_kdeui_dependencies)
   BOSON_READ_STATIC_DEPENDENCIES_FROM_LA(${_kio_la} _static_kio_dependencies)

   STRING(REGEX REPLACE " " ";" QT_LIBRARIES "${QT_LIBRARIES}")
   STRING(REGEX REPLACE " " ";" QT_AND_KDECORE_LIBS "${QT_AND_KDECORE_LIBS}")
   STRING(REGEX REPLACE " " ";" QT_AND_KDECORE_KDEUI_LIBS "${QT_AND_KDECORE_KDEUI_LIBS}")
   STRING(REGEX REPLACE " " ";" QT_AND_KDECORE_KDEUI_KIO_LIBS "${QT_AND_KDECORE_KDEUI_KIO_LIBS}")
   SET(QT_LIBRARIES "${QT_LIBRARIES};${_static_qt_dependencies}")
   SET(QT_AND_KDECORE_LIBS "${QT_AND_KDECORE_LIBS};${_static_kdecore_dependencies};${_static_qt_dependencies}")
   SET(QT_AND_KDECORE_KDEUI_LIBS "kdeui;${_static_kdeui_dependencies};${QT_AND_KDECORE_LIBS}")
   SET(QT_AND_KDECORE_KDEUI_KIO_LIBS "kio;${_static_kio_dependencies};${QT_AND_KDECORE_KDEUI_LIBS}")


   # we don't want these libs linked statically atm.
   SET(link_libs_dynamic "dl;pthread")

   # NEVER EVER link libGL statically
   SET(link_libs_dynamic "${link_libs_dynamic};GL")

   _REMOVE_LIBS_FROM_LIST(QT_AND_KDECORE_LIBS libs_removed "${link_libs_dynamic}")
   set(QT_AND_KDECORE_LIBS "${QT_AND_KDECORE_LIBS};-Wl,-Bdynamic;${libs_removed}")
   _REMOVE_LIBS_FROM_LIST(QT_LIBRARIES libs_removed "${link_libs_dynamic}")
   set(QT_LIBRARIES "${QT_LIBRARIES};-Wl,-Bdynamic;${libs_removed}")
   _REMOVE_LIBS_FROM_LIST(QT_AND_KDECORE_KDEUI_LIBS libs_removed "${link_libs_dynamic}")
   set(QT_AND_KDECORE_KDEUI_LIBS "${QT_AND_KDECORE_KDEUI_LIBS};-Wl,-Bdynamic;${libs_removed}")
   _REMOVE_LIBS_FROM_LIST(QT_AND_KDECORE_KDEUI_KIO_LIBS libs_removed "${link_libs_dynamic}")
   set(QT_AND_KDECORE_KDEUI_KIO_LIBS "${QT_AND_KDECORE_KDEUI_KIO_LIBS};-Wl,-Bdynamic;${libs_removed}")

ENDMACRO(BOSON_USE_STATIC_QT_AND_KDE)


# lib_variable: the _name_ of a variable that holds the libname.
#               the libname in that variable will be set to the transformed
#               libname, i.e.:
#                 * start with -l if possible
#                 * don't be an absolute path
#                 * have a -L with it, if possible
#               -> the variable may therefore actually be a ; separated list
#                  afterwards.
MACRO(_BOSON_TARGET_LINK_LIBRARIES_TRANSFORM_LIBNAME lib_variable)
   # AB: I don't know if whitespaces are possible here, but we definitely
   #     don't want them
   STRING(REGEX REPLACE "^ +" "" ${lib_variable} "${${lib_variable}}")
   STRING(REGEX REPLACE " +$" "" ${lib_variable} "${${lib_variable}}")

   # AB: we should be able to use -lkdecore instead of kdecore (same for the
   #     others), because we use LINK_DIRECTORIES for Qt/KDE libs anyway, so
   #     we don't need the -L line that cmake adds
   SET(_qt_kde_libs "kdecore;kdeui;kio;qt-mt;qassistantclient")
   FOREACH (current_lib ${_qt_kde_libs})
     IF ("${${lib_variable}}" MATCHES "^${current_lib}$")
        SET(${lib_variable} "-l${current_lib}")
     ENDIF ("${${lib_variable}}" MATCHES "^${current_lib}$")
   ENDFOREACH (current_lib)

   IF (${${lib_variable}} MATCHES "^/")
      GET_FILENAME_COMPONENT(_path "${${lib_variable}}" PATH)
      IF (${${lib_variable}} MATCHES "python")
         message(STATUS "${${lib_variable}}")
      ENDIF (${${lib_variable}} MATCHES "python")

      # AB: NAME_WE does not work properly: it turns "python2.4.so" into
      #     "python2" - we want only the ".so" part removed.
#      GET_FILENAME_COMPONENT(_file "${${lib_variable}}" NAME_WE)
      GET_FILENAME_COMPONENT(_file "${${lib_variable}}" NAME)
      STRING(REGEX MATCH "^.*\\." _file "${_file}")
      STRING(REGEX REPLACE "\\.$" "" _file "${_file}")
      IF (${_file} MATCHES "^lib")
         STRING(REGEX REPLACE "^lib" "" _file "${_file}")
         SET(${lib_variable} "-L${_path};-l${_file}")
      ENDIF (${_file} MATCHES "^lib")
   ENDIF (${${lib_variable}} MATCHES "^/")
ENDMACRO(_BOSON_TARGET_LINK_LIBRARIES_TRANSFORM_LIBNAME)


# behaves like TARGET_LINK_LIBRARIES, but in addition provides semi-static builds
# if BOSON_LINK_STATIC is set
MACRO(BOSON_TARGET_LINK_LIBRARIES target)

   # we don't want these libs linked statically atm.
   SET(link_libs_dynamic "dl;pthread")

   # NEVER EVER link libGL statically
   SET(link_libs_dynamic "${link_libs_dynamic};GL")


   SET(args "${ARGN}")
   IF (BOSON_LINK_STATIC)
      # first transform the libs, i.e. if possible turn things like "kdecore"
      # to "-lkdecore" and "/path/to/libfoo.a" to "-L/path/to;-lfoo.a"
      SET(tmp_args "${args}")
      SET(args "")
      FOREACH (lib ${tmp_args})
         _BOSON_TARGET_LINK_LIBRARIES_TRANSFORM_LIBNAME(lib)
         SET(args "${args};${lib}")
      ENDFOREACH (lib)

      # replace "-lfoo" by "-Wl,-Bstatic -lfoo -Wl,-Bdynamic", if static
      # linking is desired
      SET(tmp_args "${args}")
      SET(args "")
      FOREACH (lib ${tmp_args})
         SET(want_static FALSE)
         IF (${lib} MATCHES "^ *-l")
            SET(want_static TRUE)
            FOREACH (l ${link_libs_dynamic})
               IF (${lib} MATCHES "^ *(-l)?${l}")
                  SET(want_static FALSE)
               ENDIF (${lib} MATCHES "^ *(-l)?${l}")
            ENDFOREACH (l)
         ENDIF (${lib} MATCHES "^ *-l")

         IF (want_static)
            # note: the whole thing after the ';' is considered by cmake as a
            #_single_ lib, as it is only space separated (cmake lists are ;
            # separated).
            # this is the whole point of this macro, as this way the
            # -Wl,-Bdynamic won't get removed - unless the whole lib will get
            # removed (cmake does that if it notices that it is duplicated and
            # therefore not required.. however it is wrong about the
            # -Wl,-Bdynamic one)
            SET(args "${args};-Wl,-Bstatic ${lib} -Wl,-Bdynamic")
         ELSE (want_static)
            SET(args "${args};${lib}")
         ENDIF (want_static)
      ENDFOREACH (lib)
   ENDIF (BOSON_LINK_STATIC)
   TARGET_LINK_LIBRARIES(${target} ${args})
ENDMACRO(BOSON_TARGET_LINK_LIBRARIES)

# vim: et sw=3 textwidth=0
