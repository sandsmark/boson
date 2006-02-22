# - Check if the source code provided in the SOURCE argument compiles.
# CHECK_CXX_SOURCE_COMPILES(SOURCE VAR)
# - macro which checks if the source code compiles\
#  SOURCE - source code to try to compile
#  VAR    - variable to store size if the type exists.
# Checks the following optional VARIABLES (not arguments)
#  CMAKE_REQUIRED_LIBRARIES - Link to extra libraries
#  CMAKE_REQUIRED_FLAGS     - Extra flags to C compiler
#

MACRO(CHECK_CXX_SOURCE_COMPILES SOURCE VAR)
  IF("${VAR}" MATCHES "^${VAR}$")
    SET(MACRO_CHECK_FUNCTION_DEFINITIONS 
      "-D${VAR} ${CMAKE_REQUIRED_FLAGS}")
    SET(CHECK_CXX_SOURCE_COMPILES_ADD_LIBRARIES)
    SET(CHECK_CXX_SOURCE_COMPILES_ADD_INCLUDES)
    IF(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_CXX_SOURCE_COMPILES_ADD_LIBRARIES
        "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    ENDIF(CMAKE_REQUIRED_LIBRARIES)
    IF(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_CXX_SOURCE_COMPILES_ADD_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    ENDIF(CMAKE_REQUIRED_INCLUDES)
    FILE(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/src.cxx"
      "${SOURCE}")

    MESSAGE(STATUS "Performing Test ${VAR}")
    TRY_COMPILE(${VAR}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/src.cxx
      CMAKE_FLAGS 
      "${CHECK_CXX_SOURCE_COMPILES_ADD_LIBRARIES}"
      "${CHECK_CXX_SOURCE_COMPILES_ADD_INCLUDES}"
      OUTPUT_VARIABLE OUTPUT)
    IF(${VAR})
      SET(${VAR} 1 CACHE INTERNAL "Test ${FUNCTION}")
      MESSAGE(STATUS "Performing Test ${VAR} - Success")
      WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log 
        "Performing C++ SOURCE FILE Test ${VAR} succeded with the following output:\n"
        "${OUTPUT}\n"
        "Source file was:\n${SOURCE}\n" APPEND)
    ELSE(${VAR})
      MESSAGE(STATUS "Performing Test ${VAR} - Failed")
      SET(${VAR} "" CACHE INTERNAL "Test ${FUNCTION}")
      WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log 
        "Performing C++ SOURCE FILE Test ${VAR} failed with the following output:\n"
        "${OUTPUT}\n"
        "Source file was:\n${SOURCE}\n" APPEND)
    ENDIF(${VAR})
  ENDIF("${VAR}" MATCHES "^${VAR}$")
ENDMACRO(CHECK_CXX_SOURCE_COMPILES)

