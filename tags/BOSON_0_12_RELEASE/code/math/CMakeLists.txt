project(bomath)

set(math_SRCS
	bomath.cpp
	bofixed.cpp
	bo3dtoolsbase.cpp
	bovector.cpp
	borect.cpp
	bomatrix.cpp
	boquaternion.cpp
	boplane.cpp
	bofrustum.cpp
	boglmatrices.cpp
)

add_library(bomath STATIC ${math_SRCS})

