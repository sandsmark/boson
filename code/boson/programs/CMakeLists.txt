project(boson)

include_directories(
	${CMAKE_SOURCE_DIR}/kgame
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_SOURCE_DIR}/bogl
	${CMAKE_SOURCE_DIR}/boson
	${CMAKE_SOURCE_DIR}/boson/gameengine
	${CMAKE_SOURCE_DIR}/boson/boufo
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)


################ boson #################
kde3_add_executable(boson
	main.cpp
)
target_link_libraries(boson
	bosongamelib
)


################ boson-no-gui #################
set(bosonNoGUI_SRCS
	../bocheckinstallation.cpp
	mainnogui.cpp
	mainnoguimain.cpp
)
kde3_automoc(${bosonNoGUI_SRCS})
kde3_add_executable(boson-no-gui ${bosonNoGUI_SRCS})
target_link_libraries(boson-no-gui
	gameengine
	boufo
	common
	kio
	${QT_AND_KDECORE_LIBS}
)


################ bounit #################
set(bounit_SRCS
	bounitmain.cpp
	bosonsearchpathswidget.cpp
	bouniteditor.cpp
)
kde3_add_ui_files(bounit_SRCS
	bosonsearchpathswidgetbase.ui
	bouniteditorbase.ui
)
kde3_automoc(${bounit_SRCS})
kde3_add_executable(bounit ${bounit_SRCS})
target_link_libraries(bounit
	gameengine
	common
	bosoninfo
	kio
	kdeui
	${QT_AND_KDECORE_LIBS}
	${X11_XMU_LIB}
)



################ borender #################
set(borender_SRCS
	../kgame3dsmodeldebug.cpp
	../bomaterialwidget.cpp
	../bovectorinput.cpp
	../kgamespeciesdebug.cpp
	borendermain.cpp
	borenderrendermodel.cpp
	../bopixmaprenderer.cpp
	boeditturretpropertiesdialog.cpp
)
boson_add_boui_files(borender_SRCS
	borendergui.boui
)
kde3_automoc(${borender_SRCS})
kde3_add_executable(borender ${borender_SRCS})
target_link_libraries(borender
	bosonmainlib
	boufo
	3ds
	${X11_XMU_LIB}
)

################ bocursor #################
set(bocursor_SRCS
	../bosoncursor.cpp
	../botexture.cpp
	../bosonglwidget.cpp

	bocursormain.cpp
	bosoncursoreditor.cpp
)
kde3_automoc(${bocursor_SRCS})
kde3_add_executable(bocursor ${bocursor_SRCS})
target_link_libraries(bocursor
	common
	bosoninfo
	kio
	kdeui
	${QT_AND_KDECORE_LIBS}
	${X11_XMU_LIB}
)

################ bomodelpixmaps #################
set(bomodelpixmaps_SRCS
	bomodelpixmaps.cpp
	borenderrendermodel.cpp
	../bopixmaprenderer.cpp
)
kde3_add_ui_files(bomodelpixmaps_SRCS
	bomodelpixmapsgui.ui
)
kde3_automoc(${bomodelpixmaps_SRCS})
kde3_add_executable(bomodelpixmaps ${bomodelpixmaps_SRCS})
target_link_libraries(bomodelpixmaps
	bosonmainlib
	boufo
	3ds
	${X11_XMU_LIB}
)

################ boinfo #################
set(boinfo_SRCS
	../bosonglwidget.cpp
	boinfomain.cpp
)
kde3_automoc(${boinfo_SRCS})
kde3_add_executable(boinfo ${boinfo_SRCS})
target_link_libraries(boinfo
	bosoninfo
	common
	kio
	kdeui
	${QT_AND_KDECORE_LIBS}
	${X11_XMU_LIB}
)

################ bo3dsdebug #################
set(bo3dsdebug_SRCS
	../kgame3dsmodeldebug.cpp
	../bomatrixwidget.cpp

	bo3dsdebugmain.cpp
)
kde3_automoc(${bo3dsdebug_SRCS})
kde3_add_executable(bo3dsdebug ${bo3dsdebug_SRCS})
target_link_libraries(bo3dsdebug
	common
	boufo
	kio
	kdeui
	${QT_AND_KDECORE_LIBS}
	3ds
)

################ boconditioneditor #################
set(boconditioneditor_SRCS
	../boconditionwidget.cpp
	boconditioneditormain.cpp
)
kde3_automoc(${boconditioneditor_SRCS})
kde3_add_executable(boconditioneditor ${boconditioneditor_SRCS})
target_link_libraries(boconditioneditor
	gameengine
	common
	boufo
	kio
	kdeui
	${QT_AND_KDECORE_LIBS}
)

################ boprofiling #################
set(boprofiling_SRCS
	../bosonprofilingdialog.cpp
	boprofilingmain.cpp
)
kde3_add_ui_files(boprofiling_SRCS
	../bosonprofilingdialoggui.ui
)
kde3_automoc(${boprofiling_SRCS})
kde3_add_executable(boprofiling ${boprofiling_SRCS})
target_link_libraries(boprofiling
	common
	kio
	kdeui
	${QT_AND_KDECORE_LIBS}
	${X11_XMU_LIB}
)


################ bocommandframetester #################
set(bocommandframetester_SRCS
	bocommandframetestermain.cpp
	mainnogui.cpp
)
kde3_automoc(${bocommandframetester_SRCS})
kde3_add_executable(bocommandframetester
	${bocommandframetester_SRCS}
)
target_link_libraries(bocommandframetester
	bosongamelib
)

install_targets(/bin
	boson
	boson-no-gui
	bounit
	borender
	bocursor
	bomodelpixmaps
	boinfo
	bo3dsdebug
	boconditioneditor
	boprofiling
	bocommandframetester
)
