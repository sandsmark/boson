## Makefile.am for boson

SUBDIRS = data \
	bosonwidgets items commandframe startupwidgets sound info script \
	bosonfont \
	meshrenderer \
	groundrenderer

bin_PROGRAMS = boson bounit boprofiling bocursor borender boinfo bo3dsdebug

INCLUDES = -I$(kde_includes)/arts -I$(top_srcdir)/bodebug $(all_includes)

BOSON_COMMON_SRC = \
	boapplication.cpp \
	boglobal.cpp \
	cell.cpp \
	unitproperties.cpp \
	pluginproperties.cpp \
	unitbase.cpp \
	boitemlisthandler.cpp \
	boitemlist.cpp \
	boselection.cpp \
	botooltipcreator.cpp \
	bogltooltip.cpp \
	bosonmap.cpp \
	bosonfileconverter.cpp \
	bosoncampaign.cpp \
	bosonplayfield.cpp \
	bosoncanvas.cpp \
	bosoncanvasstatistics.cpp \
	bosoncollisions.cpp \
	boson.cpp \
	bosonsaveload.cpp \
	speciestheme.cpp \
	speciesdata.cpp \
	player.cpp \
	playerio.cpp \
	bo3dtools.cpp \
	bomatrixwidget.cpp \
	bosonglminimap.cpp \
	kgameunitdebug.cpp \
	kgameplayerdebug.cpp \
	kgamecelldebug.cpp \
	bodisplaymanager.cpp \
	bosonwidgetbase.cpp \
	bosonwidget.cpp \
	editorwidget.cpp \
	bosonglwidget.cpp \
	bosonbigdisplaybase.cpp \
	bogroundrenderer.cpp \
	bogroundrenderermanager.cpp \
	bocamera.cpp \
	boautocamera.cpp \
	bocamerawidget.cpp \
	boorbiterwidget.cpp \
	bosongroundtheme.cpp \
	bosonconfig.cpp \
	optionsdialog.cpp \
	optionswidgets.cpp \
	bosoncomputerio.cpp \
	bosonpath.cpp \
	unit.cpp \
	selectbox.cpp \
	bosoncursor.cpp \
	bosonstatistics.cpp \
	gameoverdialog.cpp \
	unitplugins.cpp \
	bosontexturearray.cpp \
	bosonmodel.cpp \
	bosonmodelloader.cpp \
	bo3dsload.cpp \
	bosonglchat.cpp \
	bosonprofiling.cpp \
	bosonprofilingdialog.cpp \
	bosonmodeltextures.cpp \
	upgradeproperties.cpp \
	bosonparticlesystem.cpp \
	bosonparticlesystemproperties.cpp \
	bosonstarting.cpp \
	bofile.cpp \
	bpfdescription.cpp \
	bosonweapon.cpp \
	bpfdescriptiondialog.cpp \
	bosonbigdisplayinputbase.cpp \
	bosonbigdisplayinput.cpp \
	editorbigdisplayinput.cpp \
	bomesh.cpp \
	bomeshrenderer.cpp \
	bomeshrenderermanager.cpp \
	bolodbuilder.cpp \
	bomaterial.cpp \
	bomaterialwidget.cpp \
	bovectorinput.cpp \
	bosonpropertyxml.cpp \
	bomemorytrace.cpp \
	boaction.cpp \
	bosondata.cpp \
	bosonlocalplayerinput.cpp \
	botexmapimportdialog.cpp \
	bobincoder.cpp \
	bolight.cpp \
	bopluginmanager.cpp \
	boglstatewidget.cpp \
	bofullscreen.cpp



BOPROFILING_SRC= \
	boapplication.cpp \
	boglobal.cpp \
	bosonprofilingdialog.cpp \
	bosonprofiling.cpp \
	boprofilingmain.cpp

#botriangles_test_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#botriangles_test_LDADD = \
#		$(top_builddir)/bodebug/libbodebug.la \
#		$(LIB_QT) $(LIB_KDECORE) \
#		-l3ds
#botriangles_test_SOURCES = \
#		bomesh.cpp bo3dtools.cpp bosonconfig.cpp \
#		botriangles_test.cpp

