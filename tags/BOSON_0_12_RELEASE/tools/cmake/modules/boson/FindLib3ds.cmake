# find lib3ds includes and library
#
# LIB3DS_INCLUDE_DIR - where the lib3ds directory containing the headers can be
#                      found
# LIB3DS_LIBRARY       full path to the lib3ds library
# LIB3DS_LIBRARY_DIR   directory containing the lib3ds library

FIND_PATH(LIB3DS_INCLUDE_DIR lib3ds/file.h
	/usr/include
	/usr/local/include
)
# AB: apparently FIND_LIBRARY() won't find static libraries
set(LIB3DS_LIBRARY lib3ds.a)
FIND_PATH(
	LIB3DS_LIBRARY_DIR ${LIB3DS_LIBRARY}
	/usr/lib
	/usr/local/lib
	$ENV{CMAKE_LIBRARY_PATH}
)
IF(LIB3DS_LIBRARY_DIR)
	SET(LIB3DS_LIBRARY ${LIB3DS_LIBRARY_DIR}/${LIB3DS_LIBRARY})
ELSE(LIB3DS_LIBRARY_DIR)
	SET(LIB3DS_LIBRARY "")
ENDIF(LIB3DS_LIBRARY_DIR)

IF(LIB3DS_INCLUDE_DIR)
	MESSAGE(STATUS "Found lib3ds include dir: ${LIB3DS_INCLUDE_DIR}")
ELSE(LIB3DS_INCLUDE_DIR)
	MESSAGE(STATUS "Could NOT find lib3ds headers.")
ENDIF(LIB3DS_INCLUDE_DIR)

IF(LIB3DS_LIBRARY_DIR)
	MESSAGE(STATUS "Found lib3ds library: ${LIB3DS_LIBRARY}")
ELSE(LIB3DS_LIBRARY_DIR)
	MESSAGE(STATUS "Could NOT find lib3ds library.")
ENDIF(LIB3DS_LIBRARY_DIR)


IF(LIB3DS_INCLUDE_DIR AND LIB3DS_LIBRARY_DIR)
	SET(LIB3DS_FOUND TRUE)
ELSE(LIB3DS_INCLUDE_DIR AND LIB3DS_LIBRARY_DIR)
	SET(LIB3DS_FOUND FALSE)
	IF(Lib3ds_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find lib3ds. Please install lib3ds (http://lib3ds.sourceforge.net)")
	ENDIF(Lib3ds_FIND_REQUIRED)
ENDIF(LIB3DS_INCLUDE_DIR AND LIB3DS_LIBRARY_DIR)

