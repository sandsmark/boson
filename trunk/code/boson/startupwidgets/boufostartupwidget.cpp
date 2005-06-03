/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "boufostartupwidget.h"
#include "boufostartupwidget.moc"

#include "boufoloadingwidget.h"
#include "boufonewgamewidget.h"
#include "boufonetworkoptionswidget.h"
#include "boufostarteditorwidget.h"
#include "welcomewidget.h"
#include "bosonstartupnetwork.h"
#include "bodebug.h"
#include "../boson.h"
#include "../defines.h"

#include <kmainwindow.h> // AB: urghs
#include <kstandarddirs.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <qpixmap.h>
#include <qimage.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qguardedptr.h>

#include <stdlib.h>

#define LOADSAVE_WIDGET 0

class BoUfoStartupWidgetPrivate
{
public:
	BoUfoStartupWidgetPrivate()
	{
		mWidgetStack = 0;

		mNetworkInterface = 0;

		mLocalPlayer = 0;
	}

	BoUfoWidgetStack* mWidgetStack;

	BosonStartupNetwork* mNetworkInterface;

	QGuardedPtr<Player> mLocalPlayer;
};

BoUfoStartupWidget::BoUfoStartupWidget() : BoUfoWidget()
{
 init();
}

void BoUfoStartupWidget::init()
{
 d = new BoUfoStartupWidgetPrivate;
 d->mWidgetStack = new BoUfoWidgetStack();

 QImage backgroundImage(locate("data", "boson/pics/boson-startup-bg.png"));
 QImage logoImage(locate("data", "boson/pics/boson-startup-logo.png"));

 setBackgroundImage(backgroundImage);
 setOpaque(true);

 BoUfoLabel* logo = new BoUfoLabel();
 logo->setHorizontalAlignment(BoUfoWidget::AlignHCenter);
 logo->setIcon(logoImage);

 setLayoutClass(BoUfoWidget::UVBoxLayout);
 addSpacing(5);
 addWidget(logo);
 addWidget(d->mWidgetStack);
 d->mWidgetStack->setStretch(1);

 d->mNetworkInterface = new BosonStartupNetwork(this);
 d->mNetworkInterface->setGame(boGame);

 installEventFilter(this); // for the popup menu
}

BoUfoStartupWidget::~BoUfoStartupWidget()
{
 delete d;
}

void BoUfoStartupWidget::setLocalPlayer(Player* p)
{
 d->mLocalPlayer = p;
 BoUfoNewGameWidget* w = (BoUfoNewGameWidget*)d->mWidgetStack->widget(IdNewGame);
 if (w) {
	w->setLocalPlayer(d->mLocalPlayer);
 }
}

void BoUfoStartupWidget::slotLoadGame()
{
 showWidget(IdLoadSaveGame);
#if LOADSAVE_WIDGET
 KLoadSaveGameWidget* loadSave = (KLoadSaveGameWidget*)d->mWidgetStack->widget(IdLoadSaveGame);
 if (!loadSave) {
	boError() << k_funcinfo << "load/save widget hasn't been initialized?!" << endl;
	return;
 }
 loadSave->setSaveMode(false);
 loadSave->updateGames();
#endif
}

void BoUfoStartupWidget::slotSaveGame()
{
 // TODO pause game!
 showWidget(IdLoadSaveGame);
#if LOADSAVE_WIDGET
 KLoadSaveGameWidget* loadSave = (KLoadSaveGameWidget*)d->mWidgetStack->widget(IdLoadSaveGame);
 if (!loadSave) {
	boError() << k_funcinfo << "load/save widget hasn't been initialized?!" << endl;
	return;
 }
 loadSave->setSaveMode(true);
 loadSave->updateGames();
#endif
}

void BoUfoStartupWidget::slotNewGame(KCmdLineArgs* args)
{
 showWidget(IdNewGame);
 if (!args) {
	return;
 }
 BoUfoNewGameWidget* w = (BoUfoNewGameWidget*)d->mWidgetStack->widget(IdNewGame);
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

 if (!d->mWidgetStack->widget((int)widgetId)) {
	boError() << k_funcinfo << "NULL widget " << widgetId << endl;
	return;
 }

 d->mWidgetStack->raiseStackWidget((int)widgetId);
}

