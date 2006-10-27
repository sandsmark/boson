/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bocommandframetestermain.h"
#include "bocommandframetestermain.moc"

#include "../../bomemory/bodummymemory.h"
#include "bodebugdcopiface.h"
#include "bodebug.h"
#include "../boversion.h"
#include "../boapplication.h"
#include "../bosonconfig.h"
#include "../boglobal.h"
#include "../bosonufoglwidget.h"
#include "../bocheckinstallation.h"
#include "../bosonguistarting.h"
#include "../bosonviewdata.h"
#include "../gameengine/boeventloop.h"
#include "../gameengine/boson.h"
#include "../gameengine/player.h"
#include "../gameengine/playerio.h"
#include "../gameengine/bosoncanvas.h"
#include "../gameengine/unit.h"
#include "../gameengine/bosonstarting.h"
#include "../gameengine/unitplugins/unitplugins.h"
#include "../gameview/boselection.h"
#include "../gameview/commandframe/bosoncommandframe.h"
#include "../gameview/bosonlocalplayerinput.h"
#include "mainnogui.h"
#include <boufo/boufowidget.h>
#include <boufo/boufomanager.h>
#include <boufo/boufolabel.h>
#include <bogl.h>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qtimer.h>

static void postBosonConfigInit();

static const char *description =
    I18N_NOOP("Commandframe tester for Boson");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
    { 0, 0, 0 }
};

BoCommandFrameTesterGLWidget::BoCommandFrameTesterGLWidget()
	: BosonUfoGLWidget()
{
 mTester = 0;

 QTimer* timer = new QTimer(this);
 connect(timer, SIGNAL(timeout()),
		this, SLOT(slotUpdateGL()));
 timer->start(100);

 setMouseTracking(true);
 qApp->setGlobalMouseTracking(true);
}

BoCommandFrameTesterGLWidget::~BoCommandFrameTesterGLWidget()
{
 qApp->setGlobalMouseTracking(false);
}

bool BoCommandFrameTesterGLWidget::init()
{
 initGL();

 return mTester->init();
}

void BoCommandFrameTesterGLWidget::paintGL()
{
 glClearColor(1.0, 1.0, 1.0, 0.0);
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 if (ufoManager()) {
	glColor3ub(255, 255, 255);
	ufoManager()->dispatchEvents();
	ufoManager()->render(false);
 }
}

void BoCommandFrameTesterGLWidget::initializeGL()
{
 if (isInitialized()) {
	return;
 }
 makeCurrent();
 setUpdatesEnabled(false);
 glClearColor(1.0, 1.0, 1.0, 0.0);
 glShadeModel(GL_FLAT);
 glDisable(GL_DITHER);

 initUfo();

 mTester = new BoCommandFrameTesterMain();
 ufoManager()->contentWidget()->addWidget(mTester);
}



class BoCommandFrameTesterMainPrivate
{
public:
	BoCommandFrameTesterMainPrivate()
	{
		mMainNoGUI = 0;
		mViewData = 0;
		mSelection = 0;

		mCommandFrame = 0;
	}
	MainNoGUI* mMainNoGUI;
	BosonViewData* mViewData;
	BoSelection* mSelection;

	BosonCommandFrame* mCommandFrame;
};

BoCommandFrameTesterMain::BoCommandFrameTesterMain()
	: BoUfoWidget()
{
 d = new BoCommandFrameTesterMainPrivate();

 d->mMainNoGUI = new MainNoGUI();
 d->mViewData = new BosonViewData(this);
 d->mSelection = new BoSelection(0);
 BosonViewData::setGlobalViewData(d->mViewData);

 connect(d->mMainNoGUI, SIGNAL(signalAddIOs(Player*, int*, bool*)),
		this, SLOT(slotAddIOs(Player*, int*, bool*)));

 d->mCommandFrame = new BosonCommandFrame();
 connect(d->mSelection, SIGNAL(signalSelectionChanged(BoSelection*)),
		d->mCommandFrame, SLOT(slotSelectionChanged(BoSelection*)));
 addWidget(d->mCommandFrame);
  d->mCommandFrame->setGameMode(true);
 // TODO:
 // d->mCommandFrame->setCursorRootPos();

 BosonStarting* starting = d->mMainNoGUI->startingObject();
 BosonGUIStarting* gui = new BosonGUIStarting(starting, starting);
 starting->addTaskCreator(gui);
}

BoCommandFrameTesterMain::~BoCommandFrameTesterMain()
{
 BosonViewData::setGlobalViewData(0);
 delete d->mViewData;
 delete d->mSelection;
 delete d->mMainNoGUI;
 delete d;
}

