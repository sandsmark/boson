project(boson)

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


################ boson #################

# AB: WARNING: this SUCKS!
#              bosongameviewpluginbase.cpp is already included in bosongamelib,
#              but the symbols are discarded if we just link to that lib.
#              however the gameview plugin requires these libs, so we compile
#              and link to that file again here :-(
#              TODO: how can we convince g++ not to discard symbols from static
#              libs?
set(boson_SRCS
	main.cpp
	../bosongameviewpluginbase.cpp
)
kde3_automoc(${boson_SRCS})
boson_add_executable(boson
	${boson_SRCS}
)
boson_target_link_libraries(boson
	bosongamelib
	${LIB_BOMEMORY}
)


################ boson-no-gui #################
set(bosonNoGUI_SRCS
	../bocheckinstallation.cpp
	mainnogui.cpp
	mainnoguimain.cpp
)
kde3_automoc(${bosonNoGUI_SRCS})
boson_add_executable(boson-no-gui ${bosonNoGUI_SRCS})
boson_target_link_libraries(boson-no-gui
	gameengine
	boufo
	common
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${LIB_BOMEMORY}
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
boson_add_executable(bounit ${bounit_SRCS})
boson_target_link_libraries(bounit
	gameengine
	common
	bosoninfo
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${X11_XMU_LIB}
	${LIB_BOMEMORY}
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
boson_add_executable(borender ${borender_SRCS})
boson_target_link_libraries(borender
	bosonmainlib
	boufo
	${LIB3DS_LIBRARY}
	${X11_XMU_LIB}
	${LIB_BOMEMORY}
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
boson_add_executable(bocursor ${bocursor_SRCS})
boson_target_link_libraries(bocursor
	common
	bosoninfo
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${X11_XMU_LIB}
	${LIB_BOMEMORY}
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
boson_add_executable(bomodelpixmaps ${bomodelpixmaps_SRCS})
boson_target_link_libraries(bomodelpixmaps
	bosonmainlib
	boufo
	${LIB3DS_LIBRARY}
	${X11_XMU_LIB}
	${LIB_BOMEMORY}
)

################ boinfo #################
set(boinfo_SRCS
	../bosonglwidget.cpp
	boinfomain.cpp
)
kde3_automoc(${boinfo_SRCS})
boson_add_executable(boinfo ${boinfo_SRCS})
boson_target_link_libraries(boinfo
	bosoninfo
	common
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${X11_XMU_LIB}
	${LIB_BOMEMORY}
)

################ bo3dsdebug #################
set(bo3dsdebug_SRCS
	../kgame3dsmodeldebug.cpp
	../bomatrixwidget.cpp

	bo3dsdebugmain.cpp
)
kde3_automoc(${bo3dsdebug_SRCS})
boson_add_executable(bo3dsdebug ${bo3dsdebug_SRCS})
boson_target_link_libraries(bo3dsdebug
	common
	boufo
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${LIB3DS_LIBRARY}
	${LIB_BOMEMORY}
)

################ boconditioneditor #################
set(boconditioneditor_SRCS
	../boconditionwidget.cpp
	boconditioneditormain.cpp
)
kde3_automoc(${boconditioneditor_SRCS})
boson_add_executable(boconditioneditor ${boconditioneditor_SRCS})
boson_target_link_libraries(boconditioneditor
	gameengine
	common
	boufo
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${LIB_BOMEMORY}
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
boson_add_executable(boprofiling ${boprofiling_SRCS})
boson_target_link_libraries(boprofiling
	common
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${X11_XMU_LIB}
	${LIB_BOMEMORY}
)


################ bocommandframetester #################
set(bocommandframetester_SRCS
	bocommandframetestermain.cpp
	mainnogui.cpp
)
kde3_automoc(${bocommandframetester_SRCS})
#boson_add_executable(bocommandframetester
#	${bocommandframetester_SRCS}
#)
#boson_target_link_libraries(bocommandframetester
#	bosongamelib
#	${LIB_BOMEMORY}
#)

install_targets(/bin
	boson
#	boson-no-gui
	bounit
#	borender
	bocursor
	bomodelpixmaps
	boinfo
#	bo3dsdebug
	boconditioneditor
	boprofiling
#	bocommandframetester
)

################ bofileconverter #################
set(bofileconverter_SRCS
	bofileconvertermain.cpp
)
kde3_automoc(${bofileconverter_SRCS})
boson_add_executable(bofileconverter ${bofileconverter_SRCS})
boson_target_link_libraries(bofileconverter
	gameengine
	common
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${LIB_BOMEMORY}
)

################ bocreatepreview #################
set(bocreatepreview_SRCS
	bocreatepreviewmain.cpp
)
kde3_automoc(${bocreatepreview_SRCS})
boson_add_executable(bocreatepreview ${bocreatepreview_SRCS})
boson_target_link_libraries(bocreatepreview
	gameengine
	common
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${LIB_BOMEMORY}
)

