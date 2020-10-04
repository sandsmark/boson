project(bomemory)

include_directories(
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(bomemory_SRCS
	new.cpp
	memnode.cpp
	memorymanager.cpp
	bomemorydialog.cpp
	malloc.cpp
)

if(BOSON_USE_BOMEMORY)
  boson_add_library(bomemory STATIC ${bomemory_SRCS})
  target_link_library(bomemory Qt5::Widgets)
  set(LIB_BOMEMORY bomemory)
else(BOSON_USE_BOMEMORY)
  set(LIB_BOMEMORY "")
endif(BOSON_USE_BOMEMORY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-bomemory.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-bomemory.h )

