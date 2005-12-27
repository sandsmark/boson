INCLUDES = -I$(top_srcdir)/bodebug -I$(top_srcdir)/bogl $(all_includes)

if include_plugins
GROUNDRENDERER_PLUGIN=libbogroundrendererplugin.la
else
GROUNDRENDERER_LIB=libbogroundrenderer.la
endif

noinst_LTLIBRARIES = $(GROUNDRENDERER_LIB)
boson_plugin_LTLIBRARIES = $(GROUNDRENDERER_PLUGIN)

libbogroundrendererplugin_la_SOURCES = \
		bogroundrendererfactory.cpp \
		bodefaultgroundrenderer.cpp \
		bofastgroundrenderer.cpp \
		boveryfastgroundrenderer.cpp \
		boquickgroundrenderer.cpp \
		bogroundrendererbase.cpp \
		bocolormaprenderer.cpp

libbogroundrenderer_la_SOURCES = \
		bogroundrendererfactory.cpp \
		bodefaultgroundrenderer.cpp \
		bofastgroundrenderer.cpp \
		boveryfastgroundrenderer.cpp \
		boquickgroundrenderer.cpp \
		bogroundrendererbase.cpp \
		bocolormaprenderer.cpp

pluginsdir = $(kde_datadir)/boson/groundrendererplugins
plugins_DATA =

METASOURCES = AUTO

