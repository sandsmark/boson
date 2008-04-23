/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2008 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <bogl.h>

#include "bosonmainwidget.h"
#include "bosonmainwidget.moc"

#include "defines.h"
#include "../bomemory/bodummymemory.h"
#include "bosonconfig.h"
#include "bosonprofiling.h"
#include "boson.h"
#include "bo3dtools.h"
#include "info/boinfo.h"
#include "bofullscreen.h"
#include "botexture.h"
#include "boufo/boufo.h"
#include "boufo/boufoaction.h"
#include "gameengine/bosonstarting.h"
#include "gameengine/bosongameenginestarting.h"
#include "bosonguistarting.h"
#include "bosongameviewstarting.h"
#include "gameview/bosongameview.h"
#include "gameview/bosonqtgameview.h"
#include "gameview/bosonlocalplayerinput.h"
#include "startupwidgets/boufostartupwidget.h"
#include "startupwidgets/boqtstartupwidget.h"
#include "startupwidgets/boufoloadsavegamewidget.h" // for BoUfoLoadSaveGameWidget::defaultDir()
#include "gameengine/player.h"
#include "gameengine/bosonmessageids.h"
#include "sound/bosonaudiointerface.h"
#include "gameengine/bosonsaveload.h"
#include "gameengine/bosongroundtheme.h"
#include "gameengine/bosonplayfield.h"
#include "bosoncursor.h"
#include "bosonfpscounter.h"
#include "bosonmainwidgetmenuinput.h"
#include "bosondata.h"
//#include "boufo/boufodebugwidget.h"
#include "boufo/boufofactory.h"
#include "bodebug.h"
#include "optionsdialog/optionsdialog.h"
#include "gameengine/bosongameengine.h"
#include "gameengine/bosoncomputerio.h"
#include "gameengine/speciestheme.h"
#include "bosondebugtextures.h"
#include "bosondebugmodels.h"
#include "boadvancecontrol.h"
#include "bosonufowidget.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kdialog.h>

#include <qtimer.h>
#include <qbuffer.h>
#include <qimage.h>
#include <qdir.h>
#include <qdom.h>
#include <q3valuevector.h>
#include <qpointer.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <QImageWriter>
#include <QPixmap>
#include <QStackedWidget>


#include <config.h>
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
// won't compile anymore!
#warning You dont have sys/time.h - please report this problem to boson-devel@lists.sourceforge.net and provide us with information about your system!
#endif
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#define FOO_0 0

class BosonMainWidgetPrivate
{
public:
	BosonMainWidgetPrivate()
	{
		mUfoWidget = 0;
		mGameEngine = 0;
		mQtWidgetStack = 0;
		mWidgetStack = 0;
		mStartup = 0;
		mQtStartup = 0;
		mGameView = 0;
		mQtGameView = 0;
		mMenuInput = 0;

		mFPSCounter = 0;

		mLocalPlayer = 0;

		mStarting = 0;
	}

	BosonUfoWidget* mUfoWidget;
	BosonGameEngine* mGameEngine;
	QStackedWidget* mQtWidgetStack;
	BoUfoWidgetStack* mWidgetStack;
	BoUfoStartupWidget* mStartup;
	BoQtStartupWidget* mQtStartup;
	BosonGameView* mGameView;
	BosonQtGameView* mQtGameView;
	BosonMainWidgetMenuInput* mMenuInput;

	BosonFPSCounter* mFPSCounter;

	bool mGrabMovie;

	QTimer mUpdateTimer;
	int mUpdateInterval;

	BosonStarting* mStarting;

	QPointer<Player> mLocalPlayer;

	bool mIsInUpdateGL;
};

BosonMainWidget::BosonMainWidget(QWidget* parent)
		: QWidget(parent)
{
 boDebug() << k_funcinfo << endl;
 init();

#warning FIXME: initializeGL should be called manually, not by the ctor! -> make sure the GL widget is already initialized!
 initializeGL();
 boDebug() << k_funcinfo << "done" << endl;
}

