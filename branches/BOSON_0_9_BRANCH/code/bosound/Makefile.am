INCLUDES = -I$(top_srcdir)/bodebug $(all_includes)

noinst_LTLIBRARIES = libbosonsoundcommon.la libbosonsound.la
bin_PROGRAMS = bosonaudioprocess

METASOURCES = AUTO

libbosonsoundcommon_la_SOURCES = \
	boaudiocommand.cpp

libbosonsound_la_SOURCES = \
	boaudiocommand.cpp \
	boaudiothread.cpp \
	bosonaudio.cpp \
	bosonsound.cpp \
	bosonmusic.cpp
libbosonsound_la_LIBADD = \
		$(top_builddir)/bosound/libbosonsoundcommon.la

bosonaudioprocess_LDFLAGS = $(all_libraries)
bosonaudioprocess_LDADD = \
		-lqtmcop -lsoundserver_idl \
		$(top_builddir)/bosound/libbosonsound.la \
		$(top_builddir)/bodebug/libbodebug.la \
		$(LIB_QT) \
		-lartskde $(LIBKDECORE)

bosonaudioprocess_SOURCES = \
		main.cpp

include ../admin/Doxyfile.am

