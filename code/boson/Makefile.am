## Makefile.am for boson

SUBDIRS = data \
	bopui \
	bosonwidgets \
	items \
	commandframe \
	startupwidgets \
	sound \
	info \
	script \
	bosonfont \
	meshrenderer \
	groundrenderer

noinst_LTLIBRARIES = \
		libboson.la \
		libboprofiling.la \
		libbocursor.la \
		libborender.la \
		libboinfo.la \
		libbo3dsdebug.la
bin_PROGRAMS = boson borender bounit boprofiling bocursor boinfo
#bo3dsdebug

INCLUDES = -I$(top_srcdir)/bodebug $(all_includes)


libboson_la_SOURCES = \
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
	bosonnetworksynchronizer.cpp \
	bosoncommandframeinterface.cpp \
	bosonplayerinput.cpp \
	bomessage.cpp \
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
	bofullscreen.cpp \
	bosoneffect.cpp \
	bosoneffectproperties.cpp \
	bosoneffectparticle.cpp \
	bosoneffectpropertiesparticle.cpp \
	boacload.cpp \
	boevent.cpp \
	boeventmatching.cpp \
	boeventlistener.cpp \
	boeventmanager.cpp \
	bocondition.cpp \
	boconditionwidget.cpp \
	bowater.cpp \
	botexture.cpp


libboson_la_LIBADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/items/libbosonitems.la \
		$(top_builddir)/boson/sound/libbosonsoundinterface.la \
		$(top_builddir)/boson/info/libbosoninfo.la \
		$(top_builddir)/boson/bosonwidgets/libbosonwidgets.la \
		$(top_builddir)/boson/script/libbosonscript.la \
		$(top_builddir)/boson/bosonfont/libbosonfont.la \
		$(top_builddir)/boson/bopui/libbopui.la \
		$(LIB_KGAME) \
		$(LIB_KDEUI) \
		$(LIB_KIO) \
		$(GLLIB) -l3ds \
		$(LIB_PLIB) \
		$(LIB_XRANDR)

boson_LDFLAGS = $(KDE_RPATH) $(all_libraries) -Xlinker -export-dynamic
boson_LDADD = \
		$(top_builddir)/boson/startupwidgets/libbosonstartupwidgets.la \
		$(top_builddir)/boson/commandframe/libbosoncommandframe.la \
		$(top_builddir)/boson/libboson.la \
		$(LIB_BOMEMORY)
boson_SOURCES = \
		top.cpp  \
		main.cpp

bounit_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bounit_LDADD = \
		$(top_builddir)/boson/libboson.la \
		$(LIB_BOMEMORY)
bounit_SOURCES = \
		bounitmain.cpp bosonsearchpathswidget.ui \
		bouniteditor.ui

libboprofiling_la_SOURCES = \
		boapplication.cpp \
		boglobal.cpp \
		bosonprofilingdialog.cpp \
		bosonprofiling.cpp
boprofiling_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boprofiling_LDADD = \
		$(top_builddir)/boson/libboprofiling.la \
		$(top_builddir)/bodebug/libbodebug.la \
		$(LIB_BOMEMORY) \
		$(LIB_KDEUI) \
		$(LIB_KIO)
boprofiling_SOURCES = \
		boprofilingmain.cpp

libbocursor_la_SOURCES = \
		boapplication.cpp \
		boglobal.cpp \
		bosoncursor.cpp \
		bosontexturearray.cpp \
		bosonconfig.cpp \
		bosonglwidget.cpp
bocursor_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bocursor_LDADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/libbocursor.la \
		$(top_builddir)/boson/info/libbosoninfo.la \
		$(LIB_BOMEMORY) \
		$(LIB_KDEUI) \
		$(LIB_KIO) \
		$(GLLIB)
bocursor_SOURCES = \
		bocursormain.cpp bosoncursoreditor.cpp

libborender_la_SOURCES = \
		kgamemodeldebug.cpp
borender_LDFLAGS = $(KDE_RPATH) $(all_libraries) -Xlinker -export-dynamic
borender_LDADD = \
		$(top_builddir)/boson/libboson.la \
		$(top_builddir)/boson/libborender.la \
		$(LIB_BOMEMORY)
borender_SOURCES = \
		kgamespeciesdebug.cpp \
		borendermain.cpp

libboinfo_la_SOURCES = \
		boapplication.cpp \
		boglobal.cpp \
		bosonglwidget.cpp
boinfo_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boinfo_LDADD = \
		$(top_builddir)/boson/libboinfo.la \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/info/libbosoninfo.la \
		$(LIB_BOMEMORY) \
		$(GLLIB) \
		-lkio $(LIB_KDEUI)
boinfo_SOURCES = \
		boinfomain.cpp

libbo3dsdebug_la_SOURCES = \
		boapplication.cpp \
		boglobal.cpp \
		bo3dtools.cpp \
		kgamemodeldebug.cpp \
		bomatrixwidget.cpp
bo3dsdebug_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bo3dsdebug_LDADD = \
		$(top_builddir)/boson/libbo3dsdebug.la \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/bopui/libbopui.la \
		$(LIB_BOMEMORY) \
		-l3ds \
		$(GLLIB) \
		$(LIB_PLIB) \
		-lkio $(LIB_KDEUI)
bo3dsdebug_SOURCES = \
		bo3dsdebugmain.cpp

METASOURCES = AUTO

# not yet used:
#messages: rc.cpp
#	$(XGETTEXT) rc.cpp $(COMMON_SRC) $(BOSON_COMMON_SRC) $(BOEDITOR_SRC) -o $(podir)/boson.pot

include ../admin/Doxyfile.am