BosonMainWidget::~BosonMainWidget()
{
 boDebug() << k_funcinfo << endl;

 // the bigdisplay destructor is the place where many systems tend to crash - so
 // we go back to the original (non-fullscreen) mode here
 BoFullScreen::enterOriginalMode();

 qApp->setGlobalMouseTracking(false);

 bool editor = false;
 if (boGame) {
	editor = !boGame->gameMode();
 }
 boConfig->save(editor);
 endGame();
 delete d->mMenuInput;

#if 0
 // AB: the following is a BoUfo vs valgrind issue.
 //     removeAllWidgets() will delete all ufo children.
 //     in every child, the BoUfoWidget obejct is deleted first, only then the
 //     grand-children are deleted. So when a grand-child emits a signal for
 //     which a corresponding slot needs to access a BoUfoWidget, that one is
 //     likely to be deleted already.
 //     I don't see a clean way around this yet, so we just prevent emitting
 //     such signals (which are noops in the destructor anyway)
 disconnect(d->mGameView, SIGNAL(signalSetWidgetCursor(BosonCursor*)), this, 0);
#endif

 if (d->mUfoWidget->ufoManager()) {
	boDebug() << k_funcinfo << "removing ufo widgets" << endl;
	d->mUfoWidget->ufoManager()->contentWidget()->removeAllWidgets();
	boDebug() << k_funcinfo << "removed ufo widgets" << endl;
 }
 BosonData::bosonData()->clearData();
 BoTextureManager::deleteStatic();
 delete d->mUfoWidget;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonMainWidget::init()
{
 d = new BosonMainWidgetPrivate;
 d->mIsInUpdateGL = false;
 d->mUpdateInterval = 0;
 d->mGrabMovie = false;

 d->mFPSCounter = new BosonFPSCounter(this);
 connect(d->mFPSCounter, SIGNAL(signalSkipFrame()),
		this, SLOT(slotSkipFrame()));

 setUpdatesEnabled(true);
 setFocusPolicy(Qt::WheelFocus);
 setMouseTracking(true);

#if 0
 if (!isValid()) {
	boError() << k_funcinfo << "No OpenGL support present on this system??" << endl;
	return;
 }
#endif

 boAudio->setSound(boConfig->boolValue("Sound"));
 boAudio->setMusic(boConfig->boolValue("Music"));

#if FOO_0
 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
#endif
 slotSetUpdateInterval(boConfig->uintValue("GLUpdateInterval"));

 qApp->setGlobalMouseTracking(true);

 boProfiling->setMaximalEntries("GL", boConfig->uintValue("MaxProfilingEntriesGL"));
 boProfiling->setMaximalEntries("Advance", boConfig->uintValue("MaxProfilingEntriesAdvance"));
 boProfiling->setMaximalEntries("Default", boConfig->uintValue("MaxProfilingEntries"));
}

void BosonMainWidget::initializeGL()
{
 static bool isInitialized = false;
 if (isInitialized) {
	return;
 }
 isInitialized = true;
 boDebug() << k_funcinfo << endl;
 BosonProfiler prof("initializeGL");

 // AB: WARNING you must _not_ assume this gets called once only!
 // this can get called once per context! i.e. when frames for the movie are
 // generated, then this will get called as well!
 // (keep this in mind when allocating memory)

#if 0
 if (!context()) {
	boError() << k_funcinfo << "NULL context" << endl;
	return;
 }
#endif
 static bool recursive = false;
 if (recursive) {
	// this can happen e.g. when a paint event occurs while we are in this
	// function (e.g. because of a messagebox)
	return;
 }
 recursive = true;

 glDisable(GL_DITHER); // we don't need this (and its enabled by default)

 // for anti-aliased lines (currently unused):
 glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
 // for anti-aliased points (currently unused):
 glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
 // for anti-aliased polygons (currently unused):
 glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

 glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

#if 0
#warning FIXME: is extension there?
 // AB: if everything is inside the view volume we can use this to skip the
 // volume clipping tests. should be faster
 // FIXME: do we actually render stuff thats inside the view volume only?
 glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
#endif

 BoGL::bogl()->initialize();

 d->mFPSCounter->reset();

#ifdef HAVE_DEVICE_IS_PIXMAP
 if (!context()->deviceIsPixmap()) {
#endif
	// start rendering (will also start the timer if necessary)
#if FOO_0
	 QTimer::singleShot(d->mUpdateInterval, this, SLOT(updateGL()));
#endif

	// update system information (initializeGL() must have been called before
	// this makes sense)
	boProfiling->push("Update BoInfo");
	BoInfo::boInfo()->update(this);
	boProfiling->pop();
#ifdef HAVE_DEVICE_IS_PIXMAP
 }
#endif

 boProfiling->push("init texture manager");
 BoTextureManager::initStatic();
 boProfiling->pop();

 recursive = false;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonMainWidget::initGUI()
{
 initUfoGUI();

 d->mQtWidgetStack = new QStackedWidget(this);

 d->mQtStartup = new BoQtStartupWidget(this);
 connect(d->mQtStartup, SIGNAL(signalAddLocalPlayer()),
		this, SLOT(slotAddLocalPlayer()));
 connect(d->mQtStartup, SIGNAL(signalResetGame()),
		this, SLOT(slotResetGame()));
 connect(d->mQtStartup, SIGNAL(signalGameOver()),
		this, SLOT(slotGameOver()));
 connect(d->mQtStartup, SIGNAL(signalQuit()), this, SLOT(close()));
 connect(d->mQtStartup, SIGNAL(signalPreferences()), this, SLOT(slotPreferences()));
 connect(d->mQtStartup, SIGNAL(signalCancelLoadSave()),
		this, SLOT(slotCancelLoadSave()));
 connect(d->mQtStartup, SIGNAL(signalPreferredSizeChanged()),
		this, SLOT(slotStartupPreferredSizeChanged()));
#if FOO_0
 connect(d->mQtStartup, SIGNAL(signalUpdateGL()),
		this, SLOT(updateGL()));
#endif
 d->mQtGameView = new BosonQtGameView(this);
// d->mQtGameView->createActionCollection(d->mUfoWidget->ufoManager()->actionCollection());
 connect(d->mQtGameView, SIGNAL(signalEditorChangeLocalPlayer(Player*)),
		this, SLOT(slotChangeLocalPlayer(Player*)));
 connect(d->mQtGameView, SIGNAL(signalEndGame()),
		this, SLOT(slotEndGame()));
 connect(d->mQtGameView, SIGNAL(signalQuit()),
		this, SLOT(close()));
 connect(d->mQtGameView, SIGNAL(signalSaveGame()),
		this, SLOT(slotShowSaveGamePage()));
 connect(d->mQtGameView, SIGNAL(signalLoadGame()),
		this, SLOT(slotShowLoadGamePage()));
 connect(d->mQtGameView, SIGNAL(signalQuicksaveGame()),
		this, SLOT(slotQuicksaveGame()));
 connect(d->mQtGameView, SIGNAL(signalQuickloadGame()),
		this, SLOT(slotQuickloadGame()));
 connect(d->mQtGameView, SIGNAL(signalSetWidgetCursor(BosonCursor*)),
		this, SLOT(slotSetWidgetCursor(BosonCursor*)));
 d->mQtGameView->setGameFPSCounter(new BosonGameFPSCounter(d->mFPSCounter));
 d->mQtGameView->hide();

 d->mQtWidgetStack->addWidget(d->mQtStartup);
 d->mQtWidgetStack->addWidget(d->mQtGameView);

 raiseQtWidget(d->mQtStartup);

 QVBoxLayout* layout = new QVBoxLayout(this);
 layout->addWidget(d->mQtWidgetStack);
 QLabel* foo = new QLabel("foo", this);
 layout->addWidget(foo);
}

void BosonMainWidget::initUfoGUI()
{
 static bool initialized = false;
 if (initialized) {
	return;
 }
 initialized = true;
 PROFILE_METHOD
 glPushAttrib(GL_ALL_ATTRIB_BITS);

 d->mUfoWidget = new BosonUfoWidget(this);
// addWidgetToScene(d->mUfoWidget, 0.0);

 boProfiling->push("initUfo()");
 d->mUfoWidget->initUfo();
 boProfiling->pop();

 BoUfoActionCollection::initActionCollection(d->mUfoWidget->ufoManager());
 d->mUfoWidget->ufoManager()->actionCollection()->setAccelWidget(this);
 d->mMenuInput = new BosonMainWidgetMenuInput(d->mUfoWidget->ufoManager()->actionCollection(), this);
 connect(d->mMenuInput, SIGNAL(signalDebugUfoWidgets()),
		this, SLOT(slotDebugUfoWidgets()));
 connect(d->mMenuInput, SIGNAL(signalDebugTextures()),
		this, SLOT(slotDebugTextures()));
 connect(d->mMenuInput, SIGNAL(signalDebugModels()),
		this, SLOT(slotDebugModels()));
 connect(d->mMenuInput, SIGNAL(signalPreferences()),
		this, SLOT(slotPreferences()));

 BoUfoWidget* contentWidget = d->mUfoWidget->ufoManager()->contentWidget();
 contentWidget->setLayoutClass(BoUfoWidget::UFullLayout);

 d->mWidgetStack = (BoUfoWidgetStack*)BoUfoFactory::createWidget("BoUfoWidgetStack");
 d->mWidgetStack->setName("WidgetStackOfMainWidget");
 contentWidget->addWidget(d->mWidgetStack);

 d->mStartup = new BoUfoStartupWidget();
 d->mStartup->show();

 d->mGameView = new BosonGameView();
 d->mGameView->createActionCollection(d->mUfoWidget->ufoManager()->actionCollection());
 d->mGameView->setGameFPSCounter(new BosonGameFPSCounter(d->mFPSCounter));
 d->mGameView->hide();
 boProfiling->push("adding startup widget to widget stack");
 d->mWidgetStack->addWidget(d->mStartup);
 boProfiling->pop();
 boProfiling->push("adding gameview to widget stack");
 d->mWidgetStack->addWidget(d->mGameView);
 boProfiling->pop();
 d->mWidgetStack->insertStackWidget(d->mStartup);
 d->mWidgetStack->insertStackWidget(d->mGameView);

 raiseWidget(d->mStartup);

 d->mGameView->setMouseEventsEnabled(true, true);

 boProfiling->push("createGUI()");
 d->mUfoWidget->ufoManager()->actionCollection()->createGUI();
 boProfiling->pop();

 glPopAttrib();
}

void BosonMainWidget::setGameEngine(BosonGameEngine* gameEngine)
{
 if (d->mGameEngine) {
	disconnect(d->mGameEngine, 0, this, 0);
 }
 d->mGameEngine = gameEngine;
 if (d->mGameEngine) {
	connect(d->mGameEngine, SIGNAL(signalBosonObjectAboutToBeDestroyed(Boson*)),
			this, SLOT(slotBosonObjectAboutToBeDestroyed(Boson*)));
 }
}

void BosonMainWidget::slotBosonObjectAboutToBeDestroyed(Boson* b)
{
 if (d->mQtGameView) {
	d->mQtGameView->bosonObjectAboutToBeDestroyed(b);
 }
}

void BosonMainWidget::resizeGL(int w, int h)
{
 boDebug() << k_funcinfo << w << " " << h << endl;
 //BosonGLWidget::resizeGL(w, h);

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << endl;
 }

 d->mUfoWidget->resize(w, h);
 d->mUfoWidget->resizeGL(w, h);

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << endl;
 }
}


