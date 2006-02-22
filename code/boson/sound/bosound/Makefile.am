INCLUDES = -I$(top_srcdir)/bodebug $(all_includes)

if include_bosound_openal
OPENAL_SUBDIR=openal
endif

SUBDIRS = $(OPENAL_SUBDIR)

noinst_LTLIBRARIES = libbosonsoundcommon.la libbosonsound.la
#bin_PROGRAMS = bosonaudioprocess

METASOURCES = AUTO

libbosonsoundcommon_la_SOURCES = \
	boaudiocommand.cpp

if link_static
else
LIB_OPENAL_LIB=$(LIB_OPENAL)
endif
libbosonsound_la_SOURCES = \
	boaudiocommand.cpp \
	boaudiothread.cpp \
	bosonaudio.cpp
libbosonsound_la_LIBADD = \
		$(top_builddir)/bosound/libbosonsoundcommon.la \
		$(BO_LIB_OPENAL) \
		$(LIB_OPENAL_LIB)

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

