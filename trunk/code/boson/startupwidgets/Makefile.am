INCLUDES = -I$(top_srcdir)/bodebug -I$(top_srcdir)/bogl -I$(top_srcdir)/boson -I$(top_builddir)/boson/bosonwidgets -I$(top_srcdir)/boson/boufo $(all_includes)

noinst_LTLIBRARIES = libbosonstartupwidgets.la

# AB: rename! use bosonneweditorwidgetbase.ui
libbosonstartupwidgets_la_SOURCES = \
	bosonneweditorwidgetbase.ui \
	bosonstartupnetwork.cpp \
	bosonnetworkoptionswidget.cpp \
	bosonneweditorwidget.cpp \
	kloadsavegamewidget.cpp \
	welcomewidget.boui \
	newgamewidget.boui \
	boufonewgamewidget.cpp \
	boufostartupwidget.cpp \
	boufoloadingwidget.cpp

METASOURCES = AUTO

# TODO: we have no messages target here
