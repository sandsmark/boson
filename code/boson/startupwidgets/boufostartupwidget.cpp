/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "boufostartupwidget.h"
#include "boufostartupwidget.moc"

#include "../../bomemory/bodummymemory.h"
#include "boufoloadingwidget.h"
#include "boufonewgamewidget.h"
#include "boufoloadfromlogwidget.h"
#include "boufonetworkoptionswidget.h"
#include "boufostarteditorwidget.h"
#include "boufoloadsavegamewidget.h"
#include "bosonloadsavegamehandler.h"
#include "welcomewidget.h"
#include "bosonstartupnetwork.h"
#include "bodebug.h"
#include "../gameengine/boson.h"
#include "../defines.h"

#include <kmainwindow.h> // AB: urghs
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <qimage.h>
#include <q3popupmenu.h>
#include <qcursor.h>
#include <qpointer.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QEvent>

#include <stdlib.h>

class BoUfoStartupWidgetPrivate
{
public:
	BoUfoStartupWidgetPrivate()
	{
		mWidgetStack = 0;

		mNetworkInterface = 0;

		mLocalPlayer = 0;

		mLoadSaveGameHandler = 0;
	}

	BoUfoWidgetStack* mWidgetStack;

	BosonStartupNetwork* mNetworkInterface;

	QPointer<Player> mLocalPlayer;

	bool mSinglePlayer;
	BosonLoadSaveGameHandler* mLoadSaveGameHandler;
};

BoUfoStartupWidget::BoUfoStartupWidget() : BoUfoWidget()
{
 init();
}

void BoUfoStartupWidget::init()
{
 d = new BoUfoStartupWidgetPrivate;
 d->mWidgetStack = new BoUfoWidgetStack();

 connect(d->mWidgetStack, SIGNAL(signalVisibleWidgetChanged(BoUfoWidget*)),
		this, SLOT(slotVisibleWidgetChanged(BoUfoWidget*)));

 QImage backgroundImage(locate("data", "boson/pics/boson-startup-bg.png"));
 QImage logoImage(locate("data", "boson/pics/boson-startup-logo.png"));

 setBackgroundImage(backgroundImage);
 setOpaque(true);

 setLayoutClass(BoUfoWidget::UVBoxLayout);
 BoUfoLabel* logo = new BoUfoLabel();
 logo->setHorizontalAlignment(BoUfoWidget::Qt::AlignHCenter);
 logo->setIcon(logoImage);

 addSpacing(25);
 addWidget(logo);
 addWidget(d->mWidgetStack);
 d->mWidgetStack->setStretch(1);

 d->mNetworkInterface = new BosonStartupNetwork(this);
 d->mNetworkInterface->setGame(boGame);

 d->mLoadSaveGameHandler = new BosonLoadSaveGameHandler(d->mNetworkInterface, this);
 connect(d->mLoadSaveGameHandler, SIGNAL(signalGameOver()),
		this, SIGNAL(signalGameOver()));
 connect(d->mLoadSaveGameHandler, SIGNAL(signalCancelLoadSave()),
		this, SIGNAL(signalCancelLoadSave()));

 installEventFilter(this); // for the popup menu

 d->mSinglePlayer = false;
}

BoUfoStartupWidget::~BoUfoStartupWidget()
{
 delete d->mLoadSaveGameHandler;
 delete d;
}

void BoUfoStartupWidget::setLocalPlayer(Player* p)
{
 d->mLocalPlayer = p;
 BoUfoNewGameWidget* w = (BoUfoNewGameWidget*)d->mWidgetStack->stackWidget(IdNewGame);
 if (w) {
	w->setLocalPlayer(d->mLocalPlayer);
 }
}

void BoUfoStartupWidget::loadGame(const QString& fileName)
{
 d->mLoadSaveGameHandler->slotLoadGame(fileName);
}

void BoUfoStartupWidget::slotLoadGame()
{
 showWidget(IdLoadSaveGame);
 BoUfoLoadSaveGameWidget* loadSave = (BoUfoLoadSaveGameWidget*)d->mWidgetStack->stackWidget(IdLoadSaveGame);
 if (!loadSave) {
	boError() << k_funcinfo << "load/save widget hasn't been initialized?!" << endl;
	return;
 }
 loadSave->setSaveMode(false);
 loadSave->updateGames();
}

