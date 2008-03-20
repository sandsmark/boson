project(boufotests)

include_directories(
  ${CMAKE_SOURCE_DIR}/bodebug
  ${CMAKE_SOURCE_DIR}/bogl
  ${CMAKE_SOURCE_DIR}/ufo/include
  ${CMAKE_SOURCE_DIR}/boson/boufo
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)


set(boufotest_SRCS
	boufotestmain.cpp
	boufofontselectionwidget.cpp
)
boson_add_boui_files(boufotest_SRCS
	boufofontselectionwidgetbase.boui
)
kde3_automoc(${boufotest_SRCS})
boson_add_executable(boufotest
	${boufotest_SRCS}
)
boson_target_link_libraries(boufotest
	bodebug
	bogl
	boufo
	${QT_AND_KDECORE_LIBS}
)


set(boufobuttontest_SRCS
	boufobuttontest.cpp
)
kde3_automoc(${boufobuttontest_SRCS})
boson_add_executable(boufobuttontest
	${boufobuttontest_SRCS}
)
boson_target_link_libraries(boufobuttontest
	bodebug
	bogl
	boufo
	${QT_AND_KDECORE_LIBS}
)


set(boufolistboxtest_SRCS
	boufolistboxtest.cpp
)
kde3_automoc(${boufolistboxtest_SRCS})
boson_add_executable(boufolistboxtest
	${boufolistboxtest_SRCS}
)
boson_target_link_libraries(boufolistboxtest
	bodebug
	bogl
	boufo
	${QT_AND_KDECORE_LIBS}
)


set(boufowidgets_SRCS
	boufowidgets.cpp
)
kde3_automoc(${boufowidgets_SRCS})
boson_add_executable(boufowidgets
	${boufowidgets_SRCS}
)
boson_target_link_libraries(boufowidgets
	bodebug
	bogl
	boufo
	${QT_AND_KDECORE_LIBS}
)

