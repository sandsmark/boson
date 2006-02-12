
FIND_PATH(OPENSSL_INCLUDE_DIR openssl/ssl.h
/usr/include/
/usr/local/include/
)

FIND_LIBRARY(OPENSSL_LIBRARY NAMES ssl ssleay32
PATHS
/usr/lib
/usr/local/lib
)

IF(OPENSSL_INCLUDE_DIR AND OPENSSL_LIBRARY)
   SET(OPENSSL_FOUND TRUE)
ENDIF(OPENSSL_INCLUDE_DIR AND OPENSSL_LIBRARY)


IF(OPENSSL_FOUND)
   IF(NOT OpenSSL_FIND_QUIETLY)
      MESSAGE(STATUS "Found OpenSSL: ${OPENSSL_LIBRARY}")
   ENDIF(NOT OpenSSL_FIND_QUIETLY)
ELSE(OPENSSL_FOUND)
   IF(OpenSSL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find OpenSSL")
   ENDIF(OpenSSL_FIND_REQUIRED)
ENDIF(OPENSSL_FOUND)
