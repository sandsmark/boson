project(boson)

add_subdirectory( data )
add_subdirectory( script )
add_subdirectory( info )
add_subdirectory( boufo )
add_subdirectory( gameview/plugin )
add_subdirectory( meshrenderer )
add_subdirectory( groundrenderer )
add_subdirectory( bosound )

include(BosonMacros)

CHECK_INCLUDE_FILE( sys/time.h HAVE_SYS_TIME_H )

# TODO: USE_BO_PLUGINS
# TODO: BOSON_LINK_STATIC

include_directories(
	${CMAKE_SOURCE_DIR}/kgame
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_SOURCE_DIR}/bogl
	${CMAKE_CURRENT_SOURCE_DIR}/sound/bosound
	${CMAKE_CURRENT_SOURCE_DIR}/boufo
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(imageio_SRCS
	imageio/boimageio.cpp
	imageio/tga.cpp
	imageio/pcx.cpp
)

set(common_SRCS
	boapplication.cpp
	boglobal.cpp
	bosonconfig.cpp
	bosondata.cpp
	bomath.cpp
	bo3dtools.cpp
	bosonprofiling.cpp
	bopluginmanager.cpp
	qlistviewitemnumber.cpp
)
kde3_automoc(${common_SRCS})
set(common_LIBS
	bodebug
	bogl
	bomath
	imageio
)

set(gameengine_SRCS
	boeventloop.cpp
	boupgradeableproperty.cpp
	unitbase.cpp
	unit.cpp
	unitproperties.cpp
	pluginproperties.cpp
	unitplugins.cpp
	upgradeproperties.cpp
	bosonweapon.cpp
	boitemlist.cpp
	boitemlisthandler.cpp
	bosonplayfield.cpp
	bosonmap.cpp
	cell.cpp
	bofile.cpp
	bpfdescription.cpp
	bosonfileconverter.cpp
	bosoncampaign.cpp
	bosonmessage.cpp
	boson.cpp
	bomessage.cpp
	bosoncanvas.cpp
	bosoncanvasstatistics.cpp
	bosoncollisions.cpp
	bosonnetworksynchronizer.cpp
	bosonnetworktraffic.cpp
	speciestheme.cpp
	player.cpp
	playerio.cpp
	bosoncomputerio.cpp
	bosonpath.cpp
	bosongamestatistics.cpp
	bosonstatistics.cpp
	bogameproperty.cpp
	bosonpropertyxml.cpp
	bosonplayerinputhandler.cpp
	bobincoder.cpp
	boevent.cpp
	boeventmatching.cpp
	boeventlistener.cpp
	boeventmanager.cpp
	bocondition.cpp
	bowater.cpp
	bosonsaveload.cpp
	bosongroundtheme.cpp
	bosongameengine.cpp
	bosonstarting.cpp
	bosongameenginestarting.cpp
	boquadtreenode.cpp
	boquadtreecollection.cpp
	bogroundquadtreenode.cpp
	bocanvasquadtreenode.cpp
	items/bosonitem.cpp
	items/bosonshot.cpp
	items/bosonitempropertyhandler.cpp
)
kde3_automoc(${gameengine_SRCS})

set(gameengine_LIBS
	bosonscript
	kgame
)

set(startupwidgets_SRCS
	startupwidgets/boufocolorchooser.cpp
	startupwidgets/bosonstartupnetwork.cpp
	startupwidgets/boufonewgamewidget.cpp
	startupwidgets/boufostartupwidget.cpp
	startupwidgets/boufoloadingwidget.cpp
	startupwidgets/boufonetworkoptionswidget.cpp
	startupwidgets/boufostarteditorwidget.cpp
	startupwidgets/boufoloadsavegamewidget.cpp
	startupwidgets/bocreatenewmap.cpp
)
set(startupwidgets_BOUI
	startupwidgets/welcomewidget.boui
	startupwidgets/newgamewidget.boui
	startupwidgets/boufonetworkoptionswidgetbase.boui
	startupwidgets/boufostarteditorwidgetbase.boui
)
boson_add_boui_files(startupwidgets_SRCS ${startupwidgets_BOUI})
kde3_automoc(${startupwidgets_SRCS})

set(commandframe_SRCS
	gameview/commandframe/bosoncommandframe.cpp
	gameview/commandframe/bosonorderwidget.cpp
	gameview/commandframe/bosonorderbutton.cpp
	gameview/commandframe/boactionswidget.cpp
	gameview/commandframe/bosonunitview.cpp
	gameview/commandframe/editorunitconfigwidget.cpp
	gameview/commandframe/bocommandframeeventlistener.cpp
)
kde3_automoc(${commandframe_SRCS})

set(gameview_SRCS
	items/bosonitemrenderer.cpp
	gameview/bosonufogamegui.cpp
	gameview/bosonufocanvaswidget.cpp
	gameview/bosonufogamewidgets.cpp
	gameview/bosongameview.cpp
	gameview/bosongamevieweventlistener.cpp
	gameview/bosoncanvasrenderer.cpp
	gameview/bosongameviewinputbase.cpp
	gameview/bosongameviewinput.cpp
	gameview/editorviewinput.cpp
	gameview/bosonlocalplayerinput.cpp
	gameview/bolocalplayereventlistener.cpp
	gameview/bosonmenuinput.cpp
	gameview/boeditplayerinputswidget.cpp
)
set(gameview_BOUI
	gameview/bosonufogameguihelper.boui
)
boson_add_boui_files(gameview_SRCS ${gameview_BOUI})
kde3_automoc(${gameview_SRCS})


