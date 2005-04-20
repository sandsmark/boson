INCLUDES = -I$(top_srcdir)/bodebug -I$(top_srcdir)/bogl -I$(top_srcdir)/boson -I$(top_builddir)/boson/bosonwidgets $(all_includes)

noinst_LTLIBRARIES = libbosonstartupwidgets.la

# AB: rename! use bosonneweditorwidgetbase.ui
libbosonstartupwidgets_la_SOURCES = \
	bosonnewgamewidgetbase.ui \
	bosonneweditorwidgetbase.ui \
	bosonstartupnetwork.cpp \
	bosonstartupwidget.cpp \
	bosonwelcomewidget.cpp\
	bosonnetworkoptionswidget.cpp \
	bosonloadingwidget.cpp \
	bosonnewgamewidget.cpp \
	bosonneweditorwidget.cpp \
	kloadsavegamewidget.cpp \
	welcomewidget.boui \
	newgamewidget.boui \
	boufonewgamewidget.cpp

METASOURCES = AUTO

# TODO: we have no messages target here
