## Makefile.am for boson

# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = data visual sound

bin_PROGRAMS = boson bounit boprofiling

INCLUDES = -I$(kde_includes)/arts $(all_includes)

BOSON_SRC = \
	main.cpp \
	top.cpp  \
	bosonwelcomewidget.cpp \
	bosonnewgamewidget.cpp \
	bosonserveroptionswidget.cpp \
	bosonloadingwidget.cpp \
	cell.cpp \
	unitproperties.cpp \
	pluginproperties.cpp \
	unitbase.cpp \
	boitemlist.cpp \
	boselection.cpp \
	kspritetooltip.cpp \
	bosonmap.cpp \
	bosonscenario.cpp \
	bosonplayfield.cpp \
	bosoncanvas.cpp \
	boson.cpp \
	speciestheme.cpp \
	player.cpp \
	bosoncommandframe.cpp \
	kgamedialogbosonconfig.cpp \
	bosonbigdisplaybase.cpp \
	bosonbigdisplay.cpp \
	bosonminimap.cpp \
	bosonunitview.cpp \
	kgameunitdebug.cpp \
	kgameplayerdebug.cpp \
	kgamecelldebug.cpp \
	bodisplaymanager.cpp \
	bosonwidgetbase.cpp \
	bosonwidget.cpp \
	editorwidget.cpp \
	boshot.cpp \
	bosontiles.cpp \
	bosonconfig.cpp \
	optionsdialog.cpp \
	bosoncomputerio.cpp \
	kgamedialogcomputerconfig.cpp \
	bosonpath.cpp \
	unit.cpp \
	bosoncommandwidget.cpp \
	selectbox.cpp \
	commandinput.cpp \
	bosoncursor.cpp \
	bosonstatistics.cpp \
	gameoverdialog.cpp \
	unitplugins.cpp \
	bosonstarteditorwidget.cpp \
	bosonstartupbasewidget.cpp \
	editorbigdisplay.cpp \
	bosontexturearray.cpp \
	bosonmodel.cpp \
	bosonglfont.cpp \
	bosonprofiling.cpp \
	bosonprofilingdialog.cpp \
	bosonmodeltextures.cpp

BOUNIT_SRC = \
	unitproperties.cpp \
	pluginproperties.cpp \
	bosonunitdialog.cpp \
	bounitmain.cpp

BOPROFILING_SRC= \
	bosonprofilingdialog.cpp \
	bosonprofiling.cpp \
	boprofilingmain.cpp

boson_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boson_LDADD = $(top_builddir)/boson/visual/libbosonvisual.la $(top_builddir)/boson/sound/libbosonsound.la -lkdegames -lqtmcop -lsoundserver_idl -lartskde -lGLU -l3ds $(LIB_KDEUI)
boson_SOURCES = $(BOSON_SRC)

bounit_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bounit_LDADD = -lkio $(LIB_KDEUI)
bounit_SOURCES = $(BOUNIT_SRC)

boprofiling_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boprofiling_LDADD = -lkio $(LIB_KDEUI)
boprofiling_SOURCES = $(BOPROFILING_SRC)

METASOURCES = AUTO

# not yet used:
#messages: rc.cpp
#	$(XGETTEXT) rc.cpp $(COMMON_SRC) $(BOSON_SRC) $(BOEDITOR_SRC) -o $(podir)/boson.pot
