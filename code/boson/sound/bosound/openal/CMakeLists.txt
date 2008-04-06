project(libbosoundopenal)

include_directories(
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(bosonsoundopenal_sources
	bosonaudioal.cpp
	bosonsound.cpp
	bosonmusic.cpp
)

boson_add_library(bosonsoundopenal STATIC ${bosonsoundopenal_sources})

