INCLUDES = -I$(top_srcdir)/bodebug $(all_includes)

noinst_LTLIBRARIES = libbosonsoundinterface.la libbosonsound.la
bin_PROGRAMS = bosonaudioprocess

METASOURCES = AUTO

libbosonsoundinterface_la_SOURCES = \
	boaudiocommand.cpp \
	boaudioprocesscontroller.cpp \
	bosonaudiointerface.cpp

libbosonsound_la_SOURCES = \
	boaudiocommand.cpp \
	boaudiothread.cpp \
	boaudioprocesscontroller.cpp \
	bosonaudio.cpp \
	bosonaudiointerface.cpp \
	bosonsound.cpp \
	bosonmusic.cpp

bosonaudioprocess_LDFLAGS = $(all_libraries)
bosonaudioprocess_LDADD = \
		-lqtmcop -lsoundserver_idl \
		$(top_builddir)/boson/sound/libbosonsound.la \
		$(top_builddir)/boson/bodebug/libbodebug.la \
		$(LIB_QT) \
		-lartskde $(LIBKDECORE)

bosonaudioprocess_SOURCES = \
		main.cpp