set(libboson_SRCS
	bosonviewdata.cpp
	boselection.cpp
	botooltipcreator.cpp
	bosonglwidget.cpp
	bosonufoglwidget.cpp
	bomousemovediff.cpp
	bomatrixwidget.cpp
	bocamerawidget.cpp
	boorbiterwidget.cpp
	bosonufochat.cpp
	bosonprofilingdialog.cpp
	bpfdescriptiondialog.cpp
	boglstatewidget.cpp
	bodebuglogdialog.cpp
	bofullscreen.cpp
	bocamera.cpp
	boautocamera.cpp
	bosoncursor.cpp
	bosonmodel.cpp
	bosonmodeltextures.cpp
	bomesh.cpp
	bomeshrenderer.cpp
	bomeshrenderermanager.cpp
	bomaterial.cpp
	bolight.cpp
	botexture.cpp
	bobmfload.cpp
	boshader.cpp
	speciesdata.cpp
	boaction.cpp
	bosongroundthemedata.cpp
	borendertarget.cpp
	bonuminput.cpp
	sound/bosonaudiointerface.cpp
)
kde3_automoc(${libboson_SRCS})


# TODO: if not plugins: meshrenderer, groundrenderer, gameview plugins
set(libboson_LIBS
	bosoninfo
	bosonsound
	bosonsoundcommon
	gameengine
	common
#	${common_LIBS}
	kio
	3ds

	${X11_XRANDR_LIB}
)



set(boson_SRCS
	bosonfpscounter.cpp
	bosonmainwidget.cpp
	bosonmainwidgetmenuinput.cpp
	boufoactionext.cpp
	bogltooltip.cpp
	bosonglminimap.cpp
	bosonufominimap.cpp
	optionsdialog.cpp
	optionswidgets.cpp
	bogroundrenderer.cpp
	bogroundrenderermanager.cpp
	kgameunitdebug.cpp
	kgameadvancemessagesdebug.cpp
	kgameplayerdebug.cpp
	kgamecelldebug.cpp
	gameoverdialog.cpp
	bocheckinstallation.cpp
	bosonguistarting.cpp
	bosongameviewpluginbase.cpp
	bosongameviewpluginmanager.cpp
	bosoneffectmanager.cpp
	bosoneffect.cpp
	bosoneffectproperties.cpp
	bosoneffectparticle.cpp
	bosoneffectpropertiesparticle.cpp
	botexmapimportdialog.cpp
	boconditionwidget.cpp
	bowaterrenderer.cpp
	selectbox.cpp
	main.cpp
	bosondebugtextures.cpp
	bosondebugmodels.cpp

	${startupwidgets_SRCS}
	${commandframe_SRCS}
	${gameview_SRCS}
)
set(boson_UI
	bosondebugtexturesbase.ui
	bosondebugmodelsbase.ui
)
kde3_add_ui_files( boson_SRCS ${boson_UI} )
kde3_add_ui_files( foobar ${boson_UI} )
kde3_automoc( ${boson_SRCS} )



add_library(imageio STATIC ${imageio_SRCS})
add_library(common STATIC ${common_SRCS})
target_link_libraries(common ${common_LIBS})

add_library(bosonlib STATIC ${libboson_SRCS})
target_link_libraries(bosonlib ${libboson_LIBS})

add_library(gameengine STATIC ${gameengine_SRCS})
target_link_libraries(gameengine ${gameengine_LIBS})


################ boson #################
kde3_add_executable(boson ${boson_SRCS})
target_link_libraries(boson
	bosonlib
	boufo
	${X11_XMU_LIB}
)

################ boson-no-bui #################
set(bosonNoGUI_SRCS
	bosoncomputerio.cpp
	bocheckinstallation.cpp
	mainnogui.cpp
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

################ borender #################
set(borender_SRCS
	kgame3dsmodeldebug.cpp
	bomaterialwidget.cpp
	bovectorinput.cpp
	kgamespeciesdebug.cpp
	borendermain.cpp
	borenderrendermodel.cpp
	bopixmaprenderer.cpp
	boeditturretpropertiesdialog.cpp
)
boson_add_boui_files(borender_SRCS
	borendergui.boui
)
kde3_automoc(${borender_SRCS})
kde3_add_executable(borender ${borender_SRCS})
target_link_libraries(borender
	bosonlib
	boufo
	3ds
	${X11_XMU_LIB}
)

################ bounit #################
set(bounit_SRCS
	bounitmain.cpp
	bosonsearchpathswidget.cpp
	bouniteditor.cpp
)
set(bounit_UI
	bosonsearchpathswidgetbase.ui
	bouniteditorbase.ui
)
kde3_add_ui_files(bounit_SRCS ${bounit_UI})
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

################ bocursor #################
set(bocursor_SRCS
	bosoncursor.cpp
	botexture.cpp
	bosonglwidget.cpp

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
	bopixmaprenderer.cpp
)
kde3_add_ui_files(bomodelpixmaps_SRCS
	bomodelpixmapsgui.ui
)
kde3_automoc(${bomodelpixmaps_SRCS})
kde3_add_executable(bomodelpixmaps ${bomodelpixmaps_SRCS})
target_link_libraries(bomodelpixmaps
	bosonlib
	boufo
	3ds
	${X11_XMU_LIB}
)

################ boinfo #################
set(boinfo_SRCS
	bosonglwidget.cpp
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
	kgame3dsmodeldebug.cpp
	bomatrixwidget.cpp

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
	boconditionwidget.cpp
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
	bosonprofilingdialog.cpp
	boprofilingmain.cpp
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

