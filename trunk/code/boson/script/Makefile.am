INCLUDES = -I$(top_srcdir)/boson/script -I$(top_srcdir)/bodebug $(all_includes) $(PYTHONINC)

noinst_LTLIBRARIES = libbosonscript.la

libbosonscript_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) $(PYTHONLIB)
libbosonscript_la_LIBADD = $(LIBPYTHON)
libbosonscript_la_SOURCES = bosonscript.cpp pythonscript.cpp bosonscriptinterface.cpp

METASOURCES = AUTO

scriptsdir = $(kde_datadir)/boson/scripts
scripts_DATA = boson-script.py localplayer-script.py utils.py conditions1.py
