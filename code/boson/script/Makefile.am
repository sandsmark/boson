INCLUDES = -I$(top_srcdir)/boson/script -I$(top_srcdir)/boson/bodebug $(all_includes) $(PYTHONINC)

noinst_LTLIBRARIES = libbosonscript.la

libbosonscript_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) $(PYTHONLIB)
libbosonscript_la_LIBADD = -lkdegames -lqtmcop -lsoundserver_idl -lartskde -lGLU -l3ds $(LIB_KDEUI) $(LIBPYTHON)
libbosonscript_la_SOURCES = bosonscript.cpp pythonscript.cpp

METASOURCES = AUTO

scriptsdir = $(kde_datadir)/boson/scripts
scripts_DATA = boson-script.py
