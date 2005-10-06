INCLUDES = $(UFO_INCLUDES) -I$(top_srcdir)/bodebug -I$(top_srcdir)/bogl $(all_includes) -I$(top_srcdir)/boson -I$(top_srcdir)/boson/boufo

bin_PROGRAMS = boufodesigner

METASOURCES = AUTO

KDE_CXXFLAGS = $(UFO_CFLAGS)

boufodesigner_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boufodesigner_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/boson/boufo/libboufo.la \
		$(LIB_QT) \
		$(LIB_KDECORE) \
		$(LIB_BOMEMORY)
boufodesigner_SOURCES = \
		boufodesignermain.cpp \
		bosignalsslotseditor.cpp \
		main.cpp



