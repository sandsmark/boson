include_directories(
	${CMAKE_SOURCE_DIR}/kgame
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_SOURCE_DIR}/bogl
	${CMAKE_CURRENT_SOURCE_DIR}/gameengine
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(gameviewplugin_SRCS
	bosongameviewpluginfactory.cpp
	bosongameviewplugindefault.cpp
	boselectiondebugwidget.cpp
	editorrandommapwidget.cpp
	bodebugconfigswitches.cpp
	bonetworktrafficwidget.cpp
)

kde3_automoc(${gameviewplugin_SRCS})
if(BOSON_USE_DEBUG_PLUGINS)
  add_library(bosongameviewplugin MODULE ${gameviewplugin_SRCS})
  set(LIB_GAMEVIEW_PLUGIN "")
  install_targets(${BOSON_PLUGIN_INSTALL_DIR} bosongameviewplugin)
else(BOSON_USE_DEBUG_PLUGINS)
  add_library(bosongameviewplugin STATIC ${gameviewplugin_SRCS})
  set(LIB_GAMEVIEW_PLUGIN bosongameviewplugin)
endif(BOSON_USE_DEBUG_PLUGINS)


