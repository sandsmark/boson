include_directories(
	${CMAKE_SOURCE_DIR}/kgame
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_SOURCE_DIR}/bogl
	${CMAKE_SOURCE_DIR}/boufo
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

if(BOSON_USE_DEBUG_PLUGINS)
  kde4_add_plugin(bosongameviewplugin ${gameviewplugin_SRCS})
  install(TARGETS bosongameviewplugin DESTINATION ${BOSON_PLUGIN_INSTALL_DIR})
else(BOSON_USE_DEBUG_PLUGINS)
  boson_add_library(bosongameviewplugin STATIC ${gameviewplugin_SRCS})
endif(BOSON_USE_DEBUG_PLUGINS)


target_link_libraries(bosongameviewplugin
	boufo
	bogl
	kgame
	bosongamelib
	${KDE4_KDEUI_LIBS}
)


