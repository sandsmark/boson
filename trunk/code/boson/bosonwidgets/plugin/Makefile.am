
AM_CXXFLAGS = -DQT_PLUGIN

INCLUDES = -I$(top_builddir)/boson/bosonwidgets $(all_includes)
kde_widget_LTLIBRARIES = bosonwidgets.la

METASOURCES = AUTO

bosonwidgets_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
bosonwidgets_la_LIBADD = \
	$(top_builddir)/boson/bosonwidgets/libbosonwidgets.la \
	$(LIB_BOMEMORY) \
	$(LIB_KGAME) \
	$(LIB_KDEUI) \
	$(LIB_KDECORE)
bosonwidgets_la_SOURCES = bosonwidgets.cpp dummyclasses.cpp