void BoCommandFrameTesterMain::slotAddIOs(Player* p, int* ioMask, bool* failure)
{
 if ((*ioMask) & MainNoGUIAIPlayerOptions::LocalPlayerIO) {
	BosonLocalPlayerInput* io = new BosonLocalPlayerInput();
	p->addGameIO(io);
	if (!io->initializeIO()) {
		boError() << k_funcinfo << "local player IO could not be initialized" << endl;
		*failure = true;
		return;
	}
	(*ioMask) &= ~MainNoGUIAIPlayerOptions::LocalPlayerIO;
 }
}

bool BoCommandFrameTesterMain::init()
{
 if (!d->mMainNoGUI->init()) {
	boError() << k_funcinfo << "init failed" << endl;
	KMessageBox::sorry(0, i18n("Unable to initialize game"));
	return 1;
 }

 connect(boGame, SIGNAL(signalAdvance(unsigned int, bool)),
		this, SLOT(slotAdvance(unsigned int, bool)));

 connect(boGame->canvas(), SIGNAL(signalRemovedItem(BosonItem*)),
		d->mSelection, SLOT(slotRemoveItem(BosonItem*)));


 return start();
}

bool BoCommandFrameTesterMain::start()
{
 MainNoGUIStartOptions options;

 MainNoGUIAIPlayerOptions player;
 player.io = MainNoGUIAIPlayerOptions::LocalPlayerIO;
 options.computerPlayers.append(player);

 return d->mMainNoGUI->startGame(options);
}

void BoCommandFrameTesterMain::slotAdvance(unsigned int advanceCallsCount, bool)
{
 BO_CHECK_NULL_RET(boGame);
 PlayerIO* playerIO = boGame->playerIOAtGameIndex(0);
 BO_CHECK_NULL_RET(playerIO);
 BosonLocalPlayerInput* playerInput = (BosonLocalPlayerInput*)playerIO->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);




 if (advanceCallsCount == 0) {
	boDebug() << k_funcinfo << " STARTING (advance call 0 received)" << endl;

	d->mCommandFrame->setLocalPlayerIO(playerIO);

	disconnect(d->mCommandFrame, SIGNAL(signalAdvance(const BoSpecificAction&)), 0, 0);
	if (!playerInput) {
		boWarning() << k_funcinfo << "no BosonLocalPlayerInput found in player. commandframe won't be able to do anything" << endl;
	} else {
		connect(d->mCommandFrame, SIGNAL(signalAction(const BoSpecificAction&)),
			playerInput, SLOT(slotAction(const BoSpecificAction&)));
	}

	QPtrList<Unit> units = playerIO->allMyLivingUnits();
	Unit* factory = 0;
	for (QPtrListIterator<Unit> it(units); it.current(); ++it) {
		ProductionPlugin* p = (ProductionPlugin*)it.current()->plugin(UnitPlugin::Production);
		if (!p) {
			continue;
		}
		factory = it.current();
	}
	if (factory) {
		boDebug() << k_funcinfo << "selecting factory " << factory->id() << endl;
		d->mSelection->selectUnit(factory);
	}
 }
}


int main(int argc, char **argv)
{
 KAboutData about("bocommandframetester",
		I18N_NOOP("Boson Commandframe Tester"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 2006 Andreas Beckermann",
		0,
		"http://boson.eu.org");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de" );

 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 QCString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);

 BoEventLoop eventLoop(0, "main event loop");
 BoApplication app(argv0);
 KGlobal::locale()->insertCatalogue("libkdegames");

 BoCheckInstallation checkInstallation;
 QString errorMessage = checkInstallation.checkInstallation();
 if (!errorMessage.isNull()) {
	boError() << k_funcinfo << errorMessage << endl;
	boError() << k_funcinfo << "check your installation!" << endl;
	KMessageBox::sorry(0, errorMessage, i18n("Check your installation"));
	return 1;
 }


 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

 BoCommandFrameTesterGLWidget* top = new BoCommandFrameTesterGLWidget();
 app.setMainWidget(top);
 top->init();
 top->show();

 BoDebugDCOPIface* iface = new BoDebugDCOPIface();
 args->clear();
 int r = app.exec();
 delete iface;
 delete top;
 return r;
}


static void postBosonConfigInit()
{
 BosonConfig* conf = BoGlobal::boGlobal()->bosonConfig();
 if (!conf) {
	boError() << k_funcinfo << "NULL BosonConfig object" << endl;
	return;
 }
 conf->setBoolValue("ForceDisableSound", true);
 conf->setBoolValue("ForceDisableModelLoading", true);
}