void BoUfoStartupWidget::saveGame(const QString& fileName, const QString& description, bool forceOverwrite)
{
 d->mLoadSaveGameHandler->slotSaveGame(fileName, description, forceOverwrite);
}

void BoUfoStartupWidget::slotSaveGame()
{
 // TODO pause game!
 showWidget(IdLoadSaveGame);
 BoUfoLoadSaveGameWidget* loadSave = (BoUfoLoadSaveGameWidget*)d->mWidgetStack->stackWidget(IdLoadSaveGame);
 if (!loadSave) {
	boError() << k_funcinfo << "load/save widget hasn't been initialized?!" << endl;
	return;
 }
 loadSave->setSaveMode(true);
 loadSave->updateGames();
}

void BoUfoStartupWidget::slotLoadFromLog(const QString& fileName)
{
 d->mSinglePlayer = true;

 showWidget(IdLoadFromLog);

 BoUfoLoadFromLogWidget* w = (BoUfoLoadFromLogWidget*)d->mWidgetStack->stackWidget(IdLoadFromLog);
 if (!w) {
	BO_NULL_ERROR(w);
	return;
 }
 if (!w->loadFromLog(fileName)) {
	KMessageBox::sorry(0, i18n("Could not load from log file %1").arg(fileName));
	return;
 }
}

void BoUfoStartupWidget::slotNewSinglePlayerGame(KCmdLineArgs* args)
{
 d->mSinglePlayer = true;

 newGame(args);
 BoUfoNewGameWidget* w = (BoUfoNewGameWidget*)d->mWidgetStack->stackWidget(IdNewGame);
 if (w) {
	if (boGame->isAdmin() && boGame->gamePlayerCount() <= 1) {
		if (!args || !args->isSet("start") && !args->isSet("computer")) {
			w->addAIPlayer();
		}
	}
 }
}

void BoUfoStartupWidget::slotNewMultiPlayerGame(KCmdLineArgs* args)
{
 d->mSinglePlayer = false;

 showWidget(IdNetwork);
}

void BoUfoStartupWidget::newGame(KCmdLineArgs* args)
{
 showWidget(IdNewGame);
 if (!args) {
	return;
 }
 BoUfoNewGameWidget* w = (BoUfoNewGameWidget*)d->mWidgetStack->stackWidget(IdNewGame);
 if (!w) {
	boError() << k_funcinfo << "Oops - NULL newgame widget" << endl;
	return;
 }
 // here we can check for some things like --playfield and call the functions
 // e.g.:
 if (args && args->isSet("playfield")) {
	QString identifier = args->getOption("playfield");
	if (identifier.right(4) != QString::fromLatin1(".bpf")) {
		identifier = identifier + QString::fromLatin1(".bpf");
	}
	networkInterface()->sendChangePlayField(identifier);
 }
 if (args && args->isSet("computer")) {
	QString count = args->getOption("computer");
	bool ok = true;
	int c = count.toInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "computer player count was not a valid number" << endl;
		return;
	}
	if (c >= BOSON_MAX_PLAYERS) {
		boWarning() << k_funcinfo << "Cannot add " << c << " players" << endl;
		boDebug() << k_funcinfo << "changing to " << BOSON_MAX_PLAYERS - 1<< endl;
		c = BOSON_MAX_PLAYERS - 1;
	}

	for (int i = 0; i < c; i++) {
		// note: we don't have to care about which playfield was loaded,
		// as we can *always* add unlimited players. player number is
		// checked when starting game only
		w->addAIPlayer();
	}
 }


 if (args && args->isSet("start")) {
	w->slotStartGame();
 }
}

void BoUfoStartupWidget::slotStartEditor(KCmdLineArgs* args)
{
 showWidget(IdStartEditor);
 if (!args) {
	return;
 }
 // here we can check for some things like --playfield and call the functions
 // e.g.:
 if (args->isSet("playfield")) {
	QString identifier = args->getOption("playfield");
	if (identifier.right(4) != QString::fromLatin1(".bpf")) {
		identifier = identifier + QString::fromLatin1(".bpf");
	}
	networkInterface()->sendChangePlayField(identifier);
 }


 if (args->isSet("start")) {
	networkInterface()->sendStartGameClicked();
 }
}

