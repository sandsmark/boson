INCLUDES = -I$(top_srcdir)/bodebug $(all_includes) -I$(top_srcdir)/bosound

noinst_LTLIBRARIES = libbosonsoundinterface.la

METASOURCES = AUTO

libbosonsoundinterface_la_SOURCES = \
	bosonaudiointerface.cpp
#	boaudioprocesscontroller.cpp

libbosonsoundinterface_la_LIBADD = \
		$(top_builddir)/bosound/libbosonsoundcommon.la \
		$(top_builddir)/bosound/libbosonsound.la

