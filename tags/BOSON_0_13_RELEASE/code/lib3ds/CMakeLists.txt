project(lib3ds)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(lib3ds_SRCS
	io.c
	float.c
	vector.c
	matrix.c
	quat.c
	tcb.c
	ease.c
	chunk.c
	file.c
	background.c
	atmosphere.c
	shadow.c
	viewport.c
	material.c
	mesh.c
	camera.c
	light.c
	tracks.c
	node.c
)

# AB: lib3ds does not like -ansi
#     so we just use the flags that lib3ds uses here and throw the normal flags
#     away.
set(CMAKE_C_FLAGS "")
boson_add_library(lib3ds STATIC ${lib3ds_SRCS})

