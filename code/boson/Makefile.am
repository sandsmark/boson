## Makefile.am for boson

SUBDIRS = \
		data \
		bosonfont \
		boufo \
		bosonwidgets \
		items \
		commandframe \
		startupwidgets \
		sound \
		info \
		script \
		meshrenderer \
		groundrenderer \
		gameview

noinst_LTLIBRARIES = \
		libbosoncommon.la \
		libbosongameengine.la \
		libboson.la \
		libbocursor.la \
		libborender.la \
		libboinfo.la \
		libbo3dsdebug.la \
		libboprofiling.la
bin_PROGRAMS = boson borender bounit bocursor boinfo bo3dsdebug boconditioneditor boprofiling

INCLUDES = -I$(top_srcdir)/bodebug -I$(top_srcdir)/bogl -I$(top_srcdir)/boson/boufo $(all_includes)


libbosongameengine_la_SOURCES = \
		boupgradeableproperty.cpp \
		unitbase.cpp \
		unit.cpp \
		unitproperties.cpp \
		pluginproperties.cpp \
		unitplugins.cpp \
		upgradeproperties.cpp \
		bosonweapon.cpp \
		boitemlist.cpp \
		boitemlisthandler.cpp \
		bosonplayfield.cpp \
		bosonmap.cpp \
		cell.cpp \
		bofile.cpp \
		bpfdescription.cpp \
		bosonfileconverter.cpp \
		bosoncampaign.cpp \
		bosonmessage.cpp \
		boson.cpp \
		bomessage.cpp \
		bosoncanvas.cpp \
		bosoncanvasstatistics.cpp \
		bosoncollisions.cpp \
		bosonnetworksynchronizer.cpp \
		speciestheme.cpp \
		player.cpp \
		playerio.cpp \
		bosoncomputerio.cpp \
		bosonpath.cpp \
		bosonstatistics.cpp \
		bogameproperty.cpp \
		bosonpropertyxml.cpp \
		bosonplayerinputhandler.cpp \
		bobincoder.cpp \
		boevent.cpp \
		boeventmatching.cpp \
		boeventlistener.cpp \
		boeventmanager.cpp \
		bocondition.cpp \
		bowater.cpp \
		bosonsaveload.cpp \
		bosongroundtheme.cpp

libbosoncommon_la_SOURCES = \
		boapplication.cpp \
		boeventloop.cpp \
		boglobal.cpp \
		bosonconfig.cpp \
		bosondata.cpp \
		bomath.cpp \
		bo3dtools.cpp \
		bosonprofiling.cpp \
		bopluginmanager.cpp

libboson_la_SOURCES = \
		bosonviewdata.cpp \
		boselection.cpp \
		botooltipcreator.cpp \
		bosonglwidget.cpp \
		bosonufoglwidget.cpp \
		bomousemovediff.cpp \
		bomatrixwidget.cpp \
		bocamerawidget.cpp \
		boorbiterwidget.cpp \
		bosonufochat.cpp \
		bosonprofilingdialog.cpp \
		bpfdescriptiondialog.cpp \
		boglstatewidget.cpp \
		bodebuglogdialog.cpp \
		bofullscreen.cpp \
		bocamera.cpp \
		boautocamera.cpp \
		bosoncursor.cpp \
		bosonmodel.cpp \
		bosonmodeltextures.cpp \
		bomesh.cpp \
		bomeshrenderer.cpp \
		bomeshrenderermanager.cpp \
		bomaterial.cpp \
		bolight.cpp \
		botexture.cpp \
		bobmfload.cpp \
		boshader.cpp \
		speciesdata.cpp \
		boaction.cpp \
		bosongroundthemedata.cpp \
		borendertarget.cpp

libbosongameengine_la_LIBADD = \
		$(top_builddir)/boson/items/libbosonitems.la

libboson_la_LIBADD = \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/math/libbomath.la \
		$(top_builddir)/boson/sound/libbosonsoundinterface.la \
		$(top_builddir)/boson/info/libbosoninfo.la \
		$(top_builddir)/boson/bosonwidgets/libbosonwidgets.la \
		$(top_builddir)/boson/script/libbosonscript.la \
		$(LIB_MESHRENDERER) \
		$(LIB_GROUNDRENDERER) \
		$(LIB_GAMEVIEWPLUGIN) \
		$(LIB_KGAME) \
		$(LIB_KDEUI) \
		$(LIB_KIO) \
		-l3ds \
		-lXmu \
		$(LIB_XRANDR)

boson_LDFLAGS = $(KDE_RPATH) $(all_libraries) -Xlinker --export-dynamic
boson_LDADD = \
		$(top_builddir)/boson/startupwidgets/libbosonstartupwidgets.la \
		$(top_builddir)/boson/gameview/libbosongameview.la \
		$(top_builddir)/boson/commandframe/libbosoncommandframe.la \
		$(top_builddir)/boson/libboson.la \
		$(top_builddir)/boson/libbosongameengine.la \
		$(top_builddir)/boson/libbosoncommon.la \
		$(top_builddir)/boson/boufo/libboufo.la \
		$(LIB_BOMEMORY)
