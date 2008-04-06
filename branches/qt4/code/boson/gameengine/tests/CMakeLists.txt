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
	unittests/main.cpp
	unittests/testframework.cpp
	unittests/maptest.cpp
	unittests/playfieldtest.cpp
	unittests/canvastest.cpp
	unittests/movetest.cpp
	unittests/constructiontest.cpp
	unittests/productiontest.cpp
)

boson_add_executable(tests ${tests_SRCS})
boson_target_link_libraries(tests
	gameengine
	common
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${LIB_BOMEMORY}
)

# non-unit tests, just some random test applications
boson_add_executable(collisiontest collisiontest.cpp unittests/testframework.cpp)
boson_target_link_libraries(collisiontest
	gameengine
	common
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${LIB_BOMEMORY}
)

