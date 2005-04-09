INCLUDES = -I$(top_srcdir)/bodebug -I$(top_srcdir)/bogl $(all_includes)

noinst_LTLIBRARIES = libbosoninfo.la

METASOURCES = AUTO

libbosoninfo_la_SOURCES = \
	boinfo.cpp \
	boinfo_x11.cpp \
	boinfodialog.cpp \
	boglquerystates.cpp

libbosoninfo_la_LIBADD = \
	$(top_builddir)/bogl/libbogl.la