class BosonProfilingGlFinishPopTask : public BosonProfilingPopTask
{
public:
	virtual void pop()
	{
		glFinish();
	}
};

void BosonMainWidget::updateGL()
{
 d->mIsInUpdateGL = true;
 BosonProfilingGlFinishPopTask popTask;
 bool glFinishOnProfilingPop = boConfig->boolValue("debug_glfinish_before_profiling", false);
 if (glFinishOnProfilingPop) {
	boProfiling->setPopTask(&popTask);
 }

 boProfiling->pushStorage("GL");
 boProfiling->push("updateGL");

 // AB: using the FPS counter here is most dependable and gives us some
 // additional information.
 // by doing this we can especially find out how much time _actually_ was spent
 // on rendering, i.e. including swapBuffers(), which we have no control of in
 // paintGL().
 d->mFPSCounter->startFrame();
#if 0
 BosonGLWidget::updateGL();
#else
 paintGL();
#endif
 d->mFPSCounter->endFrame();

 boProfiling->pop();
 boProfiling->popStorage();

 boProfiling->setPopTask(0);
 d->mIsInUpdateGL = false;
}

void BosonMainWidget::paintEvent(QPaintEvent*)
{
	return;
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();
 glPushAttrib(GL_ALL_ATTRIB_BITS);
 updateGL();
 glPopAttrib();
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
}

