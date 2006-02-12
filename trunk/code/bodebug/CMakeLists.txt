project(libbodebug)

add_subdirectory(bodebugdialog)

include_directories(
	${QT_INCLUDE_DIR}
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
kde3_add_dcop_skels(bodebug_sources ${bodebug_DCOP_SKEL_SOURCES})

kde3_automoc( ${bodebug_sources} )

add_library(bodebug STATIC ${bodebug_sources})