void BoUfoStartupWidget::initWidget(WidgetId widgetId)
{
 if (d->mWidgetStack->widget((int)widgetId)) {
	// already initialized
	return;
 }
 BoUfoWidget* w = 0;
 switch (widgetId) {
	case IdWelcome:
	{
		BoUfoWelcomeWidget* welcome = new BoUfoWelcomeWidget();
		connect(welcome, SIGNAL(signalQuit()), this, SIGNAL(signalQuit()));
		connect(welcome, SIGNAL(signalNewGame()), this, SLOT(slotNewGame()));
		connect(welcome, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
		connect(welcome, SIGNAL(signalStartEditor()), this, SLOT(slotStartEditor()));
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
#if LOADSAVE_WIDGET
		QColor defaultColor = BoUfoLabel::defaultForegroundColor();
		BoUfoLabel::setDefaultForegroundColor(Qt::white);
		KLoadSaveGameWidget* loadSaveWidget = new KLoadSaveGameWidget(d->mWidgetStack);
		BoUfoLabel::setDefaultForegroundColor(defaultColor);
		loadSaveWidget->setSuffix(QString::fromLatin1("bsg"));
		connect(loadSaveWidget, SIGNAL(signalLoadGame(const QString&)),
				this, SIGNAL(signalLoadGame(const QString&)));
		connect(loadSaveWidget, SIGNAL(signalSaveGame(const QString&, const QString&)),
				this, SIGNAL(signalSaveGame(const QString&, const QString&)));
		connect(loadSaveWidget, SIGNAL(signalCancel()),
				this, SIGNAL(signalCancelLoadSave()));
		w = loadSaveWidget;
#endif
		break;
	}
	case IdNewGame:
	{
		QColor defaultColor = BoUfoLabel::defaultForegroundColor();
		BoUfoLabel::setDefaultForegroundColor(Qt::white);
		BoUfoNewGameWidget* startGame = new BoUfoNewGameWidget(networkInterface());
		BoUfoLabel::setDefaultForegroundColor(defaultColor);
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
		QColor defaultColor = BoUfoLabel::defaultForegroundColor();
		BoUfoLabel::setDefaultForegroundColor(Qt::white);
		BoUfoNetworkOptionsWidget* networkOptions = new BoUfoNetworkOptionsWidget();
		BoUfoLabel::setDefaultForegroundColor(defaultColor);
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
 }
}

BoUfoLoadingWidget* BoUfoStartupWidget::loadingWidget() const
{
 return (BoUfoLoadingWidget*)d->mWidgetStack->widget(IdLoading);
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

void BoUfoStartupWidget::slotShowNetworkOptions()
{
 showWidget(IdNetwork);
}

void BoUfoStartupWidget::slotHideNetworkOptions()
{
 BoUfoNewGameWidget* startGame = (BoUfoNewGameWidget*)d->mWidgetStack->widget(IdNewGame);
 if (!startGame) {
	// strange as network widget gets shown for newgame widget only
	boError() << k_funcinfo << "NULL new game widget??" << endl;
	return;
 }
 startGame->slotSetAdmin(boGame->isAdmin());
 showWidget(IdNewGame);
}

bool BoUfoStartupWidget::eventFilter(QObject* o, QEvent* e)
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
 BoUfoWidget* w = d->mWidgetStack->widget((int)IdWelcome);
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

void BoUfoStartupWidget::resetWidgets()
{
 for (int i = 0; i < IdLast; i++) {
	removeWidget((WidgetId)i);
 }
}

void BoUfoStartupWidget::removeWidget(WidgetId widgetId)
{
 BoUfoWidget* w = d->mWidgetStack->widget((int)widgetId);
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
 BoUfoWidget* w = d->mWidgetStack->widget(IdNewGame);
 if (!w) {
	boError() << k_funcinfo << "No new game widget!!!" << endl;
	return;
 }
 ((BoUfoNewGameWidget*)w)->slotOfferingConnections();
}

void BoUfoStartupWidget::slotConnectingToServer()
{
 BoUfoWidget* w = d->mWidgetStack->widget(IdNewGame);
 if (!w) {
	boError() << k_funcinfo << "No new game widget!!!" << endl;
	return;
 }
 ((BoUfoNewGameWidget*)w)->slotConnectingToServer();
}

void BoUfoStartupWidget::slotConnectedToServer()
{
 BoUfoWidget* w = d->mWidgetStack->widget(IdNewGame);
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