void BosonMainWidget::paintGL()
{
 if (!d->mIsInUpdateGL) {
	boWarning() << "paintGL() called from outside updateGL()!";
	return;
 }
 BosonProfiler prof("paintGL");

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error at start of " << k_funcinfo << endl;
 }

 // buffer swapping might get disabled when a frame is skipped - reenable it
// setAutoBufferSwap(true);

#if 0 // disabled for qt port: Qt does this already
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

 boTextureManager->clearStatistics();
 glColor3ub(255, 255, 255);

 d->mUpdateTimer.stop();

 boProfiling->push("Rendering");

 glDisable(GL_DEPTH_TEST);
 glDisable(GL_LIGHTING);
 glDisable(GL_NORMALIZE);
 boTextureManager->disableTexturing();
 glDisable(GL_FOG);

 boProfiling->push("renderUfo");
 renderUfo();
 boProfiling->pop(); // "renderUfo"

 boProfiling->pop(); // "Rendering"

 if (d->mUpdateInterval) {
	d->mUpdateTimer.start(d->mUpdateInterval);
 }

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error at end of " << k_funcinfo << endl;
 }
}

void BosonMainWidget::renderUfo()
{
	boDebug();
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error before method" << endl;
 }
 if (d->mUfoWidget->ufoManager()) {
	boTextureManager->invalidateCache();
	glColor3ub(255, 255, 255);

	boProfiling->push("dispatchEvents");
	d->mUfoWidget->ufoManager()->dispatchEvents();
	boProfiling->pop(); // "dispatchEvents"
	d->mUfoWidget->ufoManager()->render(false);
 }
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error at end of method" << endl;
 }
}

void BosonMainWidget::slotSetUpdateInterval(unsigned int ms)
{
 d->mUpdateInterval = ms;
#if FOO_0
 QTimer::singleShot(d->mUpdateInterval, this, SLOT(updateGL()));
#endif
}

QByteArray BosonMainWidget::grabMovieFrame()
{
 boDebug() << k_funcinfo << "Grabbing movie frame..." << endl;

 // Repaint
 updateGL();
 glFinish();

 // Slots in Qt can be called in any order, so it is possible that particle
 //  systems haven't been advanced yet. If particles' dirty flag would remain
 //  false, then if some particles or particle systems would be deleted, we'd
 //  have 0 pointers in particle list next time.
 // No need to do this before repainting, because it has already been done in
 //  BoDisplayManager (setParticlesDirty() is called before grabbing movie
 //  frame)
#warning FIXME: canvas renderer in movies
#if 0
 d->mCanvasRenderer->setParticlesDirty(true);
#endif

 // WARNING this is NOT dependable! e.g. if boson has the focus, but another
 // window is in front of it, then the other window will be grabbed as well!
 // better render to a pixmap instead.
 QPixmap shot = QPixmap::grabWindow(winId());

 QByteArray ba;
 QBuffer b(&ba);
 b.open(QIODevice::WriteOnly);
 QImageWriter io(&b, "JPEG");
 io.setQuality(90);
 io.write(shot.convertToImage());
 return ba;
}

void BosonMainWidget::grabMovieFrameAndSave()
{
 if (!d->mGrabMovie) {
	return;
 }
 QByteArray shot = grabMovieFrame();

 if (shot.size() == 0) {
	return;
 }

 // Save frame
 static int frame = -1;
 QString file;
 if (frame == -1) {
	int i;
	for (i = 0; i <= 10000; i++) {
		file.sprintf("%s-%04d.%s", "boson-movie", i, "jpg");
		if (!QFile::exists(file)) {
			frame = i;
			break;
		}
	}
	if (i == 10000) {
		boWarning() << k_funcinfo << "Can't find free filename???" << endl;
		frame = 50000;
	}
 }
 file.sprintf("%s-%04d.%s", "boson-movie", frame++, "jpg");
 file = QFileInfo(file).absoluteFilePath();

 //boDebug() << k_funcinfo << "Saving movie frame to " << file << endl;
 bool ok = QPixmap(shot).save(file, "JPEG", 90);
 if (!ok) {
	boError() << k_funcinfo << "Error saving screenshot to " << file << endl;
	return;
 }
 boDebug() << k_funcinfo << "Movie frame saved to file " << file << endl;
}


