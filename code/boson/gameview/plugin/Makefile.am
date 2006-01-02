INCLUDES = -I$(top_srcdir)/bodebug -I$(top_srcdir)/bogl $(all_includes)

if include_plugins
GAMEVIEW_PLUGIN = libbosongameviewplugin.la
else
GAMEVIEW_LIB = libbosongameviewext.la
endif

noinst_LTLIBRARIES = $(GAMEVIEW_LIB)
boson_plugin_LTLIBRARIES = $(GAMEVIEW_PLUGIN)

libbosongameviewplugin_la_SOURCES = \
		bosongameviewpluginfactory.cpp \
		bosongameviewplugindefault.cpp \
		boselectiondebugwidget.cpp \
		editorrandommapwidget.cpp \
		bodebugconfigswitches.cpp \
		bonetworktrafficwidget.cpp

libbosongameviewext_la_SOURCES = \
		bosongameviewpluginfactory.cpp \
		bosongameviewplugindefault.cpp \
		boselectiondebugwidget.cpp \
		editorrandommapwidget.cpp \
		bodebugconfigswitches.cpp \
		bonetworktrafficwidget.cpp

pluginsdir = $(kde_datadir)/boson/gameviewplugins
plugins_DATA =

METASOURCES = AUTO

