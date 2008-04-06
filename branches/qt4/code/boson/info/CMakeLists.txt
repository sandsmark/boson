include_directories(
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_SOURCE_DIR}/bogl
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

CHECK_INCLUDE_FILE( sys/stat.h HAVE_SYS_STAT_H )

set(info_SRCS
	boinfo.cpp
	bocurrentinfo.cpp
	boinfoglcache.cpp
	boinfo_x11.cpp
	boinfodialog.cpp
	boglquerystates.cpp
)

boson_add_library(bosoninfo STATIC ${info_SRCS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-info.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-info.h )
