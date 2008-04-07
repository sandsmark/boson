include_directories(
	${CMAKE_SOURCE_DIR}/kgame
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_SOURCE_DIR}/bogl
	${CMAKE_CURRENT_SOURCE_DIR}/gameengine
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(meshrenderer_SRCS
	bomeshrendererfactory.cpp
	bomeshrendererimmediate.cpp
	bomeshrenderersemiimmediate.cpp
	bomeshrenderervertexarray.cpp
	bomeshrenderervbo.cpp
)
if(BOSON_USE_DEBUG_PLUGINS)
  kde4_add_plugin(bomeshrendererplugin ${meshrenderer_SRCS})
  install(TARGETS bomeshrendererplugin DESTINATION ${BOSON_PLUGIN_INSTALL_DIR})
else(BOSON_USE_DEBUG_PLUGINS)
  boson_add_library(bomeshrendererplugin STATIC ${meshrenderer_SRCS})
endif(BOSON_USE_DEBUG_PLUGINS)


# AB: FIXME: KDE4 port: do we really need to link the plugin against
# _everything_?
target_link_libraries(bomeshrendererplugin
	bogl
	bosonmainlib
	${KDE4_KDEUI_LIBS}
)

