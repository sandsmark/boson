include_directories(
	${CMAKE_SOURCE_DIR}/kgame
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_SOURCE_DIR}/bogl
	${CMAKE_CURRENT_SOURCE_DIR}/gameengine
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(groundrenderer_SRCS
	bogroundrendererfactory.cpp
	bodefaultgroundrenderer.cpp
	bofastgroundrenderer.cpp
	boveryfastgroundrenderer.cpp
	boquickgroundrenderer.cpp
	bogroundrendererbase.cpp
	bocolormaprenderer.cpp
)
kde3_automoc(${groundrenderer_SRCS})
if(BOSON_USE_DEBUG_PLUGINS)
  boson_add_library(bogroundrendererplugin MODULE ${groundrenderer_SRCS})
  install_targets(${BOSON_PLUGIN_INSTALL_DIR} bogroundrendererplugin)
else(BOSON_USE_DEBUG_PLUGINS)
  boson_add_library(bogroundrendererplugin STATIC ${groundrenderer_SRCS})
endif(BOSON_USE_DEBUG_PLUGINS)