void BosonMainWidget::initBoson()
{
 d->mGameEngine->initGame();
 if (!d->mStarting) {
	BO_NULL_ERROR(d->mStarting);
 }
#if FOO_0
 connect(boGame, SIGNAL(signalUpdateGL()), SLOT(updateGL()));
#endif

 connect(boGame, SIGNAL(signalStartingCompletedReceived(const QByteArray&, quint32)),
		d->mStarting, SLOT(slotStartingCompletedReceived(const QByteArray&, quint32)));
 connect(boGame, SIGNAL(signalSetNewGameData(const QByteArray&, bool*)),
		d->mStarting, SLOT(slotSetNewGameData(const QByteArray&, bool*)));

 // new games are handled in this order: ADMIN clicks on start games - this
 // sends an IdStartGame over network. Once this is received signalStartNewGame()
 // is emitted and we start here
 connect(boGame, SIGNAL(signalStartNewGame()), this, SLOT(slotStartNewGame()));
 connect(boGame, SIGNAL(signalGameStarted()), this, SLOT(slotGameStarted()));

 connect(boGame, SIGNAL(signalLoadExternalStuffFromXML(const QDomElement&)),
		this, SLOT(slotLoadExternalStuffFromXML(const QDomElement&)));
 connect(boGame, SIGNAL(signalSaveExternalStuffAsXML(QDomElement&)),
		this, SLOT(slotSaveExternalStuffAsXML(QDomElement&)));
 connect(boGame, SIGNAL(signalAddChatSystemMessage(const QString&, const QString&, const Player*)),
		this, SLOT(slotAddChatSystemMessage(const QString&, const QString&, const Player*)));

 // for editor (new maps)
 connect(boGame, SIGNAL(signalEditorNewMap(const QByteArray&)),
		this, SLOT(slotEditorNewMap(const QByteArray&)));

 d->mQtGameView->bosonObjectCreated(boGame);
}

void BosonMainWidget::deleteBoson()
{
 if (!Boson::boson()) {
	return;
 }
 if (d->mQtGameView) {
	d->mQtGameView->bosonObjectAboutToBeDestroyed(boGame);
 }
 Boson::deleteBoson();
}

void BosonMainWidget::endGame()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(d->mGameEngine);
 d->mFPSCounter->reset();
 if (d->mQtGameView) {
	d->mQtGameView->quitGame();
 }

 d->mGameEngine->endGameAndDeleteBoson();
 delete d->mStarting;
 d->mStarting = 0;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonMainWidget::slotAddLocalPlayer()
{
 if (!boGame) {
	boError() << k_funcinfo << "NULL game object" << endl;
	return;
 }
 if (boGame->allPlayerCount() != 0) {
	boError() << k_funcinfo << "there are already players in the game!" << endl;
	return;
 }
 boDebug() << k_funcinfo << endl;
 Player* p = new Player;


 BosonLocalPlayerInput* input = new BosonLocalPlayerInput();
 p->addGameIO(input);
 if (!input->initializeIO()) {
	p->removeGameIO(input, true);

	boError() << k_funcinfo << "localplayer IO could not be initialized. fatal error - quitting" << endl;
	KMessageBox::sorry(this, i18n("internal error. could not initialize IO of local player. quitting now."));
	exit(1);
	return;
 }

 boGame->bosonAddPlayer(p);
 if (d->mQtStartup) {
	// this must be done _now_, we cannot delay it!
	// -> the newgame widget must know about the local player
	d->mQtStartup->setLocalPlayer(p);
 }
}

void BosonMainWidget::slotResetGame()
{
 reinitGame();
}

void BosonMainWidget::slotEditorNewMap(const QByteArray& buffer)
{
 if (boGame->gameStatus() != KGame::Init) {
	boError() << k_funcinfo << "game not in init status anymore" << endl;
	return;
 }
 if (!d->mStarting) {
	boError() << k_funcinfo << "NULL starting object" << endl;
	return;
 }
 boDebug() << k_funcinfo << endl;
 d->mStarting->setEditorMap(buffer);
}

