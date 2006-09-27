project(bodebugdialog)

include_directories(
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(bodebugdialog_sources
	boabstractdebugdialog.cpp
	bodebugdialog.cpp
	bolistdebugdialog.cpp
	main.cpp
)

kde3_automoc(${bodebugdialog_sources})


boson_add_executable(bodebugdialog ${bodebugdialog_sources})
boson_target_link_libraries(bodebugdialog
	bodebug
	kdeui
	${QT_AND_KDECORE_LIBS}
)


install_targets(/bin bodebugdialog)
