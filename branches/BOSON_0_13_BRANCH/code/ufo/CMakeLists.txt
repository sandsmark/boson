add_subdirectory( src )

Include(${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)
CHECK_INCLUDE_FILE_CXX( sstream HAVE_SSTREAM )
CHECK_INCLUDE_FILE_CXX( strstream HAVE_STRSTREAM )
CHECK_INCLUDE_FILE( stdint.h HAVE_STDINT_H )
CHECK_INCLUDE_FILE( execinfo.h HAVE_EXECINFO_H )
CHECK_INCLUDE_FILE( pwd.h HAVE_PWD_H )
CHECK_INCLUDE_FILE( dlfcn.h HAVE_DLFCN_H )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-ufo.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ufo.h )

