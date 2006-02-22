INCLUDES = -I$(top_srcdir)/boson/gameengine/script -I$(top_srcdir)/boson/gameengine -I$(top_srcdir)/bodebug -I$(top_srcdir)/bogl $(all_includes) $(PYTHONINC)

noinst_LTLIBRARIES = libbosonscript.la

libbosonscript_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) $(PYTHONLIB)
libbosonscript_la_LIBADD = $(LIBPYTHON)
libbosonscript_la_SOURCES = bosonscript.cpp pythonscript.cpp bosonscriptinterface.cpp

METASOURCES = AUTO
