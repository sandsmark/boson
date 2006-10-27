include(BosonMacros)

CHECK_INCLUDE_FILE( sys/time.h HAVE_SYS_TIME_H )
CHECK_INCLUDE_FILE( limits.h HAVE_LIMITS_H )


add_subdirectory( data )
add_subdirectory( gameengine/script )
add_subdirectory( info )
add_subdirectory( gameview/plugin )
add_subdirectory( boufo )
add_subdirectory( modelrendering/meshrenderer )
add_subdirectory( groundrenderer )
add_subdirectory( sound/bosound )
add_subdirectory( programs )

set(LIB_MESHRENDERER_PLUGIN "")
set(LIB_GROUNDRENDERER_PLUGIN "")
set(LIB_GAMEVIEW_PLUGIN "")
if(NOT BOSON_USE_DEBUG_PLUGINS)
  set(LIB_MESHRENDERER_PLUGIN bomeshrendererplugin)
  set(LIB_GROUNDRENDERER_PLUGIN bogroundrendererplugin)
  set(LIB_GAMEVIEW_PLUGIN bosongameviewplugin)
endif(NOT BOSON_USE_DEBUG_PLUGINS)


include_directories(
	${CMAKE_SOURCE_DIR}/kgame
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_SOURCE_DIR}/bogl
	${CMAKE_CURRENT_SOURCE_DIR}/sound/bosound
	${CMAKE_CURRENT_SOURCE_DIR}/boufo
	${CMAKE_CURRENT_SOURCE_DIR}/gameengine
	${KDE3_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)


set(imageio_SRCS
	imageio/boimageio.cpp
	imageio/tga.cpp
	imageio/pcx.cpp
)
boson_add_library(imageio STATIC ${imageio_SRCS})


set(common_SRCS
	boapplication.cpp
	boglobal.cpp
	bosonconfig.cpp
	bosonconfigentries.cpp
	bosondata.cpp
	bomath.cpp
	bo3dtools.cpp
	bosonprofiling.cpp
	bopluginmanager.cpp
	qlistviewitemnumber.cpp
)
kde3_automoc(${common_SRCS})
boson_add_library(common STATIC ${common_SRCS})
boson_target_link_libraries(common
	bodebug
	bogl
	bomath
	imageio
)


set(gameengine_SRCS
	gameengine/boeventloop.cpp
	gameengine/boupgradeableproperty.cpp
	gameengine/unitbase.cpp
	gameengine/unit.cpp
	gameengine/unitmover.cpp
	gameengine/unitorder.cpp
	gameengine/unitproperties.cpp
	gameengine/pluginproperties.cpp
	gameengine/unitplugins/unitplugin.cpp
	gameengine/unitplugins/ammunitionstorageplugin.cpp
	gameengine/unitplugins/bombingplugin.cpp
	gameengine/unitplugins/harvesterplugin.cpp
	gameengine/unitplugins/miningplugin.cpp
	gameengine/unitplugins/productionplugin.cpp
	gameengine/unitplugins/radarplugin.cpp
	gameengine/unitplugins/radarjammerplugin.cpp
	gameengine/unitplugins/refineryplugin.cpp
	gameengine/unitplugins/repairplugin.cpp
	gameengine/unitplugins/resourcemineplugin.cpp
	gameengine/upgradeproperties.cpp
	gameengine/bosonweapon.cpp
	gameengine/boitemlist.cpp
	gameengine/boitemlisthandler.cpp
	gameengine/bosonplayfield.cpp
	gameengine/bosonmap.cpp
	gameengine/cell.cpp
	gameengine/bofile.cpp
	gameengine/bpfdescription.cpp
	gameengine/bosoncampaign.cpp
	gameengine/bosonmessage.cpp
	gameengine/boson.cpp
	gameengine/bomessage.cpp
	gameengine/bosoncanvas.cpp
	gameengine/bosoncanvasstatistics.cpp
	gameengine/bosoncollisions.cpp
	gameengine/bosonnetworksynchronizer.cpp
	gameengine/bosonnetworktraffic.cpp
	gameengine/speciestheme.cpp
	gameengine/player.cpp
	gameengine/playerio.cpp
	gameengine/bosoncomputerio.cpp
	gameengine/bosonpath.cpp
	gameengine/bosongamestatistics.cpp
	gameengine/bosonstatistics.cpp
	gameengine/bogameproperty.cpp
	gameengine/bosonpropertyxml.cpp
	gameengine/bosonplayerinputhandler.cpp
	gameengine/bobincoder.cpp
	gameengine/boevent.cpp
	gameengine/boeventmatching.cpp
	gameengine/boeventlistener.cpp
	gameengine/boeventmanager.cpp
	gameengine/bocondition.cpp
	gameengine/bowater.cpp
	gameengine/bosonsaveload.cpp
	gameengine/bosongroundtheme.cpp
	gameengine/bosongameengine.cpp
	gameengine/bosonstarting.cpp
	gameengine/bosongameenginestarting.cpp
	gameengine/boquadtreenode.cpp
	gameengine/boquadtreecollection.cpp
	gameengine/bogroundquadtreenode.cpp
	gameengine/bocanvasquadtreenode.cpp
	gameengine/bosonitem.cpp
	gameengine/bosonshot.cpp
	gameengine/bosonitempropertyhandler.cpp
	gameengine/fileconverter/bosonplayfieldconverter.cpp
	gameengine/fileconverter/bosonfileconverter.cpp
	gameengine/fileconverter/bosavegameconverter.cpp
)
kde3_automoc(${gameengine_SRCS})
boson_add_library(gameengine STATIC ${gameengine_SRCS})
boson_target_link_libraries(gameengine
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
	startupwidgets/bosonloadsavegamehandler.cpp
	startupwidgets/boufoloadfromlogwidget.cpp
	startupwidgets/boufomappreview.cpp
)
boson_add_boui_files(startupwidgets_SRCS
	startupwidgets/welcomewidget.boui
	startupwidgets/newgamewidget.boui
	startupwidgets/boufonetworkoptionswidgetbase.boui
	startupwidgets/boufostarteditorwidgetbase.boui
	startupwidgets/boufoloadfromlogwidgetbase.boui
	startupwidgets/boufoloadingwidgetbase.boui
)
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
	gameview/bosonitemrenderer.cpp
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
	gameview/bosoneffectmanager.cpp
	gameview/bosoneffect.cpp
	gameview/bosoneffectproperties.cpp
	gameview/bosoneffectparticle.cpp
	gameview/bosoneffectpropertiesparticle.cpp
	gameview/boselection.cpp
	gameview/minimap/bosonglcompleteminimap.cpp
	gameview/minimap/bosonglminimapview.cpp
	gameview/minimap/bosonufominimap.cpp
	gameview/minimap/bosonufominimapdisplay.cpp
)
boson_add_boui_files(gameview_SRCS
	gameview/bosonufogameguihelper.boui
)
kde3_automoc(${gameview_SRCS})


