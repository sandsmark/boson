INCLUDES = -I$(top_srcdir)/bodebug $(all_includes)

SUBDIRS = openal

noinst_LTLIBRARIES = libbosonsoundcommon.la libbosonsound.la
#bin_PROGRAMS = bosonaudioprocess

METASOURCES = AUTO

libbosonsoundcommon_la_SOURCES = \
	boaudiocommand.cpp

libbosonsound_la_SOURCES = \
	boaudiocommand.cpp \
	boaudiothread.cpp \
	bosonaudio.cpp
libbosonsound_la_LIBADD = \
		$(top_builddir)/bosound/libbosonsoundcommon.la \
		$(top_builddir)/bosound/openal/libbosonsoundal.la \
		$(LIB_OPENAL)

#bosonaudioprocess_LDFLAGS = $(all_libraries)
#bosonaudioprocess_LDADD = \
#		$(top_builddir)/bosound/libbosonsound.la \
#		$(top_builddir)/bodebug/libbodebug.la \
#		$(LIB_QT) \
#		$(LIB_KDECORE) $(LIB_KIO)
#
#bosonaudioprocess_SOURCES = \
#		main.cpp

include ../admin/Doxyfile.am

