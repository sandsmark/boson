/*
    This file is part of the Boson game
    Copyright (C) 2002-2008 Andreas Beckermann (b_mann@gmx.de)
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

#include "boqtstartupwidget.h"
#include "boqtstartupwidget.moc"

#include "../../bomemory/bodummymemory.h"
#if 0
#include "boufoloadfromlogwidget.h"
#include "boufonetworkoptionswidget.h"
#include "boufostarteditorwidget.h"
#include "boufoloadsavegamewidget.h"
#endif
#include "bowelcomewidget.h"
#include "boloadingwidget.h"
#include "bonewgamewidget.h"
#include "bosonloadsavegamehandler.h"
#include "bosonstartupnetwork.h"
#include "bodebug.h"
#include "../gameengine/boson.h"
#include "../defines.h"
#include "../gameengine/player.h"

#include <kmainwindow.h> // AB: urghs
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <qimage.h>
#include <q3popupmenu.h>
#include <qcursor.h>
#include <qpointer.h>
#include <QMouseEvent>
#include <QEvent>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>

#include <stdlib.h>

class BoQtStartupWidgetPrivate
{
public:
	BoQtStartupWidgetPrivate()
	{
		mWidgetStack = 0;

		mNetworkInterface = 0;

		mLocalPlayer = 0;

		mLoadSaveGameHandler = 0;

		mWelcomeWidgetIndex = -1;
		mNewGameWidgetIndex = -1;
		mLoadSaveGameWidgetIndex = -1;
		mStartEditorWidgetIndex = -1;
		mLoadingWidgetIndex = -1;
		mNetworkWidgetIndex = -1;
		mLoadFromLogWidgetIndex = -1;
	}

	QStackedWidget* mWidgetStack;

	BosonStartupNetwork* mNetworkInterface;

	QPointer<Player> mLocalPlayer;

	bool mSinglePlayer;
	BosonLoadSaveGameHandler* mLoadSaveGameHandler;

	int mWelcomeWidgetIndex;
	int mNewGameWidgetIndex;
	int mLoadSaveGameWidgetIndex;
	int mStartEditorWidgetIndex;
	int mLoadingWidgetIndex;
	int mNetworkWidgetIndex;
	int mLoadFromLogWidgetIndex;
};

BoQtStartupWidget::BoQtStartupWidget(QWidget* parent) : QWidget(parent)
{
 init();
}

void BoQtStartupWidget::init()
{
 d = new BoQtStartupWidgetPrivate;
 d->mWidgetStack = new QStackedWidget();

 connect(d->mWidgetStack, SIGNAL(currentChanged(int)),
		this, SLOT(slotVisibleWidgetChanged(int)));

 QPalette myPalette = palette();
 QBrush myBackgroundBrush = myPalette.brush(QPalette::Window);
 QImage backgroundImage(KStandardDirs::locate("data", "boson/pics/boson-startup-bg.png"));
 myBackgroundBrush.setTextureImage(backgroundImage);
 myPalette.setBrush(QPalette::Window, myBackgroundBrush);
 setPalette(myPalette);

// setOpaque(true);

 QLabel* logo = new QLabel(this);
 logo->setAlignment(Qt::AlignCenter);

 QPalette logoPalette = logo->palette();
 QBrush logoBackgroundBrush = logoPalette.brush(QPalette::Window);
 QImage logoImage(KStandardDirs::locate("data", "boson/pics/boson-startup-logo.png"));
 logoBackgroundBrush.setTextureImage(logoImage);
 logoPalette.setBrush(QPalette::Window, logoBackgroundBrush);
 logo->setPalette(logoPalette);

 QVBoxLayout* layout = new QVBoxLayout(this);
 layout->addSpacing(25);
 layout->addWidget(logo);
 layout->addWidget(d->mWidgetStack, 1);

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

BoQtStartupWidget::~BoQtStartupWidget()
{
 delete d->mLoadSaveGameHandler;
 delete d;
}

int BoQtStartupWidget::widgetId2Index(WidgetId id) const
{
 int index = -1;
 switch (id) {
	case IdWelcome:
		index = d->mWelcomeWidgetIndex;
		break;
	case IdNewGame:
		index = d->mNewGameWidgetIndex;
		break;
	case IdLoadSaveGame:
		index = d->mLoadSaveGameWidgetIndex;
		break;
	case IdStartEditor:
		index = d->mStartEditorWidgetIndex;
		break;
	case IdLoading:
		index = d->mLoadingWidgetIndex;
		break;
	case IdNetwork:
		index = d->mNetworkWidgetIndex;
		break;
	case IdLoadFromLog:
		index = d->mLoadFromLogWidgetIndex;
		break;
	case IdLast:
		boError() << "IdLast is not a valid Id";
		break;
 }
 return index;
}

void BoQtStartupWidget::setLocalPlayer(Player* p)
{
 d->mLocalPlayer = p;
 if (d->mNewGameWidgetIndex >= 0) {
	BoNewGameWidget* w = qobject_cast<BoNewGameWidget*>(d->mWidgetStack->widget(d->mNewGameWidgetIndex));
	if (w) {
		w->setLocalPlayer(d->mLocalPlayer);
	}
 }
}

void BoQtStartupWidget::loadGame(const QString& fileName)
{
 d->mLoadSaveGameHandler->slotLoadGame(fileName);
}

void BoQtStartupWidget::slotLoadGame()
{
#warning TODO
#if 0
 showWidget(IdLoadSaveGame);
 BoUfoLoadSaveGameWidget* loadSave = (BoUfoLoadSaveGameWidget*)d->mWidgetStack->stackWidget(IdLoadSaveGame);
 if (!loadSave) {
	boError() << k_funcinfo << "load/save widget hasn't been initialized?!" << endl;
	return;
 }
 loadSave->setSaveMode(false);
 loadSave->updateGames();
#endif
}

void BoQtStartupWidget::saveGame(const QString& fileName, const QString& description, bool forceOverwrite)
{
 d->mLoadSaveGameHandler->slotSaveGame(fileName, description, forceOverwrite);
}

void BoQtStartupWidget::slotSaveGame()
{
#warning TODO
#if 0
 // TODO pause game!
 showWidget(IdLoadSaveGame);
 BoUfoLoadSaveGameWidget* loadSave = (BoUfoLoadSaveGameWidget*)d->mWidgetStack->stackWidget(IdLoadSaveGame);
 if (!loadSave) {
	boError() << k_funcinfo << "load/save widget hasn't been initialized?!" << endl;
	return;
 }
 loadSave->setSaveMode(true);
 loadSave->updateGames();
#endif
}

void BoQtStartupWidget::slotLoadFromLog(const QString& fileName)
{
#warning TODO
#if 0
 d->mSinglePlayer = true;

 showWidget(IdLoadFromLog);

 BoUfoLoadFromLogWidget* w = (BoUfoLoadFromLogWidget*)d->mWidgetStack->stackWidget(IdLoadFromLog);
 if (!w) {
	BO_NULL_ERROR(w);
	return;
 }
 if (!w->loadFromLog(fileName)) {
	KMessageBox::sorry(0, i18n("Could not load from log file %1", fileName));
	return;
 }
#endif
}

void BoQtStartupWidget::slotNewSinglePlayerGame(KCmdLineArgs* args)
{
 d->mSinglePlayer = true;

 newGame(args);
 BoNewGameWidget* w = 0;
 if (d->mNewGameWidgetIndex >= 0) {
	w = qobject_cast<BoNewGameWidget*>(d->mWidgetStack->widget(d->mNewGameWidgetIndex));
 }
 if (w) {
	if (boGame->isAdmin() && boGame->gamePlayerCount() <= 1) {
		if (!args || !args->isSet("start") && !args->isSet("computer")) {
			w->addAIPlayer();
		}
	}
 }
}

void BoQtStartupWidget::slotNewMultiPlayerGame(KCmdLineArgs* args)
{
 d->mSinglePlayer = false;

 showWidget(IdNetwork);
}

void BoQtStartupWidget::newGame(KCmdLineArgs* args)
{
 showWidget(IdNewGame);
 if (!args) {
	return;
 }
 BoNewGameWidget* w = qobject_cast<BoNewGameWidget*>(d->mWidgetStack->widget(d->mNewGameWidgetIndex));
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

void BoQtStartupWidget::slotStartEditor(KCmdLineArgs* args)
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

void BoQtStartupWidget::showWidget(WidgetId widgetId)
{
 initWidget(widgetId);

 int index = widgetId2Index(widgetId);
 if (index < 0) {
	boError() << "unknown widget ID" << widgetId;
	return;
 }

 d->mWidgetStack->setCurrentIndex(index);
}

void BoQtStartupWidget::initWidget(WidgetId widgetId)
{
 if (widgetId2Index(widgetId) >= 0) {
	// already initialized
	return;
 }
 QWidget* w = 0;
 switch (widgetId) {
	case IdWelcome:
	{
		BoWelcomeWidget* welcome = new BoWelcomeWidget(d->mWidgetStack);
		connect(welcome, SIGNAL(signalQuit()), this, SIGNAL(signalQuit()));
		connect(welcome, SIGNAL(signalNewSPGame()), this, SLOT(slotNewSinglePlayerGame()));
		connect(welcome, SIGNAL(signalNewMPGame()), this, SLOT(slotNewMultiPlayerGame()));
		connect(welcome, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
		connect(welcome, SIGNAL(signalStartEditor()), this, SLOT(slotStartEditor()));
		connect(welcome, SIGNAL(signalPreferences()), this, SIGNAL(signalPreferences()));
		w = welcome;
		d->mWelcomeWidgetIndex = d->mWidgetStack->addWidget(w);
		break;
	}
	case IdLoading:
	{
		BoLoadingWidget* loadingWidget = new BoLoadingWidget(d->mWidgetStack);
		connect(loadingWidget, SIGNAL(signalUpdateGL()),
				this, SIGNAL(signalUpdateGL()));
		w = loadingWidget;
		d->mLoadingWidgetIndex = d->mWidgetStack->addWidget(w);
		break;
	}
	case IdLoadSaveGame:
	{
#if 0
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
		d->mLoadSaveGameWidgetIndex = d->mWidgetStack->addWidget(w);
#endif
		break;
	}
	case IdNewGame:
	{
		BoNewGameWidget* startGame = new BoNewGameWidget(networkInterface(), d->mWidgetStack);
		connect(startGame, SIGNAL(signalCancelled()),
				this, SLOT(slotNewGameCancelled()));
		connect(startGame, SIGNAL(signalKickedOut()),
				this, SLOT(slotKickedOut()));

		// AB: this does nothing, as d->mLocalPlayer is NULL. but maybe
		// that will change.
		startGame->setLocalPlayer(d->mLocalPlayer);

		w = startGame;
		d->mNewGameWidgetIndex = d->mWidgetStack->addWidget(w);
		break;
	}
	case IdLoadFromLog:
	{
#if 0
		QColor defaultColor = BoUfoLabel::defaultForegroundColor();
		BoUfoLabel::setDefaultForegroundColor(Qt::white);
		BoUfoLoadFromLogWidget* load = new BoUfoLoadFromLogWidget(networkInterface());
		BoUfoLabel::setDefaultForegroundColor(defaultColor);
		connect(load, SIGNAL(signalCancelled()),
				this, SLOT(slotShowWelcomeWidget()));

		w = load;
		d->mLoadFromLogWidgetIndex = d->mWidgetStack->addWidget(w);
#endif
		break;
	}
	case IdNetwork:
	{
#if 0
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
		d->mNetworkWidgetIndex = d->mWidgetStack->addWidget(w);
#endif
		break;
	}
	case IdStartEditor:
	{
#if 0
		QColor defaultColor = BoUfoLabel::defaultForegroundColor();
		BoUfoLabel::setDefaultForegroundColor(Qt::white);
		BoUfoStartEditorWidget* startEditor = new BoUfoStartEditorWidget(networkInterface());
		BoUfoLabel::setDefaultForegroundColor(defaultColor);
		connect(startEditor, SIGNAL(signalCancelled()),
				this, SLOT(slotShowWelcomeWidget()));

		w = startEditor;
		d->mStartEditorWidgetIndex = d->mWidgetStack->addWidget(w);
#endif
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


 if (widgetId == IdNewGame) {
	// the new game widget requires a local player. this gets added
	// here.
	// note that the new player ends up in boGame->playerList() once we
	// return to the event loop only, NOT immediately
	emit signalAddLocalPlayer();

	// should be done _after_ adding the local player
	qobject_cast<BoNewGameWidget*>(w)->initInitialPlayField();
 } else if (widgetId == IdLoadFromLog) {
	emit signalAddLocalPlayer();
 }
}

BoLoadingWidget* BoQtStartupWidget::loadingWidget() const
{
 if (widgetId2Index(IdLoading) < 0) {
	return 0;
 }
 return qobject_cast<BoLoadingWidget*>(d->mWidgetStack->widget(widgetId2Index(IdLoading)));
}

void BoQtStartupWidget::showLoadingWidget()
{
 showWidget(IdLoading);
}

void BoQtStartupWidget::slotLoadingMaxDuration(unsigned int maxDuration)
{
 if (loadingWidget()) {
	loadingWidget()->setMaxDuration(maxDuration);
 }
}

void BoQtStartupWidget::slotLoadingTaskCompleted(unsigned int duration)
{
 if (loadingWidget()) {
	loadingWidget()->setDuration(duration);
 }
}

void BoQtStartupWidget::slotLoadingStartTask(const QString& text)
{
 if (loadingWidget()) {
	loadingWidget()->setCurrentTask(text);
 }
}

void BoQtStartupWidget::slotLoadingStartSubTask(const QString& text)
{
 if (loadingWidget()) {
	loadingWidget()->setCurrentSubTask(text);
 }
}

void BoQtStartupWidget::slotNetworkOptionsOk()
{
 newGame(0);
}

void BoQtStartupWidget::slotNetworkOptionsCancel()
{
 slotShowWelcomeWidget();
 boGame->disconnect();
}

bool BoQtStartupWidget::eventFilter(QObject* o, QEvent* e)
{
#if 0
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
#endif
 return QWidget::eventFilter(o, e);
}

// this shows the welcome widget and emits signalResetGame(), i.e. resets the
// game!
void BoQtStartupWidget::slotShowWelcomeWidget()
{
 boDebug() << k_funcinfo << endl;
 showWidget(IdWelcome);

 // reset the game now:
 // first remove all widgets except the welcome widget
 QWidget* w = d->mWidgetStack->widget(d->mWelcomeWidgetIndex);
 if (w) {
	d->mWidgetStack->removeWidget(w);
 }
 resetWidgets();
 if (w) {
	d->mWelcomeWidgetIndex = d->mWidgetStack->addWidget(w);
 }
 d->mWidgetStack->setCurrentIndex(d->mWelcomeWidgetIndex);
 emit signalResetGame();
 networkInterface()->setGame(boGame);

 // the startup widget gets hidden when game is started, so when we want to show
 // the welcome widget we also need to show the startup widget
 show();
}

void BoQtStartupWidget::slotNewGameCancelled()
{
 if (d->mSinglePlayer) {
	slotShowWelcomeWidget();
 } else {
	showWidget(IdNetwork);
 }
}

void BoQtStartupWidget::resetWidgets()
{
 for (int i = 0; i < IdLast; i++) {
	removeWidget((WidgetId)i);
 }
}

void BoQtStartupWidget::removeWidget(WidgetId widgetId)
{
 if (widgetId2Index(widgetId) < 0) {
	return;
 }
 QWidget* w = d->mWidgetStack->widget(widgetId2Index(widgetId));
 if (w) {
	d->mWidgetStack->removeWidget(w);
 }
}

BosonStartupNetwork* BoQtStartupWidget::networkInterface() const
{
 return d->mNetworkInterface;
}

void BoQtStartupWidget::slotOfferingConnections()
{
 if (d->mNewGameWidgetIndex < 0) {
	boError() << k_funcinfo << "No new game widget!!!";
	return;
 }
 QWidget* w = d->mWidgetStack->widget(d->mNewGameWidgetIndex);
 if (!w) {
	boError() << k_funcinfo << "No new game widget!!!";
	return;
 }
 qobject_cast<BoNewGameWidget*>(w)->slotOfferingConnections();
}

void BoQtStartupWidget::slotConnectingToServer()
{
 if (d->mNewGameWidgetIndex < 0) {
	boError() << k_funcinfo << "No new game widget!!!";
	return;
 }
 QWidget* w = d->mWidgetStack->widget(d->mNewGameWidgetIndex);
 if (!w) {
	boError() << k_funcinfo << "No new game widget!!!";
	return;
 }
 qobject_cast<BoNewGameWidget*>(w)->slotConnectingToServer();
}

void BoQtStartupWidget::slotConnectedToServer()
{
 if (d->mNewGameWidgetIndex < 0) {
	boError() << k_funcinfo << "No new game widget!!!";
	return;
 }
 QWidget* w = d->mWidgetStack->widget(d->mNewGameWidgetIndex);
 if (!w) {
	boError() << k_funcinfo << "No new game widget!!!";
	return;
 }
 qobject_cast<BoNewGameWidget*>(w)->slotConnectedToServer();
}


void BoQtStartupWidget::slotKickedOut()
{
 boDebug() << k_funcinfo << "disconnect" << endl;
 boGame->disconnect();
 boDebug() << k_funcinfo << "disconnect DONE" << endl;
 boDebug() << k_funcinfo << "re-adding local player" << endl;
 emit signalAddLocalPlayer();
}

void BoQtStartupWidget::slotVisibleWidgetChanged(int index)
{
 Q_UNUSED(index);
 emit signalPreferredSizeChanged();
}

