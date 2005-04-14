/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonstartupwidget.h"
#include "bosonstartupwidget.moc"

#include "bosonwelcomewidget.h"
#include "bosonloadingwidget.h"
#include "bosonnewgamewidget.h"
#include "bosonneweditorwidget.h"
#include "bosonnetworkoptionswidget.h"
#include "kloadsavegamewidget.h"
#include "bosonstartupnetwork.h"
#include "bodebug.h"
#include "../boson.h"
#include "../defines.h"

#include <kmainwindow.h> // AB: urghs
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <qwidgetstack.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qobjectlist.h>
#include <qguardedptr.h>

#include <stdlib.h>

class BosonStartupWidget::BosonStartupWidgetPrivate
{
public:
	BosonStartupWidgetPrivate()
	{
		mWidgetStack = 0;

		mBackgroundPix = 0;
		mLogoPix = 0;

		mNetworkInterface = 0;

		mLocalPlayer = 0;
	}

	QWidgetStack* mWidgetStack;

	QPixmap* mBackgroundPix;
	QPixmap* mLogoPix;

	BosonStartupNetwork* mNetworkInterface;

	QGuardedPtr<Player> mLocalPlayer;
};

BosonStartupWidget::BosonStartupWidget(QWidget* parent) : QWidget(parent)
{
 init();
}

void BosonStartupWidget::init()
{
 d = new BosonStartupWidgetPrivate;
 d->mWidgetStack = new QWidgetStack(this);

 d->mBackgroundPix = new QPixmap(locate("data", "boson/pics/boson-startup-bg.png"));
 d->mLogoPix = new QPixmap(locate("data", "boson/pics/boson-startup-logo.png"));

 setBackgroundMode(FixedPixmap);
 setPaletteBackgroundPixmap(*d->mBackgroundPix);
 setBackgroundOrigin(WindowOrigin);

 d->mWidgetStack->setBackgroundMode(FixedPixmap);
 d->mWidgetStack->setPaletteBackgroundPixmap(*d->mBackgroundPix);
 d->mWidgetStack->setBackgroundOrigin(WindowOrigin);

 QLabel* logo = new QLabel(this, "bosonlogo");
 logo->setBackgroundOrigin(WindowOrigin);
 logo->setPixmap(*d->mLogoPix);

 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->addSpacing(5); // FIXME hardcoded
 topLayout->addWidget(logo, 0, AlignHCenter);
 topLayout->addSpacing(0); // FIXME hardcoded
 topLayout->addWidget(d->mWidgetStack, 1);

 d->mNetworkInterface = new BosonStartupNetwork(this);
 d->mNetworkInterface->setGame(boGame);

 installEventFilter(this); // for the popup menu
}

BosonStartupWidget::~BosonStartupWidget()
{
 delete d->mLogoPix;
 delete d->mBackgroundPix;
 delete d;
}

void BosonStartupWidget::setLocalPlayer(Player* p)
{
 d->mLocalPlayer = p;
 BosonNewGameWidget* w = (BosonNewGameWidget*)d->mWidgetStack->widget(IdNewGame);
 if (w) {
	w->setLocalPlayer(d->mLocalPlayer);
 }
}

void BosonStartupWidget::slotLoadGame()
{
 showWidget(IdLoadSaveGame);
 KLoadSaveGameWidget* loadSave = (KLoadSaveGameWidget*)d->mWidgetStack->widget(IdLoadSaveGame);
 if (!loadSave) {
	boError() << k_funcinfo << "load/save widget hasn't been initialized?!" << endl;
	return;
 }
 loadSave->setSaveMode(false);
 loadSave->updateGames();
}

void BosonStartupWidget::slotSaveGame()
{
 // TODO pause game!
 showWidget(IdLoadSaveGame);
 KLoadSaveGameWidget* loadSave = (KLoadSaveGameWidget*)d->mWidgetStack->widget(IdLoadSaveGame);
 if (!loadSave) {
	boError() << k_funcinfo << "load/save widget hasn't been initialized?!" << endl;
	return;
 }
 loadSave->setSaveMode(true);
 loadSave->updateGames();
}

