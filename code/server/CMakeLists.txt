project(boserver)

include_directories(
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(boserver_SRCS
	main.cpp
	server.cpp
	webinterface.cpp
	game.cpp
	player.cpp
)


boson_add_executable(boserver ${boserver_SRCS})
boson_target_link_libraries(boserver
	kgame
	bodebug
	${QT_AND_KDECORE_LIBS}
)

install_targets(/bin
	boserver
)

