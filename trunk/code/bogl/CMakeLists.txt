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
	gl/bogl_resolve_symbols.cpp
	myqlibrary.cpp
)

find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
	if(NOT OPENGL_gl_LIBRARY)
		message(FATAL_ERROR "The OpenGL library was not found on your system")
	endif(NOT OPENGL_gl_LIBRARY)
	if(NOT OPENGL_INCLUDE_DIR)
		message(FATAL_ERROR "The OpenGL headers were not found on your system")
	endif(NOT OPENGL_INCLUDE_DIR)
	message(FATAL_ERROR "OpenGL was not found on your system")
endif(NOT OPENGL_FOUND)
if(NOT OPENGL_GLU_FOUND)
	message(FATAL_ERROR "GLU was not found on your system. Please install libGLU")
endif(NOT OPENGL_GLU_FOUND)

# find_package(OpenGL) does not tell us whether glu.h was actually found. check
# it manually.
find_file(_glu_h GL/glu.h ${OPENGL_INCLUDE_DIR})
if(NOT _glu_h)
	message(FATAL_ERROR "GLU header (GL/glu.h) was not found on your system.  Please install the development package (i.e. the header) of libGLU")
ENDif(NOT _glu_h)

boson_add_library(bogl STATIC ${bogl_SRCS})

if("${BOGL_DO_DLOPEN}" STREQUAL "0")
	target_link_libraries(bogl ${OPENGL_LIBRARIES})
endif("${BOGL_DO_DLOPEN}" STREQUAL "0")

