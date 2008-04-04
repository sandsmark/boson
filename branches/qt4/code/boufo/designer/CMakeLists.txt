project(boufodesigner)

include_directories(
  ${CMAKE_SOURCE_DIR}/bodebug
  ${CMAKE_SOURCE_DIR}/bogl
  ${CMAKE_SOURCE_DIR}/ufo/include
  ${CMAKE_SOURCE_DIR}/boufo
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)


set(boufodesigner_SRCS
	boufodesignermain.cpp
	formpreview.cpp
	bowidgetlist.cpp
	bowidgettree.cpp
	bopropertieswidget.cpp
	bosignalsslotseditor.cpp
	optionsdialog.cpp
	main.cpp
)

kde3_automoc(${boufodesigner_SRCS})

boson_add_executable(boufodesigner ${boufodesigner_SRCS})

boson_target_link_libraries(boufodesigner
	boufo
	bodebug
	bogl
	${QT_AND_KDECORE_LIBS}
)

install_targets(/bin
	boufodesigner
)
