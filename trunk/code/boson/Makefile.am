## Makefile.am for boson

# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = data items commandframe sound startupwidgets bodebug

bin_PROGRAMS = boson bounit boprofiling bocursor borender

INCLUDES = -I$(kde_includes)/arts -I$(top_srcdir)/boson/bodebug $(all_includes)

BOSON_SRC = \
	top.cpp  \
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
	kgamedialogbosonconfig.cpp \
	bo3dtools.cpp \
	bosonbigdisplaybase.cpp \
	bosonbigdisplay.cpp \
	bosonminimap.cpp \
	kgameunitdebug.cpp \
	kgameplayerdebug.cpp \
	kgamecelldebug.cpp \
	bodisplaymanager.cpp \
	bosonwidgetbase.cpp \
	bosonwidget.cpp \
	editorwidget.cpp \
	bosontiles.cpp \
	bosonconfig.cpp \
	optionsdialog.cpp \
	optionswidgets.cpp \
	bosoncomputerio.cpp \
	bosonpath.cpp \
	unit.cpp \
	selectbox.cpp \
	commandinput.cpp \
	bosoncursor.cpp \
	bosonstatistics.cpp \
	gameoverdialog.cpp \
	unitplugins.cpp \
	editorbigdisplay.cpp \
	bosontexturearray.cpp \
	bosonmodel.cpp \
	bosonglfont.cpp \
	bosonglchat.cpp \
	bosonprofiling.cpp \
	bosonprofilingdialog.cpp \
	bosonmodeltextures.cpp \
	upgradeproperties.cpp \
	bosonparticlesystem.cpp \
	bosonparticlemanager.cpp \
	bosonstarting.cpp \
	bosonweapon.cpp

BOPROFILING_SRC= \
	bosonprofilingdialog.cpp \
	bosonprofiling.cpp \
	boprofilingmain.cpp

boson_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boson_LDADD = \
		$(top_builddir)/boson/items/libbosonitems.la \
		$(top_builddir)/boson/sound/libbosonsound.la \
		$(top_builddir)/boson/commandframe/libbosoncommandframe.la \
		$(top_builddir)/boson/startupwidgets/libbosonstartupwidgets.la \
		$(top_builddir)/boson/bodebug/libbodebug.la \
		-lkdegames -lqtmcop -lsoundserver_idl -lartskde \
		-lGLU -l3ds \
		$(LIB_KDEUI)
boson_SOURCES = \
		$(BOSON_SRC)\
		main.cpp

bounit_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bounit_LDADD = \
		$(top_builddir)/boson/items/libbosonitems.la \
		$(top_builddir)/boson/sound/libbosonsound.la \
		$(top_builddir)/boson/commandframe/libbosoncommandframe.la \
		$(top_builddir)/boson/startupwidgets/libbosonstartupwidgets.la \
		$(top_builddir)/boson/bodebug/libbodebug.la \
		-lkdegames -lqtmcop -lsoundserver_idl -lartskde \
		-lGLU -l3ds \
		-lkio $(LIB_KDEUI)
bounit_SOURCES = \
		$(BOSON_SRC) \
		bounitmain.cpp bosonsearchpathswidget.ui bouniteditor.ui

boprofiling_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boprofiling_LDADD = \
		$(top_builddir)/boson/bodebug/libbodebug.la \
		-lkio $(LIB_KDEUI)
boprofiling_SOURCES = \
		$(BOPROFILING_SRC)

bocursor_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bocursor_LDADD = \
		$(top_builddir)/boson/bodebug/libbodebug.la \
		$(top_builddir)/boson/items/libbosonitems.la \
		$(top_builddir)/boson/sound/libbosonsound.la \
		$(top_builddir)/boson/commandframe/libbosoncommandframe.la \
		$(top_builddir)/boson/startupwidgets/libbosonstartupwidgets.la \
		-lkdegames -lqtmcop -lsoundserver_idl -lartskde \
		-lGLU -l3ds 
bocursor_SOURCES = \
		$(BOSON_SRC) \
		bocursormain.cpp bosoncursoreditor.cpp

borender_LDFLAGS = $(KDE_RPATH) $(all_libraries)
borender_LDADD = \
		$(top_builddir)/boson/bodebug/libbodebug.la \
		$(top_builddir)/boson/items/libbosonitems.la \
		$(top_builddir)/boson/sound/libbosonsound.la \
		$(top_builddir)/boson/commandframe/libbosoncommandframe.la \
		$(top_builddir)/boson/startupwidgets/libbosonstartupwidgets.la \
		-lkdegames -lqtmcop -lsoundserver_idl -lartskde \
		-lGLU -l3ds \
		$(LIB_KDEUI)
borender_SOURCES = \
		$(BOSON_SRC) \
		kgamemodeldebug.cpp \
		borendermain.cpp

METASOURCES = AUTO

# not yet used:
#messages: rc.cpp
#	$(XGETTEXT) rc.cpp $(COMMON_SRC) $(BOSON_SRC) $(BOEDITOR_SRC) -o $(podir)/boson.pot
