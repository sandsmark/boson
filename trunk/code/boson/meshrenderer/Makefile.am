INCLUDES = -I$(top_srcdir)/bodebug $(all_includes)

boson_plugin_LTLIBRARIES = libbomeshrendererplugin.la

libbomeshrendererplugin_la_SOURCES = \
		bomeshrendererfactory.cpp \
		bomeshrendererimmediate.cpp \
		bomeshrenderersemiimmediate.cpp \
		bomeshrenderervertexarray.cpp \
		bomeshrenderervbo.cpp

# libbomeshrendererplugin_la_LIBADD = $(LIB_KDEUI) $(LIB_KPARTS) $(LIB_KHTML)
# libbomeshrendererplugin_la_LDFLAGS = $(KDE_PLUGIN)

pluginsdir = $(kde_datadir)/boson/meshrendererplugins
plugins_DATA =

METASOURCES = AUTO