void BosonMainWidget::slotShowSaveGamePage()
{
#warning TODO (Qt4 port)
#if 0
 // TODO: pause the game!
 if (!d->mStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 d->mStartup->slotSaveGame();
 raiseQtWidget(d->mQtStartup);
#endif
}

void BosonMainWidget::slotStartingFailed()
{
 boDebug() << k_funcinfo << endl;

 // TODO: display reason
 KMessageBox::sorry(this, i18n("Game starting failed"));
 slotGameOver();
}

void BosonMainWidget::slotChangeLocalPlayer(Player* p)
{
 changeLocalPlayer(p);
}

void BosonMainWidget::slotLoadExternalStuffFromXML(const QDomElement& root)
{
 d->mQtGameView->loadFromXML(root);
}

void BosonMainWidget::slotSaveExternalStuffAsXML(QDomElement& root)
{
 d->mQtGameView->saveAsXML(root);
}

void BosonMainWidget::slotAddChatSystemMessage(const QString& fromName, const QString& text, const Player* forPlayer)
{
 if (forPlayer && forPlayer != d->mLocalPlayer) {
	return;
 }
 d->mQtGameView->addChatMessage(i18n("--- %1: %2", fromName, text));
}

void BosonMainWidget::saveConfig()
{
 if (!boGame) {
	return;
 }
 if (!d->mLocalPlayer) {
	return;
 }
 if (boGame->gameMode()) {
	BosonConfig::saveLocalPlayerName(d->mLocalPlayer->name());
 } else {
 }
}

void BosonMainWidget::slotEndGame()
{
 BO_CHECK_NULL_RET(boGame);
 int answer;
 if (boGame->gameMode()) {
	answer = KMessageBox::warningYesNo(this, i18n("Are you sure you want to end this game?"),
		i18n("Are you sure?"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "ConfirmEndGame");
 } else {
	answer = KMessageBox::warningYesNo(this, i18n("Are you sure you want to end this editor session?"),
		i18n("Are you sure?"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "ConfirmEndGame");
 }
 if (answer != KMessageBox::Yes) {
	return;
 }
 slotGameOver();
}

bool BosonMainWidget::changeLocalPlayer(Player* p)
{
 boDebug() << k_funcinfo << p << endl;

 // AB: note that both, p == 0 AND p == currentplayer are valid and must be executed!
 if (!boGame) {
	boError() << k_funcinfo << "NULL game object" << endl;
	return false;
 }
 d->mLocalPlayer = p;
 if (d->mLocalPlayer) {
	d->mQtGameView->setLocalPlayerIO(d->mLocalPlayer->playerIO());
 } else {
	d->mQtGameView->setLocalPlayerIO(0);
 }

 // AB: note: the startup widgets don't need to know the new local player
 // -> they need the unique local player only (new game widget), which is set
 // right after construction of the player.
 return true;
}

void BosonMainWidget::reinitGame()
{
 endGame();

 delete d->mStarting;
 d->mStarting = new BosonStarting(this);
 connect(d->mStarting, SIGNAL(signalLoadingMaxDuration(unsigned int)),
		d->mQtStartup, SLOT(slotLoadingMaxDuration(unsigned int)));
 connect(d->mStarting, SIGNAL(signalLoadingTaskCompleted(unsigned int)),
		d->mQtStartup, SLOT(slotLoadingTaskCompleted(unsigned int)));
 connect(d->mStarting, SIGNAL(signalLoadingStartTask(const QString&)),
		d->mQtStartup, SLOT(slotLoadingStartTask(const QString&)));
 connect(d->mStarting, SIGNAL(signalLoadingStartSubTask(const QString&)),
		d->mQtStartup, SLOT(slotLoadingStartSubTask(const QString&)));
 connect(d->mStarting, SIGNAL(signalStartingFailed()),
		this, SLOT(slotStartingFailed()));

 BosonGameEngineStarting* gameStarting = new BosonGameEngineStarting(d->mStarting, d->mStarting);
 d->mStarting->addTaskCreator(gameStarting);
 BosonGUIStarting* guiStarting = new BosonGUIStarting(d->mStarting, d->mStarting);
 d->mStarting->addTaskCreator(guiStarting);
 BosonGameViewStarting* gameViewStarting = new BosonGameViewStarting(d->mStarting, d->mStarting);
 gameViewStarting->setGameView(d->mQtGameView);
 d->mStarting->addTaskCreator(gameViewStarting);

 initBoson();
}

void BosonMainWidget::slotGameOver()
{
 boDebug() << k_funcinfo << endl;
 endGame();

 if (d->mUfoWidget->ufoManager() && d->mUfoWidget->ufoManager()->actionCollection()) {
	d->mUfoWidget->ufoManager()->actionCollection()->createGUI();
 }

 // this also resets the game!
 // if you replace this by something else you must call slotResetGame()
 // manually!
 d->mQtStartup->slotShowWelcomeWidget();
 raiseQtWidget(d->mQtStartup);
}

void BosonMainWidget::slotCancelLoadSave()
{
#warning TODO (Qt4 port)
#if 0
 boDebug() << k_funcinfo << endl;
 if (boGame && boGame->gameStatus() != KGame::Init) {
	// called from a running game - but the player doesn't want to load/save a
	// game anymore
	raiseQtWidget(d->mQtGameView);
 } else {
	if (!d->mStartup) {
		boError() << k_funcinfo << "NULL startup widget??" << endl;
		return;
	}
	d->mStartup->slotShowWelcomeWidget();
 }
#endif
}

void BosonMainWidget::slotQuicksaveGame()
{
#warning TODO (Qt4 port)
#if 0
 BO_CHECK_NULL_RET(d->mLocalPlayer);
 BO_CHECK_NULL_RET(boGame);
 QString dir = BoUfoLoadSaveGameWidget::defaultDir();
 if (dir.isEmpty()) {
	boError() << k_funcinfo << "cannot find default quicksave directory" << endl;
	return;
 }
 d->mStartup->saveGame(dir + "quicksave.bsg", i18n("Quicksave"), true);

 BO_CHECK_NULL_RET(d->mLocalPlayer);
 BO_CHECK_NULL_RET(boGame);
 boGame->slotAddChatSystemMessage(i18n("Quicksaving succeeded"), d->mLocalPlayer);
#endif
}

void BosonMainWidget::slotQuickloadGame()
{
#warning TODO (Qt4 port)
#if 0
 BO_CHECK_NULL_RET(d->mLocalPlayer);
 QString dir = BoUfoLoadSaveGameWidget::defaultDir();
 if (dir.isEmpty()) {
	boError() << k_funcinfo << "cannot find default quicksave directory" << endl;
	return;
 }
 d->mStartup->loadGame(dir + "quicksave.bsg");

 // AB: note that loading works asynchonously. after returning here, we haven't
 // started loading yet.
#endif
}

void BosonMainWidget::slotStartNewGame()
{
 BO_CHECK_NULL_RET(boGame);
 if (!d->mStarting) {
	boError() << k_funcinfo << "NULL starting object!!" << endl;
	boGame->unlock();
	return;
 }
 if (!d->mQtStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	boGame->unlock();
	return;
 }
 boDebug(270) << k_funcinfo << endl;

 // AB: we need to have boGame->localPlayer() be valid and non-NULL for the
 // starup widgets (atm).
 // but for the actual starting process it is much cleaner, if the local player
 // is NULL and gets applied at the end only.
 slotChangeLocalPlayer(0);

 d->mQtStartup->showLoadingWidget();

 setFocusPolicy(Qt::StrongFocus);

 // this will take care of all data loading, like models, textures and so. this
 // also initializes the map and will send IdStartScenario - in short this will
 // start the game. Once it's done it'll send IdGameIsStarted (see
 // Boson::signalGameStarted())
 //
 // Note that this return immediately - the loading is started using a
 // QTimer::singleShot()
 d->mStarting->slotStartNewGameWithTimer();
}

// TODO: when this fails we should go back to the welcome widget!
void BosonMainWidget::slotGameStarted()
{
 boDebug(270) << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 if (boGame->gameStatus() != KGame::Run) {
	boWarning(270) << k_funcinfo << "not in Run status" << endl;
	return;
 }

 boDebug(270) << k_funcinfo << "init player" << endl;
 Player* localPlayer = 0;
 foreach (Player* p, boGame->allPlayerList()) {
	disconnect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*,KPlayer*)),
			this, 0);
 }
 foreach (Player* p, boGame->allPlayerList()) {
	if (!p->isVirtual()) {
		// a non-virtual player is a player that is running on this host
		// (either the local player or a computer player - never a
		// network player)
		if (p->hasRtti(BosonLocalPlayerInput::LocalPlayerInputRTTI)) {
			if (p->hasRtti(KGameIO::ComputerIO)) {
				boError(270) << k_funcinfo << "a player must not be both - local player and computer player!" << endl;
				return;
			}
			if (localPlayer) {
				if (!boGame->gameMode()) {
					// in editor mode all players are
					// actually "local".
					// we use the first one as "localPlayer".
					continue;
				}
				boError(270) << k_funcinfo << "two local players found?!" << endl;
				return;
			}
			localPlayer = p;
		}
	}
 }

 if (!localPlayer) {
	// this can happen in two cases
	// 1. editor mode
	// 2. loading a game

	if (!boGame->gameMode()) {
		// pick one player for editor mode
		// (it doesnt matter which)
		boDebug() << k_funcinfo << "picking a local player for editor mode" << endl;
		localPlayer = (Player*)boGame->gamePlayerList().first();
	} else {
		// we are loading a game
		// we are chosing the first player here - this is
		// valid as long as we don't support loading network games.
		// later we may allow selecting a player in the startup widgets
		//
		// if we ever do that, we should remove these line here!
		boDebug() << k_funcinfo << "picking a local player for loading games" << endl;
		localPlayer = (Player*)boGame->gamePlayerList().first();
	}
 }
 if (!localPlayer) {
	boError(270) << k_funcinfo << "NULL local player" << endl;
	return;
 }

 if (!changeLocalPlayer(localPlayer)) {
	boError(270) << k_funcinfo << "Changing to localplayer failed. Probably localPlayer could not be initialized." << endl;
	// TODO: see top of this method. return to welcome widget.
	return;
 }

 d->mQtGameView->setCanvas(boGame->canvasNonConst());

 raiseQtWidget(d->mQtGameView);
// setMinimumSize(BOSON_MINIMUM_WIDTH, BOSON_MINIMUM_HEIGHT);
 setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

 int progress = 0; // FIXME: wrong value!

 // we don't need these anymore. lets save the memory.
 d->mQtStartup->resetWidgets();

 // Center home base if new game was started. If game is loaded, camera was
 //  already loaded as well
 // TODO: load the camera settings from XML if present. if not present, center
 // hombase.
 d->mQtGameView->slotCenterHomeBase();

 d->mQtGameView->setFocus();

 if (boGame->gameMode()) {
	if (boGame->isAdmin()) {
		if (boGame->gameSpeed() == 0) {
			// don't do this if gameSpeed() != 0, as it was set already
			// (e.g. due to a savegame)
			boGame->slotSetGameSpeed(boConfig->intValue("GameSpeed"));
		}
	}
	boMusic->startLoop();
 }
}

