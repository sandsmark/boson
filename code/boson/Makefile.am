## Makefile.am for boson

# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = data 
#boson editor

bin_PROGRAMS = boson boeditor

INCLUDES = $(all_includes)

# all boson(game)-only parts
BOSON_SRC = main.cpp top.cpp 
#optionsdialog.cpp # should be boson-only but... currently we need it for both.

# all map editor-only parts. 
BOEDITOR_SRC = editor.cpp bosonunitdialog.cpp

# all used-by-both files.
# actually editorinput.cpp should be in BOEDITOR_SRC but it is used by
# bosonwidget.cpp...
COMMON_SRC = unit.cpp unitproperties.cpp cell.cpp \
	kspritetooltip.cpp bosonmap.cpp bosoncanvas.cpp boson.cpp \
	player.cpp selectpart.cpp visualunit.cpp speciestheme.cpp \
	bosoncommandframe.cpp kgamedialogbosonconfig.cpp \
	bosonbigdisplay.cpp bosonminimap.cpp bosonunitview.cpp \
	kgameunitdebug.cpp bosonwidget.cpp \
	editorinput.cpp boshot.cpp bosontiles.cpp \
	bosonscenario.cpp bosonconfig.cpp \
	optionsdialog.cpp 

boson_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boson_LDADD = -lkdegames -lqtmcop -lsoundserver_idl $(LIB_KDEUI)
boson_SOURCES = $(BOSON_SRC) $(COMMON_SRC)

boeditor_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boeditor_LDADD = -lkdegames -lqtmcop -lsoundserver_idl $(LIB_KDEUI) $(LIB_KFILE)
boeditor_SOURCES = $(BOEDITOR_SRC) $(COMMON_SRC)

METASOURCES = AUTO
