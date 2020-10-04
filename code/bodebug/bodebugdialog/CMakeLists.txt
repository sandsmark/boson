project(bodebugdialog)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(bodebugdialog_sources
	boabstractdebugdialog.cpp
	bodebugdialog.cpp
	bolistdebugdialog.cpp
	main.cpp
)

boson_add_executable(bodebugdialog ${bodebugdialog_sources})
boson_target_link_libraries(bodebugdialog
	bodebug
	kdeui
	${QT_AND_KDECORE_LIBS}
)


install_targets(/bin bodebugdialog)
