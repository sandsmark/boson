project(libbodebug)

include(CheckFunctionExists)
check_function_exists(backtrace HAVE_BACKTRACE)

# create config.h
#include(ConfigureChecks.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-bodebug.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-bodebug.h )


add_subdirectory(bodebugdialog)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(bodebug_sources
	bodebug.cpp
	bodebuglog.cpp
	bodebugdcopiface.cpp
)

set(bodebug_DCOP_SKEL_SOURCES
	bodebugdcopiface.h
)
#kde3_add_dcop_skels(bodebug_sources ${bodebug_DCOP_SKEL_SOURCES})

boson_add_library(bodebug STATIC ${bodebug_sources})
target_link_libraries(bodebug Qt5::Widgets)