#warning TODO: queryClose() replacement
// maybe use closeEvent() directly?
#if 0
bool BosonMainWidget::queryClose()
{
 boDebug() << k_funcinfo << endl;
 if (!boGame) {
	return true;
 }
 if (boGame->gameStatus() != KGame::Init) {
	int answer = KMessageBox::warningYesNo(this, i18n("Are you sure you want to quit Boson?\n"
			"This will end current game."), i18n("Are you sure?"), KStandardGuiItem::yes(),
			KStandardGuiItem::no(), "ConfirmQuitWhenGameRunning");
	if (answer == KMessageBox::Yes) {
		return true;
	}
	else if (answer == KMessageBox::No) {
		return false;
	}
 }
 return true;
}
#endif

void BosonMainWidget::slotShowLoadGamePage(KCmdLineArgs* args)
{
#warning TODO (Qt4 port)
#if 0
 // TODO: pause the game!
 if (!d->mStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 d->mStartup->slotLoadGame();
 raiseQtWidget(d->mQtStartup);
#endif
}

void BosonMainWidget::slotShowNewGamePage(KCmdLineArgs* args)
{
 if (!d->mQtStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 d->mQtStartup->slotNewSinglePlayerGame(args);
}

void BosonMainWidget::slotLoadFromLog(const QString& logFile)
{
#warning TODO (Qt4 port)
#if 0
 if (!d->mStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 BO_CHECK_NULL_RET(boGame);
 boDebug() << k_funcinfo << "trying to load from log file " << logFile << endl;
 d->mStartup->slotLoadFromLog(logFile);

 // AB: this is mostly a dummy call: we just need a non-null string to tell
 // Boson not to start the game timer.
 // -> the messages are actually loaded already at this point
 BO_CHECK_NULL_RET(d->mStarting);
 d->mStarting->setLoadFromLogFile(logFile);
#endif
}

void BosonMainWidget::slotShowStartEditorPage(KCmdLineArgs* args)
{
 BO_CHECK_NULL_RET(boGame);
 if (!d->mQtStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 d->mQtStartup->slotStartEditor(args);
}

void BosonMainWidget::slotSkipFrame()
{
// setAutoBufferSwap(false);
}

void BosonMainWidget::slotSetWidgetCursor(BosonCursor* c)
{
 BO_CHECK_NULL_RET(c);
 if (d->mQtWidgetStack->currentWidget() != d->mQtGameView) {
	return;
 }
 c->setWidgetCursor(this);
}

void BosonMainWidget::raiseWidget(BoUfoWidget* w)
{
 d->mWidgetStack->raiseStackWidget(w);
 if (w != d->mGameView) {
	unsetCursor();
 } else {
	// gameview widget is maximized by default
	boDebug() << k_funcinfo << "maximized" << endl;
#if 1
	// AB: by some reason the showMaximized() call is ignored, if we dont do
	// this
	// (at least with the current toolbar that want about 2100 pixels in
	// width)
	QDesktopWidget* desktop = QApplication::desktop();
	QRect r = desktop->availableGeometry(this);
	resize(r.width(), r.height());
#endif
	showMaximized();
 }
}

void BosonMainWidget::raiseQtWidget(QWidget* w)
{
 w->show();
 d->mQtWidgetStack->setCurrentWidget(w);
 if (w != d->mQtGameView) {
	unsetCursor();
 } else {
	// gameview widget is maximized by default
	boDebug() << k_funcinfo << "maximized" << endl;
#if 1
	// AB: by some reason the showMaximized() call is ignored, if we dont do
	// this
	// (at least with the current toolbar that want about 2100 pixels in
	// width)
	QDesktopWidget* desktop = QApplication::desktop();
	QRect r = desktop->availableGeometry(this);
	resize(r.width(), r.height());
#endif
	showMaximized();
 }
}

void BosonMainWidget::slotPreferences()
{
 OptionsDialog* dlg = new OptionsDialog(0);
 dlg->slotLoad();

 // FIXME: is this called if quit with "cancel" ?
 connect(dlg, SIGNAL(finished()), dlg, SLOT(deleteLater()));

 connect(dlg, SIGNAL(signalCursorChanged(int, const QString&)),
		d->mQtGameView, SLOT(slotChangeCursor(int, const QString&)));
 connect(dlg, SIGNAL(signalApply()),
		this, SLOT(slotPreferencesApply()));
// connect(dlg, SIGNAL(signalFontChanged(const BoFontInfo&)),
//		displayManager(), SLOT(slotChangeFont(const BoFontInfo&)));

 dlg->show();

}
void BosonMainWidget::slotPreferencesApply()
{
 // apply all options from boConfig to boson, that need to be applied. all
 // options that are stored in boConfig only don't need to be touched.
 // AB: cursor is still a special case and not handled here.
 boDebug() << k_funcinfo << endl;
 d->mQtGameView->setToolTipCreator(boConfig->intValue("ToolTipCreator"));
 d->mQtGameView->setToolTipUpdatePeriod(boConfig->intValue("ToolTipUpdatePeriod"));
 slotSetUpdateInterval(boConfig->uintValue("GLUpdateInterval"));
 if (boGame) {
	if (boGame->gameSpeed() != boConfig->intValue("GameSpeed")) {
		boGame->slotSetGameSpeed(boConfig->intValue("GameSpeed"));
	}
 }
}

void BosonMainWidget::slotDebugUfoWidgets()
{
#if 0
 KDialog* dialog = new KDialog();
 dialog->setWindowTitle(KDialog::makeStandardCaption(i18n("Debug Ufo Widgets")));
 dialog->setButtons(KDialog::Close);
 dialog->setDefaultButton(KDialog::Close);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(delayedDestruct()));
 BoUfoDebugWidget* debug = new BoUfoDebugWidget(dialog);
 dialog->setMainWidget(debug);
// Q3VBoxLayout* l = new Q3VBoxLayout(dialog->plainPage());
// l->addWidget(debug);

 debug->setBoUfoManager(d->mUfoWidget->ufoManager());

 dialog->show();
#else
 boError() << k_funcinfo << "BoUfoDebugWidget has been disabled due to the Qt4 port" << endl;
#endif
}

void BosonMainWidget::slotDebugTextures()
{
 BosonDebugTextures* dialog = new BosonDebugTextures(this);
 dialog->slotUpdate();
 dialog->show();
}

void BosonMainWidget::slotDebugModels()
{
 BosonDebugModels* dialog = new BosonDebugModels(this);
 dialog->slotUpdate();
 dialog->show();
}

void BosonMainWidget::slotStartupPreferredSizeChanged()
{
 BO_CHECK_NULL_RET(d->mUfoWidget->ufoManager());
 BO_CHECK_NULL_RET(d->mUfoWidget->ufoManager()->rootPaneWidget());
 BO_CHECK_NULL_RET(d->mWidgetStack);
 if (d->mQtWidgetStack->currentWidget() == d->mQtGameView) {
	return;
 }
 int w = d->mUfoWidget->ufoManager()->rootPaneWidget()->preferredWidth();
 int h = d->mUfoWidget->ufoManager()->rootPaneWidget()->preferredHeight();

 // make sure we don't exceed the maximum desktop size
 QDesktopWidget* desktop = QApplication::desktop();
 QRect r = desktop->availableGeometry(this);
 w = qMin(w, r.width());
 h = qMin(h, r.height());

 if (width() >= w && height() >= h) {
	return;
 }
 w = qMax(w, width());
 h = qMax(h, height());
 w = qMin(w, r.width());
 h = qMin(h, r.height());
 resize(w, h);
 resizeGL(w, h);
}

