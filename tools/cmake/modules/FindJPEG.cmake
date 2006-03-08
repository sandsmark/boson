# - Find JPEG
# Find the native JPEG includes and library
# This module defines
#  JPEG_INCLUDE_DIR, where to find jpeglib.h, etc.
#  JPEG_LIBRARIES, the libraries needed to use JPEG.
#  JPEG_FOUND, If false, do not try to use JPEG.
# also defined, but not for general use are
#  JPEG_LIBRARY, where to find the JPEG library.

# under windows, try to find the base gnuwin32 directory, do nothing under UNIX
FIND_PACKAGE(GNUWIN32)


FIND_PATH(JPEG_INCLUDE_DIR jpeglib.h
/usr/local/include
/usr/include
${GNUWIN32_DIR}/include
)

SET(JPEG_NAMES ${JPEG_NAMES} jpeg)
FIND_LIBRARY(JPEG_LIBRARY
  NAMES ${JPEG_NAMES}
  PATHS /usr/lib /usr/local/lib ${GNUWIN32_DIR}/lib
  )

IF (JPEG_LIBRARY)
  IF (JPEG_INCLUDE_DIR)
    SET(JPEG_LIBRARIES ${JPEG_LIBRARY})
    SET(JPEG_FOUND TRUE)
  ENDIF (JPEG_INCLUDE_DIR)
ENDIF (JPEG_LIBRARY)

# Deprecated declarations.
SET (NATIVE_JPEG_INCLUDE_PATH ${JPEG_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_JPEG_LIB_PATH ${JPEG_LIBRARY} PATH)

MARK_AS_ADVANCED(
  JPEG_LIBRARY
  JPEG_INCLUDE_DIR
  )
