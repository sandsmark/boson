SUBDIRS = bouic .

INCLUDES = $(UFO_INCLUDES) -I$(top_srcdir)/bodebug -I$(top_srcdir)/bogl $(all_includes) -I$(top_srcdir)/boson

noinst_LTLIBRARIES = libboufo.la
bin_PROGRAMS = boufodesigner boufotest

METASOURCES = AUTO

KDE_CXXFLAGS = $(UFO_CFLAGS)

libboufo_la_SOURCES = \
	boufo.cpp \
	boufoaction.cpp \
	boufofontrenderer.cpp \
	ubolabelui.cpp \
	uboprogress.cpp \
	uboprogressui.cpp
#	ubotable.cpp 
#	ubotableui.cpp
#	uboboxlayout.cpp 

libboufo_la_LIBADD = \
		$(top_builddir)/boson/bosonfont/libbosonfont.la \
		$(LIB_UFO)

boufodesigner_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boufodesigner_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/boson/boufo/libboufo.la \
		$(LIB_QT) \
		$(LIB_KDECORE)
boufodesigner_SOURCES = \
		boufodesignermain.cpp


boufotest_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boufotest_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/boson/boufo/libboufo.la \
		$(LIB_QT) \
		$(LIB_KDECORE)
boufotest_SOURCES = \
		boufotestmain.cpp \
		boufofontselectionwidgetbase.boui \
		boufofontselectionwidget.cpp

