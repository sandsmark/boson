include_directories(
	${CMAKE_SOURCE_DIR}/kgame
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_SOURCE_DIR}/bogl
	${CMAKE_SOURCE_DIR}/boson
	${CMAKE_SOURCE_DIR}/boson/gameengine
	${CMAKE_SOURCE_DIR}/boufo
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

# unit tests
set(tests_SRCS
	main.cpp
	testframework.cpp
	maptest.cpp
	playfieldtest.cpp
	canvastest.cpp
)
kde3_automoc(${tests_SRCS})

boson_add_executable(tests ${tests_SRCS})
boson_target_link_libraries(tests
	gameengine
	common
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${LIB_BOMEMORY}
)