void BoUfoStartupWidget::showWidget(WidgetId widgetId)
{
 initWidget(widgetId);

 if (!d->mWidgetStack->stackWidget((int)widgetId)) {
	boError() << k_funcinfo << "NULL widget " << widgetId << endl;
	return;
 }

 d->mWidgetStack->raiseStackWidget((int)widgetId);
}

void BoUfoStartupWidget::initWidget(WidgetId widgetId)
{
 if (d->mWidgetStack->stackWidget((int)widgetId)) {
	// already initialized
	return;
 }
 BoUfoWidget* w = 0;
 switch (widgetId) {
	case IdWelcome:
	{
		BoUfoWelcomeWidget* welcome = new BoUfoWelcomeWidget();
		connect(welcome, SIGNAL(signalQuit()), this, SIGNAL(signalQuit()));
		connect(welcome, SIGNAL(signalNewSPGame()), this, SLOT(slotNewSinglePlayerGame()));
		connect(welcome, SIGNAL(signalNewMPGame()), this, SLOT(slotNewMultiPlayerGame()));
		connect(welcome, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
		connect(welcome, SIGNAL(signalStartEditor()), this, SLOT(slotStartEditor()));
		connect(welcome, SIGNAL(signalPreferences()), this, SIGNAL(signalPreferences()));
		w = welcome;
		break;
	}
	case IdLoading:
	{
		QColor defaultColor = BoUfoLabel::defaultForegroundColor();
		BoUfoLabel::setDefaultForegroundColor(Qt::white);
		BoUfoLoadingWidget* loadingWidget = new BoUfoLoadingWidget();
		BoUfoLabel::setDefaultForegroundColor(defaultColor);
		connect(loadingWidget, SIGNAL(signalUpdateGL()),
				this, SIGNAL(signalUpdateGL()));
		w = loadingWidget;
		break;
	}
	case IdLoadSaveGame:
	{
		QColor defaultColor = BoUfoLabel::defaultForegroundColor();
		BoUfoLabel::setDefaultForegroundColor(Qt::white);
		BoUfoLoadSaveGameWidget* loadSaveWidget = new BoUfoLoadSaveGameWidget();
		BoUfoLabel::setDefaultForegroundColor(defaultColor);
		loadSaveWidget->setSuffix(QString::fromLatin1("bsg"));
		connect(loadSaveWidget, SIGNAL(signalLoadGame(const QString&)),
				d->mLoadSaveGameHandler, SLOT(slotLoadGame(const QString&)));
		connect(loadSaveWidget, SIGNAL(signalSaveGame(const QString&, const QString&)),
				d->mLoadSaveGameHandler, SLOT(slotSaveGame(const QString&, const QString&)));
		connect(loadSaveWidget, SIGNAL(signalCancel()),
				this, SIGNAL(signalCancelLoadSave()));
		w = loadSaveWidget;
		break;
	}
	case IdNewGame:
	{
		QColor defaultColor = BoUfoLabel::defaultForegroundColor();
		BoUfoLabel::setDefaultForegroundColor(Qt::white);
		BoUfoNewGameWidget* startGame = new BoUfoNewGameWidget(networkInterface());
		BoUfoLabel::setDefaultForegroundColor(defaultColor);
		connect(startGame, SIGNAL(signalCancelled()),
				this, SLOT(slotNewGameCancelled()));
		connect(startGame, SIGNAL(signalKickedOut()),
				this, SLOT(slotKickedOut()));

		// AB: this does nothing, as d->mLocalPlayer is NULL. but maybe
		// that will change.
		startGame->setLocalPlayer(d->mLocalPlayer);

		w = startGame;
		break;
	}
	case IdLoadFromLog:
	{
		QColor defaultColor = BoUfoLabel::defaultForegroundColor();
		BoUfoLabel::setDefaultForegroundColor(Qt::white);
		BoUfoLoadFromLogWidget* load = new BoUfoLoadFromLogWidget(networkInterface());
		BoUfoLabel::setDefaultForegroundColor(defaultColor);
		connect(load, SIGNAL(signalCancelled()),
				this, SLOT(slotShowWelcomeWidget()));

		w = load;
		break;
	}
	case IdNetwork:
	{
		// AB: the network widget requires the newgame widget to be
		// present already
		initWidget(IdNewGame);

		QColor defaultColor = BoUfoLabel::defaultForegroundColor();
		BoUfoLabel::setDefaultForegroundColor(Qt::white);
		BoUfoNetworkOptionsWidget* networkOptions = new BoUfoNetworkOptionsWidget();
		BoUfoLabel::setDefaultForegroundColor(defaultColor);
		connect(networkOptions, SIGNAL(signalOkClicked()),
				this, SLOT(slotNetworkOptionsOk()));
		connect(networkOptions, SIGNAL(signalCancelled()),
				this, SLOT(slotNetworkOptionsCancel()));
		connect(networkOptions, SIGNAL(signalOfferingConnections()),
				this, SLOT(slotOfferingConnections()));
		connect(networkOptions, SIGNAL(signalConnectingToServer()),
				this, SLOT(slotConnectingToServer()));
		connect(networkOptions, SIGNAL(signalConnectedToServer()),
				this, SLOT(slotConnectedToServer()));

		w = networkOptions;
		break;
	}
	case IdStartEditor:
	{
		QColor defaultColor = BoUfoLabel::defaultForegroundColor();
		BoUfoLabel::setDefaultForegroundColor(Qt::white);
		BoUfoStartEditorWidget* startEditor = new BoUfoStartEditorWidget(networkInterface());
		BoUfoLabel::setDefaultForegroundColor(defaultColor);
		connect(startEditor, SIGNAL(signalCancelled()),
				this, SLOT(slotShowWelcomeWidget()));

		w = startEditor;
		break;
	}
	case IdLast:
		boError() << k_funcinfo << "invalid id " << (int)widgetId << endl;
		return;
 }
 if (!w) {
	boError() << k_funcinfo << "NULL widget" << endl;
	return;
 }
 w->installEventFilter(this); // for the popup menu

 d->mWidgetStack->addWidget(w);
 d->mWidgetStack->insertStackWidget(w, (int)widgetId);

 if (widgetId == IdNewGame) {
	// the new game widget requires a local player. this gets added
	// here.
	// note that the new player ends up in boGame->playerList() once we
	// return to the event loop only, NOT immediately
	emit signalAddLocalPlayer();

	// should be done _after_ adding the local player
	((BoUfoNewGameWidget*)w)->initInitialPlayField();
 } else if (widgetId == IdLoadFromLog) {
	emit signalAddLocalPlayer();
 }
}

BoUfoLoadingWidget* BoUfoStartupWidget::loadingWidget() const
{
 return (BoUfoLoadingWidget*)d->mWidgetStack->stackWidget(IdLoading);
}

void BoUfoStartupWidget::showLoadingWidget()
{
 showWidget(IdLoading);
}

void BoUfoStartupWidget::slotLoadingMaxDuration(unsigned int maxDuration)
{
 if (loadingWidget()) {
	loadingWidget()->setMaxDuration(maxDuration);
 }
}

void BoUfoStartupWidget::slotLoadingTaskCompleted(unsigned int duration)
{
 if (loadingWidget()) {
	loadingWidget()->setDuration(duration);
 }
}

void BoUfoStartupWidget::slotLoadingStartTask(const QString& text)
{
 if (loadingWidget()) {
	loadingWidget()->setCurrentTask(text);
 }
}

void BoUfoStartupWidget::slotLoadingStartSubTask(const QString& text)
{
 if (loadingWidget()) {
	loadingWidget()->setCurrentSubTask(text);
 }
}

void BoUfoStartupWidget::slotNetworkOptionsOk()
{
 newGame(0);
}

void BoUfoStartupWidget::slotNetworkOptionsCancel()
{
 slotShowWelcomeWidget();
 boGame->disconnect();
}

bool BoUfoStartupWidget::eventFilter(QObject* o, QEvent* e)
{
 switch (e->type()) {
	case QEvent::MouseButtonPress:
		if (((QMouseEvent*)e)->button() == Qt::RightButton) {
			// this is pretty ugly, but im afraid its necessary.
			KMainWindow* main = (KMainWindow*)qApp->mainWidget();
			if (!main) {
				boError() << k_funcinfo << "NULL main widget?!" << endl;
				return true;
			}
			if (!main->factory()) {
				boError() << k_funcinfo << "NULL factory (this is expected until the startup widgets are ported to libufo)" << endl;
				return true;
			}
			Q3PopupMenu* p = (Q3PopupMenu*)main->factory()->container("welcomepopup", main);
			if (p) {
				p->popup(QCursor::pos());
			}
			return true;
		}
		break;
	case IdLast:
	default:
		break;
 }
 return BoUfoWidget::eventFilter(o, e);
}

// this shows the welcome widget and emits signalResetGame(), i.e. resets the
// game!
void BoUfoStartupWidget::slotShowWelcomeWidget()
{
 boDebug() << k_funcinfo << endl;
 showWidget(IdWelcome);

 // reset the game now:
 // first remove all widgets except the welcome widget
 BoUfoWidget* w = d->mWidgetStack->stackWidget((int)IdWelcome);
 if (w) {
	d->mWidgetStack->removeStackWidget(w);
 }
 resetWidgets();
 if (w) {
	d->mWidgetStack->insertStackWidget(w, (int)IdWelcome);
 }
 d->mWidgetStack->raiseStackWidget((int)IdWelcome);
 emit signalResetGame();
 networkInterface()->setGame(boGame);

 // the startup widget gets hidden when game is started, so when we want to show
 // the welcome widget we also need to show the startup widget
 show();
}

void BoUfoStartupWidget::slotNewGameCancelled()
{
 if (d->mSinglePlayer) {
	slotShowWelcomeWidget();
 } else {
	showWidget(IdNetwork);
 }
}

void BoUfoStartupWidget::resetWidgets()
{
 for (int i = 0; i < IdLast; i++) {
	removeWidget((WidgetId)i);
 }
}

void BoUfoStartupWidget::removeWidget(WidgetId widgetId)
{
 BoUfoWidget* w = d->mWidgetStack->stackWidget((int)widgetId);
 if (w) {
	d->mWidgetStack->removeStackWidget(w);
	d->mWidgetStack->removeWidget(w);
 }
}

BosonStartupNetwork* BoUfoStartupWidget::networkInterface() const
{
 return d->mNetworkInterface;
}

void BoUfoStartupWidget::slotOfferingConnections()
{
 BoUfoWidget* w = d->mWidgetStack->stackWidget(IdNewGame);
 if (!w) {
	boError() << k_funcinfo << "No new game widget!!!" << endl;
	return;
 }
 ((BoUfoNewGameWidget*)w)->slotOfferingConnections();
}

void BoUfoStartupWidget::slotConnectingToServer()
{
 BoUfoWidget* w = d->mWidgetStack->stackWidget(IdNewGame);
 if (!w) {
	boError() << k_funcinfo << "No new game widget!!!" << endl;
	return;
 }
 ((BoUfoNewGameWidget*)w)->slotConnectingToServer();
}

void BoUfoStartupWidget::slotConnectedToServer()
{
 BoUfoWidget* w = d->mWidgetStack->stackWidget(IdNewGame);
 if (!w) {
	boError() << k_funcinfo << "No new game widget!!!" << endl;
	return;
 }
 ((BoUfoNewGameWidget*)w)->slotConnectedToServer();
}


void BoUfoStartupWidget::slotKickedOut()
{
 boDebug() << k_funcinfo << "disconnect" << endl;
 boGame->disconnect();
 boDebug() << k_funcinfo << "disconnect DONE" << endl;
 boDebug() << k_funcinfo << "re-adding local player" << endl;
 emit signalAddLocalPlayer();
}

void BoUfoStartupWidget::slotVisibleWidgetChanged(BoUfoWidget* w)
{
 Q_UNUSED(w);
 emit signalPreferredSizeChanged();
}

