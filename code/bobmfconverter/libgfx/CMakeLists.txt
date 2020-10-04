
INCLUDE(CheckFunctionExists)
Include(${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)

CHECK_INCLUDE_FILE( sys/time.h HAVE_SYS_TIME_H )
CHECK_INCLUDE_FILE_CXX( sstream HAVE_SSTREAM )
CHECK_INCLUDE_FILE_CXX( strstream HAVE_STRSTREAM )
set(HAVE_BOOL 1)
set(CMAKE_REQUIRED_LIBRARIES_ORIG ${CMAKE_REQUIRED_LIBRARIES})
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} m)
check_function_exists( rint HAVE_RINT )
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES_ORIG})
check_function_exists( getrusage HAVE_GETRUSAGE )
check_function_exists( times HAVE_TIMES )
check_function_exists( random HAVE_RANDOM )
check_function_exists( getopt HAVE_GETOPT )
check_function_exists( getopt_long HAVE_GETOPT_LONG )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-libgfx.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-libgfx.h )


