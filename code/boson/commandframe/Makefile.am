INCLUDES = -I$(top_srcdir)/boson/bodebug $(all_includes)

noinst_LTLIBRARIES = libbosoncommandframe.la


COMMON_SRC = bosoncommandframebase.cpp \
		bosonorderwidget.cpp \
		bosonorderbutton.cpp \
		bosonunitview.cpp
EDITOR_SRC = editorcommandframe.cpp
GAME_SRC = bosoncommandframe.cpp \
		boactionswidget.cpp

libbosoncommandframe_la_SOURCES = \
		$(COMMON_SRC) \
		$(GAME_SRC) \
		$(EDITOR_SRC)

METASOURCES = AUTO

# AB: we will need a message target here?!
