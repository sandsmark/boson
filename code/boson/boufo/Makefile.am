SUBDIRS = bouic .

INCLUDES = $(UFO_INCLUDES) -I$(top_srcdir)/bodebug $(all_includes)

noinst_LTLIBRARIES = libboufo.la
bin_PROGRAMS = boufodesigner

METASOURCES = AUTO

KDE_CXXFLAGS = $(UFO_CFLAGS)

libboufo_la_SOURCES = \
	boufo.cpp \
	boufoaction.cpp \
	ubolabelui.cpp \
	uboprogress.cpp \
	uboprogressui.cpp

libboufo_la_LIBADD = \
	$(LIB_UFO)

boufodesigner_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boufodesigner_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/boufo/libboufo.la \
		$(LIB_QT) \
		$(LIB_KDECORE) \
		$(GLLIB)
boufodesigner_SOURCES = \
		boufodesignermain.cpp

