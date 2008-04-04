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
kde3_automoc(${meshrenderer_SRCS})
if(BOSON_USE_DEBUG_PLUGINS)
  boson_add_library(bomeshrendererplugin MODULE ${meshrenderer_SRCS})
  install_targets(${BOSON_PLUGIN_INSTALL_DIR} bomeshrendererplugin)
else(BOSON_USE_DEBUG_PLUGINS)
  boson_add_library(bomeshrendererplugin STATIC ${meshrenderer_SRCS})
endif(BOSON_USE_DEBUG_PLUGINS)

