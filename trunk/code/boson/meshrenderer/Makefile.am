INCLUDES = -I$(top_srcdir)/bodebug $(all_includes)

if include_plugins
MESHRENDERER_PLUGIN=libbomeshrendererplugin.la
else
MESHRENDERER_LIB=libbomeshrenderer.la
endif

noinst_LTLIBRARIES = $(MESHRENDERER_LIB)
boson_plugin_LTLIBRARIES = $(MESHRENDERER_PLUGIN)

libbomeshrendererplugin_la_SOURCES = \
		bomeshrendererfactory.cpp \
		bomeshrendererimmediate.cpp \
		bomeshrenderersemiimmediate.cpp \
		bomeshrenderervertexarray.cpp \
		bomeshrenderervbo.cpp

libbomeshrenderer_la_SOURCES = \
		bomeshrendererfactory.cpp \
		bomeshrendererimmediate.cpp \
		bomeshrenderersemiimmediate.cpp \
		bomeshrenderervertexarray.cpp \
		bomeshrenderervbo.cpp

pluginsdir = $(kde_datadir)/boson/meshrendererplugins
plugins_DATA =

METASOURCES = AUTO

