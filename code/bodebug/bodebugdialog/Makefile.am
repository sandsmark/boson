bin_PROGRAMS = bodebugdialog

INCLUDES = $(all_includes)

METASOURCES = AUTO

bodebugdialog_SOURCES = boabstractdebugdialog.cpp \
		bodebugdialog.cpp \
		bolistdebugdialog.cpp \
		main.cpp
bodebugdialog_LDFLAGS = $(all_libraries) $(KDE_RPATH)
bodebugdialog_LDADD = \
		$(LIB_KDEUI) \
		$(LIB_BOMEMORY) \
		$(top_builddir)/bodebug/libbodebug.la