set(modelrendering_SRCS
	modelrendering/bosonmodel.cpp
	modelrendering/bosonmodeltextures.cpp
	modelrendering/bomesh.cpp
	modelrendering/bomeshrenderer.cpp
	modelrendering/bomeshrenderermanager.cpp
	modelrendering/bobmfload.cpp
)
kde3_automoc(${modelrendering_SRCS})
boson_add_library(modelrendering STATIC ${modelrendering_SRCS})

# AB: this sucks. bosonmainlib should depend on modelrendering, not the other
#     way around.
#     -> but we need bomaterial.*, otherwise the commandframe tester won't link
#        (weird enough: boson does link successfully)
boson_target_link_libraries(modelrendering
	bosonmainlib
	${LIB_MESHRENDERER_PLUGIN}
)


# AB: bosonmainlib:
#     this contains pretty much everything of boson. except of those parts that
#     are clearly usable in the game only (e.g. startupwidgets, commandframe,
#     ...)
#     -> other binaries (borender, ...) are supposed to link against this, too
set(bosonmainlib_SRCS
	bosonviewdata.cpp
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
	bomaterial.cpp
	bolight.cpp
	botexture.cpp
	boshader.cpp
	speciesdata.cpp
	boaction.cpp
	bosongroundthemedata.cpp
	borendertarget.cpp
	bonuminput.cpp
	sound/bosonaudiointerface.cpp
	bosongldriverworkarounds.cpp
)
kde3_add_ui_files(bosonmainlib_SRCS
	bosonprofilingdialoggui.ui
)
kde3_automoc(${bosonmainlib_SRCS})
boson_add_library(bosonmainlib STATIC ${bosonmainlib_SRCS})

# TODO: if not plugins: meshrenderer, groundrenderer, gameview plugins
boson_target_link_libraries(bosonmainlib
	bosoninfo
	bosonsound
	bosonsoundcommon
	${LIB_GROUNDRENDERER_PLUGIN}
	${LIB_GAMEVIEW_PLUGIN}
	modelrendering
	gameengine
	common
	${QT_AND_KDECORE_KDEUI_KIO_LIBS}
	${LIB3DS_LIBRARY}

	${X11_XRANDR_LIB}
)



# AB: bosongamelib:
#     this contains bosonmainlib + everything that is required for the actual
#     boson binary - except for main.cpp which is added by the actual binary.
#     -> other binaries are not supposed to link against this. they may do so
#        however, e.g. to provide a test application (-> commandframe tester)
set(bosongamelib_SRCS
	bosonfpscounter.cpp
	bosonmainwidget.cpp
	bosonmainwidgetmenuinput.cpp
	boufoactionext.cpp
	bogltooltip.cpp
	optionsdialog/optionsdialog.cpp
	optionsdialog/optionswidgets.cpp
	optionsdialog/opengloptions.cpp
	optionsdialog/configoptionwidgets.cpp
	bogroundrenderer.cpp
	bogroundrenderermanager.cpp
	kgameunitdebug.cpp
	kgameadvancemessagesdebug.cpp
	kgameplayerdebug.cpp
	kgamecelldebug.cpp
	gameoverdialog.cpp
	bocheckinstallation.cpp
	bosonguistarting.cpp
	bosongameviewstarting.cpp
	bosongameviewpluginbase.cpp
	bosongameviewpluginmanager.cpp
	botexmapimportdialog.cpp
	boconditionwidget.cpp
	bowaterrenderer.cpp
	selectbox.cpp
	bosondebugtextures.cpp
	bosondebugmodels.cpp

	${startupwidgets_SRCS}
	${commandframe_SRCS}
	${gameview_SRCS}
)
kde3_add_ui_files(bosongamelib_SRCS
	bosondebugtexturesbase.ui
	bosondebugmodelsbase.ui
)
kde3_automoc( ${bosongamelib_SRCS} )
boson_add_library(bosongamelib STATIC ${bosongamelib_SRCS})
boson_target_link_libraries(bosongamelib
	bosonmainlib
	boufo
	${X11_XMU_LIB}
)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-boson.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-boson.h )

