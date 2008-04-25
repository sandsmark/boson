project(libbodebug)

include_directories(
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(bodebug_sources
	bodebug.cpp
	bodebuglog.cpp
)

boson_add_library(bodebug STATIC ${bodebug_sources})


