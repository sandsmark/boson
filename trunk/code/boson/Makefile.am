## Makefile.am for boson

# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = data 
#boson editor

bin_PROGRAMS = boson boeditor bounit

INCLUDES = -I$(kde_includes)/arts $(all_includes)

# all boson(game)-only parts
BOSON_SRC = main.cpp \
	top.cpp  \
	bosonwelcomewidget.cpp \
	bosonnewgamewidget.cpp \
	bosonserveroptionswidget.cpp \
	bosonloadingwidget.cpp
#optionsdialog.cpp # should be boson-only but... currently we need it for both.

# all map editor-only parts. 
BOEDITOR_SRC = editor.cpp \
	bosonunitdialog.cpp \
	editorinput.cpp \
	topbase.cpp

# all used-by-both files.
# actually editorinput.cpp should be in BOEDITOR_SRC but it is used by
# bosonwidget.cpp...
COMMON_SRC = \
	cell.cpp \
	unitproperties.cpp \
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
	bosonbigdisplay.cpp \
	bosonminimap.cpp \
	bosonunitview.cpp \
	kgameunitdebug.cpp \
	bodisplaymanager.cpp \
	bosonwidget.cpp \
	boshot.cpp \
	bosontiles.cpp \
	bosonconfig.cpp \
	optionsdialog.cpp \
	bosoncomputerio.cpp \
	kgamedialogcomputerconfig.cpp \
	bosonpath.cpp \
	unit.cpp \
	bosoncommandwidget.cpp \
	bosonmusic.cpp \
	selectbox.cpp \
	commandinput.cpp \
	kgamecanvaschat.cpp \
	bosoncursor.cpp \
	bosonsound.cpp \
	unitgroup.cpp \
	bosonstatistics.cpp \
	gameoverdialog.cpp \
	unitplugins.cpp

BOUNIT_SRC = \
	unitproperties.cpp \
	bosonunitdialog.cpp \
	bounitmain.cpp

boson_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boson_LDADD = -lkdegames -lqtmcop -lsoundserver_idl -lartskde $(LIB_KDEUI)
boson_SOURCES = $(BOSON_SRC) $(COMMON_SRC)

boeditor_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boeditor_LDADD = -lkdegames -lqtmcop -lsoundserver_idl -lartskde $(LIB_KDEUI) 
boeditor_SOURCES = $(BOEDITOR_SRC) $(COMMON_SRC)

bounit_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bounit_LDADD = -lkio $(LIB_KDEUI) 
bounit_SOURCES = $(BOUNIT_SRC) 

METASOURCES = AUTO

# not yet used:
#messages: rc.cpp
#	$(XGETTEXT) rc.cpp $(COMMON_SRC) $(BOSON_SRC) $(BOEDITOR_SRC) -o $(podir)/boson.pot