void BosonStartupWidget::slotNewGame(KCmdLineArgs* args)
{
 showWidget(IdNewGame);
 if (!args) {
	return;
 }
 BosonNewGameWidget* w = (BosonNewGameWidget*)d->mWidgetStack->widget(IdNewGame);
 if (!w) {
	boError() << k_funcinfo << "Oops - NULL newgame widget" << endl;
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
 if (args->isSet("computer")) {
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


 if (args->isSet("start")) {
	w->slotStartGame();
 }
}

void BosonStartupWidget::slotStartEditor(KCmdLineArgs* args)
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

void BosonStartupWidget::showWidget(WidgetId widgetId)
{
 initWidget(widgetId);

 if (!d->mWidgetStack->widget((int)widgetId)) {
	boError() << k_funcinfo << "NULL widget " << widgetId << endl;
	return;
 }

 // TODO: see Top::raiseWidget() ?
 d->mWidgetStack->raiseWidget((int)widgetId);
 d->mWidgetStack->widget((int)widgetId)->show();
}

void BosonStartupWidget::initWidget(WidgetId widgetId)
{
 if (d->mWidgetStack->widget((int)widgetId)) {
	// already initialized
	return;
 }
 QWidget* w = 0;
 switch (widgetId) {
	case IdWelcome:
	{
		BosonWelcomeWidget* welcome = new BosonWelcomeWidget(d->mWidgetStack);
		connect(welcome, SIGNAL(signalQuit()), this, SIGNAL(signalQuit()));
		connect(welcome, SIGNAL(signalNewGame()), this, SLOT(slotNewGame()));
		connect(welcome, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
		connect(welcome, SIGNAL(signalStartEditor()), this, SLOT(slotStartEditor()));
		w = welcome;
		break;
	}
	case IdLoading:
	{
		BosonLoadingWidget* loadingWidget = new BosonLoadingWidget(d->mWidgetStack);

		w = loadingWidget;
		break;
	}
	case IdLoadSaveGame:
	{
		KLoadSaveGameWidget* loadSaveWidget = new KLoadSaveGameWidget(d->mWidgetStack);
		loadSaveWidget->setSuffix(QString::fromLatin1("bsg"));
		connect(loadSaveWidget, SIGNAL(signalLoadGame(const QString&)),
				this, SIGNAL(signalLoadGame(const QString&)));
		connect(loadSaveWidget, SIGNAL(signalSaveGame(const QString&, const QString&)),
				this, SIGNAL(signalSaveGame(const QString&, const QString&)));
		connect(loadSaveWidget, SIGNAL(signalCancel()),
				this, SIGNAL(signalCancelLoadSave()));
		w = loadSaveWidget;
		break;
	}
	case IdNewGame:
	{
		BosonNewGameWidget* startGame = new BosonNewGameWidget(networkInterface(), this);
		connect(startGame, SIGNAL(signalCancelled()),
				this, SLOT(slotShowWelcomeWidget()));
		connect(startGame, SIGNAL(signalShowNetworkOptions()),
				this, SLOT(slotShowNetworkOptions()));
		connect(startGame, SIGNAL(signalKickedOut()),
				this, SLOT(slotKickedOut()));

		// AB: this does nothing, as d->mLocalPlayer is NULL. but maybe
		// that will change.
		startGame->setLocalPlayer(d->mLocalPlayer);

		w = startGame;
		break;
	}
	case IdNetwork:
	{
		BosonNetworkOptionsWidget* networkOptions = new BosonNetworkOptionsWidget(this);
		connect(networkOptions, SIGNAL(signalOkClicked()),
				this, SLOT(slotHideNetworkOptions()));
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
		BosonNewEditorWidget* startEditor = new BosonNewEditorWidget(networkInterface(), this);
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
 initBackgroundOrigin(w);

 d->mWidgetStack->addWidget(w, (int)widgetId);

 if (widgetId == IdNewGame) {
	// the new game widget requires a local player. this gets added
	// here.
	// note that the new player ends up in boGame->playerList() once we
	// return to the event loop only, NOT immediately
	emit signalAddLocalPlayer();
 }
}

BosonLoadingWidget* BosonStartupWidget::loadingWidget() const
{
 return (BosonLoadingWidget*)d->mWidgetStack->widget(IdLoading);
}

void BosonStartupWidget::showLoadingWidget()
{
 showWidget(IdLoading);
}

void BosonStartupWidget::slotLoadingType(int type)
{
 if (loadingWidget()) {
	loadingWidget()->setLoading((BosonLoadingWidget::LoadingType)type);
 }
}

void BosonStartupWidget::slotLoadingShowProgressBar(bool s)
{
 if (loadingWidget()) {
	loadingWidget()->showProgressBar(s);
 }
}

void BosonStartupWidget::slotLoadingReset()
{
 if (loadingWidget()) {
	loadingWidget()->resetProgress();
 }
}

void BosonStartupWidget::slotLoadingSetAdmin(bool isAdmin)
{
 if (loadingWidget()) {
	loadingWidget()->setAdmin(isAdmin);
 }
}

void BosonStartupWidget::slotLoadingSetLoading(bool isLoading)
{
 if (loadingWidget()) {
	loadingWidget()->setLoading(isLoading);
 }
}

void BosonStartupWidget::slotLoadingPlayersCount(int count)
{
 if (loadingWidget()) {
	loadingWidget()->setTotalPlayers(count);
 }
}

void BosonStartupWidget::slotLoadingPlayer(int current)
{
 if (loadingWidget()) {
	loadingWidget()->setCurrentPlayer(current);
 }
}

void BosonStartupWidget::slotLoadingUnitsCount(int count)
{
 if (loadingWidget()) {
	loadingWidget()->setTotalUnits(count);
 }
}

void BosonStartupWidget::slotLoadingUnit(int current)
{
 if (loadingWidget()) {
	loadingWidget()->setCurrentUnit(current);
 }
}

void BosonStartupWidget::slotShowNetworkOptions()
{
 showWidget(IdNetwork);
}

void BosonStartupWidget::slotHideNetworkOptions()
{
 BosonNewGameWidget* startGame = (BosonNewGameWidget*)d->mWidgetStack->widget(IdNewGame);
 if (!startGame) {
	// strange as network widget gets shown for newgame widget only
	boError() << k_funcinfo << "NULL new game widget??" << endl;
	return;
 }
 removeWidget(IdNetwork);
 startGame->slotSetAdmin(boGame->isAdmin());
 showWidget(IdNewGame);
}

bool BosonStartupWidget::eventFilter(QObject* o, QEvent* e)
{
 switch (e->type()) {
	case QEvent::MouseButtonPress:
		if (((QMouseEvent*)e)->button() == RightButton) {
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
			QPopupMenu* p = (QPopupMenu*)main->factory()->container("welcomepopup", main);
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
 return QWidget::eventFilter(o, e);
}

void BosonStartupWidget::initBackgroundOrigin(QWidget* widget)
{
 // warning! hack!
 // we need to change the backgroundorigin of all child widgets to
 // WindowOrigin. is there a better way?
 // update: probably not, since we also need to change the grand-childs (thats
 // why we need *Window*Origin - ParentOrigin doesnt work with grandchilds).
 QObjectList* l = widget->queryList("QWidget", 0, true, true);
 QObjectListIt it(*l);
 QWidget* w;
 boDebug() << k_funcinfo << endl;
 while ((w = (QWidget*)it.current()) != 0) {
	w->setBackgroundOrigin(WindowOrigin);
	++it;
 }
 delete l;
 // (hack end)
}

// this shows the welcome widget and emits signalResetGame(), i.e. resets the
// game!
void BosonStartupWidget::slotShowWelcomeWidget()
{
 boDebug() << k_funcinfo << endl;
 showWidget(IdWelcome);

 // reset the game now:
 // first remove all widgets except the welcome widget
 QWidget* w = d->mWidgetStack->widget((int)IdWelcome);
 if (w) {
	d->mWidgetStack->removeWidget(w);
 }
 resetWidgets();
 if (w) {
	d->mWidgetStack->addWidget(w, (int)IdWelcome);
 }
 d->mWidgetStack->raiseWidget((int)IdWelcome);
 emit signalResetGame();
 networkInterface()->setGame(boGame);

 // the startup widget gets hidden when game is started, so when we want to show
 // the welcome widget we also need to show the startup widget
 show();
}

void BosonStartupWidget::resetWidgets()
{
 for (int i = 0; i < IdLast; i++) {
	removeWidget((WidgetId)i);
 }
}

void BosonStartupWidget::removeWidget(WidgetId widgetId)
{
 QWidget* w = d->mWidgetStack->widget((int)widgetId);
 if (w) {
	d->mWidgetStack->removeWidget(w);
	delete w;
 }
}

BosonStartupNetwork* BosonStartupWidget::networkInterface() const
{
 return d->mNetworkInterface;
}

void BosonStartupWidget::slotOfferingConnections()
{
 QWidget* w = d->mWidgetStack->widget(IdNewGame);
 if (!w) {
	boError() << k_funcinfo << "No new game widget!!!" << endl;
	return;
 }
 ((BosonNewGameWidget*)w)->slotOfferingConnections();
}

void BosonStartupWidget::slotConnectingToServer()
{
 QWidget* w = d->mWidgetStack->widget(IdNewGame);
 if (!w) {
	boError() << k_funcinfo << "No new game widget!!!" << endl;
	return;
 }
 ((BosonNewGameWidget*)w)->slotConnectingToServer();
}

void BosonStartupWidget::slotConnectedToServer()
{
 QWidget* w = d->mWidgetStack->widget(IdNewGame);
 if (!w) {
	boError() << k_funcinfo << "No new game widget!!!" << endl;
	return;
 }
 ((BosonNewGameWidget*)w)->slotConnectedToServer();
}


void BosonStartupWidget::slotKickedOut()
{
 boDebug() << k_funcinfo << "disconnect" << endl;
 boGame->disconnect();
 boDebug() << k_funcinfo << "disconnect DONE" << endl;
 boDebug() << k_funcinfo << "re-adding local player" << endl;
 emit signalAddLocalPlayer();
}