boson_LDFLAGS = $(KDE_RPATH) $(all_libraries) -Xlinker -export-dynamic
boson_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/items/libbosonitems.la \
		$(top_builddir)/boson/sound/libbosonsoundinterface.la \
		$(top_builddir)/boson/info/libbosoninfo.la \
		$(top_builddir)/boson/commandframe/libbosoncommandframe.la \
		$(top_builddir)/boson/startupwidgets/libbosonstartupwidgets.la \
		$(top_builddir)/boson/bosonwidgets/libbosonwidgets.la \
		$(top_builddir)/boson/script/libbosonscript.la \
		$(top_builddir)/boson/bosonfont/libbosonfont.la \
		-lkdegames \
		$(GLLIB) -l3ds \
		$(LIB_KDEUI) \
		$(LIB_XRANDR)
boson_SOURCES = \
		$(BOSON_COMMON_SRC)\
		top.cpp  \
		main.cpp

bounit_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bounit_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/items/libbosonitems.la \
		$(top_builddir)/boson/sound/libbosonsoundinterface.la \
		$(top_builddir)/boson/info/libbosoninfo.la \
		$(top_builddir)/boson/commandframe/libbosoncommandframe.la \
		$(top_builddir)/boson/bosonwidgets/libbosonwidgets.la \
		$(top_builddir)/boson/script/libbosonscript.la \
		$(top_builddir)/boson/bosonfont/libbosonfont.la \
		-lkdegames \
		$(GLLIB) -l3ds \
		-lkio $(LIB_KDEUI) \
		$(LIB_XRANDR)
bounit_SOURCES = \
		$(BOSON_COMMON_SRC) \
		bounitmain.cpp bosonsearchpathswidget.ui \
		bouniteditor.ui

boprofiling_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boprofiling_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		-lkio $(LIB_KDEUI)
boprofiling_SOURCES = \
		$(BOPROFILING_SRC)

bocursor_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bocursor_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/info/libbosoninfo.la \
		-lkio $(LIB_KDEUI) $(GLLIB)
bocursor_SOURCES = \
		boapplication.cpp \
		boglobal.cpp \
		bocursormain.cpp bosoncursoreditor.cpp bosoncursor.cpp \
		bosontexturearray.cpp bosonconfig.cpp \
		bosonglwidget.cpp

borender_LDFLAGS = $(KDE_RPATH) $(all_libraries) -Xlinker -export-dynamic
borender_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/items/libbosonitems.la \
		$(top_builddir)/boson/sound/libbosonsoundinterface.la \
		$(top_builddir)/boson/info/libbosoninfo.la \
		$(top_builddir)/boson/commandframe/libbosoncommandframe.la \
		$(top_builddir)/boson/bosonwidgets/libbosonwidgets.la \
		$(top_builddir)/boson/script/libbosonscript.la \
		$(top_builddir)/boson/bosonfont/libbosonfont.la \
		-lkdegames \
		$(GLLIB) -l3ds \
		$(LIB_KDEUI) \
		$(LIB_XRANDR)
borender_SOURCES = \
		$(BOSON_COMMON_SRC) \
		kgamemodeldebug.cpp \
		kgamespeciesdebug.cpp \
		borendermain.cpp

boinfo_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boinfo_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/info/libbosoninfo.la \
		$(GLLIB) \
		-lkio $(LIB_KDEUI)
boinfo_SOURCES = \
		boapplication.cpp \
		boglobal.cpp \
		bosonglwidget.cpp \
		boinfomain.cpp

bo3dsdebug_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bo3dsdebug_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		-l3ds \
		$(GLLIB) \
		-lkio $(LIB_KDEUI)
bo3dsdebug_SOURCES = \
		boapplication.cpp \
		boglobal.cpp \
		bo3dtools.cpp \
		kgamemodeldebug.cpp \
		bomatrixwidget.cpp \
		bo3dsdebugmain.cpp

METASOURCES = AUTO

# not yet used:
#messages: rc.cpp
#	$(XGETTEXT) rc.cpp $(COMMON_SRC) $(BOSON_COMMON_SRC) $(BOEDITOR_SRC) -o $(podir)/boson.pot

include ../admin/Doxyfile.am