boson_SOURCES = \
		bosonfpscounter.cpp \
		bosonmainwidget.cpp \
		bosonmainwidgetmenuinput.cpp \
		boufoactionext.cpp \
		bogltooltip.cpp \
		bosonglminimap.cpp \
		bosonufominimap.cpp \
		optionsdialog.cpp \
		optionswidgets.cpp \
		bogroundrenderer.cpp \
		bogroundrenderermanager.cpp \
		kgameunitdebug.cpp \
		kgameadvancemessagesdebug.cpp \
		kgameplayerdebug.cpp \
		kgamecelldebug.cpp \
		gameoverdialog.cpp \
		bocheckinstallation.cpp \
		bosonstarting.cpp \
		bosongameviewpluginbase.cpp \
		bosongameviewpluginmanager.cpp \
		bosoneffectmanager.cpp \
		bosoneffect.cpp \
		bosoneffectproperties.cpp \
		bosoneffectparticle.cpp \
		bosoneffectpropertiesparticle.cpp \
		botexmapimportdialog.cpp \
		boconditionwidget.cpp \
		bowaterrenderer.cpp \
		selectbox.cpp \
		main.cpp

bounit_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bounit_LDADD = \
		$(top_builddir)/boson/libbosongameengine.la \
		$(top_builddir)/boson/libbosoncommon.la \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/math/libbomath.la \
		$(top_builddir)/boson/info/libbosoninfo.la \
		$(top_builddir)/boson/script/libbosonscript.la \
		$(LIB_KGAME) \
		$(LIB_KDEUI) \
		$(LIB_KIO) \
		-lXmu \
		$(LIB_XRANDR) \
		$(LIB_BOMEMORY)
bounit_SOURCES = \
		bounitmain.cpp \
		bosonsearchpathswidgetbase.ui \
		bosonsearchpathswidget.cpp \
		bouniteditorbase.ui \
		bouniteditor.cpp

libbocursor_la_SOURCES = \
		boapplication.cpp \
		boglobal.cpp \
		bosoncursor.cpp \
		botexture.cpp \
		bosonprofiling.cpp \
		bosonconfig.cpp \
		bosonglwidget.cpp
bocursor_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bocursor_LDADD = \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/math/libbomath.la \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/libbocursor.la \
		$(top_builddir)/boson/info/libbosoninfo.la \
		$(LIB_BOMEMORY) \
		$(LIB_KDEUI) \
		$(LIB_KIO) \
		-lXmu
bocursor_SOURCES = \
		bocursormain.cpp bosoncursoreditor.cpp

libborender_la_SOURCES = \
		kgamemodeldebug.cpp
borender_LDFLAGS = $(KDE_RPATH) $(all_libraries) -Xlinker --export-dynamic
borender_LDADD = \
		$(top_builddir)/math/libbomath.la \
		$(top_builddir)/boson/libbosongameengine.la \
		$(top_builddir)/boson/libboson.la \
		$(top_builddir)/boson/libbosoncommon.la \
		$(top_builddir)/boson/libborender.la \
		$(top_builddir)/boson/boufo/libboufo.la \
		$(LIB_BOMEMORY) -l3ds
borender_SOURCES = \
		bomaterialwidget.cpp \
		bovectorinput.cpp \
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
		$(top_builddir)/math/libbomath.la \
		$(top_builddir)/bogl/libbogl.la \
		$(LIB_BOMEMORY) \
		-lkio $(LIB_KDEUI) \
		-lXmu
boinfo_SOURCES = \
		boinfomain.cpp

libbo3dsdebug_la_SOURCES = \
		boapplication.cpp \
		boglobal.cpp \
		bomath.cpp \
		bo3dtools.cpp \
		bosonprofiling.cpp \
		kgamemodeldebug.cpp \
		bomatrixwidget.cpp
bo3dsdebug_LDFLAGS = $(KDE_RPATH) $(all_libraries)
bo3dsdebug_LDADD = \
		$(top_builddir)/boson/libbo3dsdebug.la \
		$(top_builddir)/math/libbomath.la \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/boson/boufo/libboufo.la \
		$(LIB_BOMEMORY) \
		-l3ds \
		-lkio $(LIB_KDEUI)
bo3dsdebug_SOURCES = \
		bo3dsdebugmain.cpp


boconditioneditor_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boconditioneditor_LDADD = \
		$(top_builddir)/boson/libbosongameengine.la \
		$(top_builddir)/boson/libbosoncommon.la \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/math/libbomath.la \
		$(top_builddir)/boson/script/libbosonscript.la \
		$(LIB_KGAME) \
		$(LIB_KDEUI) \
		$(LIB_KIO) \
		-lXmu \
		$(LIB_XRANDR) \
		$(LIB_BOMEMORY)
boconditioneditor_SOURCES = \
		boconditionwidget.cpp \
		boconditioneditormain.cpp

libboprofiling_la_SOURCES = \
		bosonprofiling.cpp \
		bosonprofilingdialog.cpp
boprofiling_LDFLAGS = $(KDE_RPATH) $(all_libraries)
boprofiling_LDADD = \
		$(top_builddir)/boson/libboprofiling.la \
		$(top_builddir)/boson/libbosoncommon.la \
		$(top_builddir)/bodebug/libbodebug.la \
		$(top_builddir)/bogl/libbogl.la \
		$(top_builddir)/math/libbomath.la \
		$(LIB_KDEUI) \
		$(LIB_KIO) \
		-lXmu \
		$(LIB_XRANDR) \
		$(LIB_BOMEMORY)
boprofiling_SOURCES = \
		boprofilingmain.cpp


METASOURCES = AUTO

# not yet used:
#messages: rc.cpp
#	$(XGETTEXT) rc.cpp $(COMMON_SRC) $(BOSON_COMMON_SRC) $(BOEDITOR_SRC) -o $(podir)/boson.pot

include ../admin/Doxyfile.am

