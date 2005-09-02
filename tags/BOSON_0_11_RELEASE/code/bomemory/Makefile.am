INCLUDES = -I$(top_srcdir)/bodebug $(all_includes)

if include_bomemory
BOMEMORYLIB = libbomemory.la
endif

noinst_LTLIBRARIES = $(BOMEMORYLIB)

METASOURCES = AUTO

libbomemory_la_SOURCES = \
	new.cpp \
	memnode.cpp \
	memorymanager.cpp \
	bomemorydialog.cpp \
	malloc.cpp

