project(bogl)

include_directories(
	${CMAKE_SOURCE_DIR}/bodebug
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(bogl_SRCS
	bogl.cpp
	bogl_resolve.cpp
	bogl_resolve_symbols.cpp
	myqlibrary.cpp
)

boson_add_library(bogl STATIC ${bogl_SRCS})


