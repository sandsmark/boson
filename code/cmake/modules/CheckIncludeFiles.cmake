# - Check if the files can be included
#
# CHECK_INCLUDE_FILES(INCLUDE VARIABLE)
#
#  INCLUDE  - list of files to include
#  VARIABLE - variable to return result
#   
# If CMAKE_REQUIRED_FLAGS is set then those flags will be passed into the
# compile of the program
# additionally to the original cmake module this one uses CMAKE_REQUIRED_INCLUDES
# where you can specify a directory which will be added to the include path

MACRO(CHECK_INCLUDE_FILES INCLUDE VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
   
    SET(CHECK_INCLUDE_FILES_INCLUDE_DIRS "-DINCLUDE_DIRECTORIES=${CMAKE_REQUIRED_INCLUDES}")
#    STRING(REGEX REPLACE ";" "\\\\;" CHECK_INCLUDE_FILES_INCLUDE_DIRS "${CHECK_INCLUDE_FILES_INCLUDE_DIRS}")
    
    SET(CHECK_INCLUDE_FILES_CONTENT "/* */\n")
    SET(MACRO_CHECK_INCLUDE_FILES_FLAGS ${CMAKE_REQUIRED_FLAGS})
    FOREACH(FILE ${INCLUDE})
      SET(CHECK_INCLUDE_FILES_CONTENT
        "${CHECK_INCLUDE_FILES_CONTENT}#include <${FILE}>\n")
    ENDFOREACH(FILE)
    SET(CHECK_INCLUDE_FILES_CONTENT
      "${CHECK_INCLUDE_FILES_CONTENT}\n\nint main(){return 0;}\n")
    FILE(WRITE ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckIncludeFiles.c 
      "${CHECK_INCLUDE_FILES_CONTENT}")

    MESSAGE(STATUS "Looking for include files ${VARIABLE}")
    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckIncludeFiles.c
      CMAKE_FLAGS
      -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_INCLUDE_FILES_FLAGS}
      "${CHECK_INCLUDE_FILES_INCLUDE_DIRS}"
      OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      MESSAGE(STATUS "Looking for include files ${VARIABLE} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have include ${VARIABLE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeFiles/CMakeOutput.log 
        "Determining if files ${INCLUDE} "
        "exist passed with the following output:\n"
        "${OUTPUT}\n\n")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Looking for include files ${VARIABLE} - not found.")
      SET(${VARIABLE} "" CACHE INTERNAL "Have includes ${VARIABLE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log 
        "Determining if files ${INCLUDE} "
        "exist failed with the following output:\n"
        "${OUTPUT}\nSource:\n${CHECK_INCLUDE_FILES_CONTENT}\n")
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_INCLUDE_FILES)
