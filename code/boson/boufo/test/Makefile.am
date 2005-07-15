INCLUDES = $(UFO_INCLUDES) -I$(top_srcdir)/bodebug -I$(top_srcdir)/bogl $(all_includes) -I$(top_srcdir)/boson -I$(top_srcdir)/boson/boufo

bin_PROGRAMS = boufotest boufobuttontest

METASOURCES = AUTO

KDE_CXXFLAGS = $(UFO_CFLAGS)

boufotest_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boufotest_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/boson/boufo/libboufo.la \
		$(LIB_QT) \
		$(LIB_KDECORE) \
		$(LIB_BOMEMORY)
boufotest_SOURCES = \
		boufotestmain.cpp \
		boufofontselectionwidgetbase.boui \
		boufofontselectionwidget.cpp

boufobuttontest_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boufobuttontest_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/boson/boufo/libboufo.la \
		$(LIB_QT) \
		$(LIB_KDECORE) \
		$(LIB_BOMEMORY)
boufobuttontest_SOURCES = \
		boufobuttontest.cpp


