# - Check if the symbol exists in include files
# CHECK_SYMBOL_EXISTS(SYMBOL FILES VARIABLE)
#
#  SYMBOL   - symbol
#  FILES    - include files to check
#  VARIABLE - variable to return result
#  
# If CMAKE_REQUIRED_FLAGS is set then those flags will be passed into the
# compile of the program likewise if CMAKE_REQUIRED_LIBRARIES is set then
# those libraries will be linked against the test program. If CMAKE_REQUIRED_INCLUDES
# is set then these directories will be added to the include search path.


MACRO(CHECK_SYMBOL_EXISTS SYMBOL FILES VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(CHECK_SYMBOL_EXISTS_CONTENT "/* */\n")
    SET(MACRO_CHECK_SYMBOL_EXISTS_FLAGS ${CMAKE_REQUIRED_FLAGS})
    SET(CHECK_SYMBOL_EXISTS_LIBS)
    SET(CHECK_SYMBOL_EXISTS_ADD_INCLUDES)
    IF(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_SYMBOL_EXISTS_LIBS 
        "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    ENDIF(CMAKE_REQUIRED_LIBRARIES)
    IF(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_SYMBOL_EXISTS_ADD_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    ENDIF(CMAKE_REQUIRED_INCLUDES)
    FOREACH(FILE ${FILES})
      SET(CHECK_SYMBOL_EXISTS_CONTENT
        "${CHECK_SYMBOL_EXISTS_CONTENT}#include <${FILE}>\n")
    ENDFOREACH(FILE)
    SET(CHECK_SYMBOL_EXISTS_CONTENT
      "${CHECK_SYMBOL_EXISTS_CONTENT}\nvoid cmakeRequireSymbol(int dummy,...){(void)dummy;}\nint main()\n{\n#ifndef ${SYMBOL}\n  cmakeRequireSymbol(0,&${SYMBOL});\n#endif\n  return 0;\n}\n")

    FILE(WRITE ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckSymbolExists.c 
      "${CHECK_SYMBOL_EXISTS_CONTENT}")

    MESSAGE(STATUS "Looking for ${SYMBOL}")
    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckSymbolExists.c
      CMAKE_FLAGS 
      -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_SYMBOL_EXISTS_FLAGS}
      "${CHECK_SYMBOL_EXISTS_ADD_INCLUDES}"
      "${CHECK_SYMBOL_EXISTS_LIBS}"
      OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      MESSAGE(STATUS "Looking for ${SYMBOL} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have symbol ${SYMBOL}")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log 
        "Determining if the ${SYMBOL} "
        "exist passed with the following output:\n"
        "${OUTPUT}\nFile ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckSymbolExists.c:\n"
        "${CHECK_SYMBOL_EXISTS_CONTENT}\n")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Looking for ${SYMBOL} - not found.")
      SET(${VARIABLE} "" CACHE INTERNAL "Have symbol ${SYMBOL}")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log 
        "Determining if the ${SYMBOL} "
        "exist failed with the following output:\n"
        "${OUTPUT}\nFile ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckSymbolExists.c:\n"
        "${CHECK_SYMBOL_EXISTS_CONTENT}\n")
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_SYMBOL_EXISTS)
