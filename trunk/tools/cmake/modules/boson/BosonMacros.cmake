# BOSON_ADD_BOUI_FILES
# defines X11_XMU_LIB
# defines X11_XRANDR_LIB


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

# vim: et sw=3 textwidth=0
