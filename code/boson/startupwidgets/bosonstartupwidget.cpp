/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonstarteditorwidget.h"
#include "bosonnetworkoptionswidget.h"
#include "kloadsavegamewidget.h"
#include "bodebug.h"
#include "../boson.h"

#include <kmainwindow.h> // AB: urghs
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <qwidgetstack.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qobjectlist.h>

#include <stdlib.h>

class BosonStartupWidget::BosonStartupWidgetPrivate
{
public:
	BosonStartupWidgetPrivate()
	{
		mWidgetStack = 0;

		mBackgroundPix = 0;
		mLogoPix = 0;
	}

	QWidgetStack* mWidgetStack;

	QPixmap* mBackgroundPix;
	QPixmap* mLogoPix;
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
 if (d->mBackgroundPix->isNull() || d->mLogoPix->isNull()) {
        KMessageBox::error(this, i18n("You seem not to have Boson data files installed!\n"
			"Please install data package of Boson and restart Boson."), i18n("Data files not found!"));
	exit(1);  // Evil, but if we'd call qApp->exit(1); then we would return to event loop
	return;
 }

 setPaletteBackgroundPixmap(*d->mBackgroundPix);

 QLabel* logo = new QLabel(this, "bosonlogo");
 logo->setBackgroundOrigin(WindowOrigin);
 logo->setPixmap(*d->mLogoPix);

 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->addSpacing(20); // FIXME hardcoded
 topLayout->addWidget(logo, 0, AlignHCenter);
 topLayout->addSpacing(10); // FIXME hardcoded
 topLayout->addWidget(d->mWidgetStack, 1);
}

BosonStartupWidget::~BosonStartupWidget()
{
 delete d->mLogoPix;
 delete d->mBackgroundPix;
 delete d;
}

void BosonStartupWidget::slotNewGame()
{
 showWidget(IdNewGame);
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

void BosonStartupWidget::slotStartEditor()
{
 showWidget(IdStartEditor);
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

		// If we're loading game, we don't know number of players here
		// If game is loaded, we disable progressbar in loading widget, but still set
		// steps and progress to make code less messy (it's better than having
		// if (!mLoading) { ... }  everywhere)
		loadingWidget->setTotalSteps(3400, boGame->playerCount());
		loadingWidget->setProgress(0);

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
		BosonNewGameWidget* newGame = new BosonNewGameWidget(this);
		connect(newGame, SIGNAL(signalCancelled()),
				this, SLOT(slotShowWelcomeWidget()));
		connect(newGame, SIGNAL(signalShowNetworkOptions()),
				this, SLOT(slotShowNetworkOptions()));

		// the new game widget requires a local player. this gets added
		// here.
		// note that the player will get added once we return to the
		// event loop only, NOT immediately!
		emit signalAddLocalPlayer();

		w = newGame;
		break;
	}
	case IdNetwork:
	{
		BosonNetworkOptionsWidget* networkOptions = new BosonNetworkOptionsWidget(this);
		connect(networkOptions, SIGNAL(signalOkClicked()),
				this, SLOT(slotHideNetworkOptions()));

		w = networkOptions;
		break;
	}
	case IdStartEditor:
	{
		BosonStartEditorWidget* startEditor = new BosonStartEditorWidget(this);
		connect(startEditor, SIGNAL(signalCancelled()),
				this, SLOT(slotShowWelcomeWidget()));
		connect(startEditor, SIGNAL(signalSetLocalPlayer(Player*)),
				this, SIGNAL(signalSetLocalPlayer(Player*)));

		// AB: ensure that the local player actually gets assigned!

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
}

BosonLoadingWidget* BosonStartupWidget::loadingWidget() const
{
 return (BosonLoadingWidget*)d->mWidgetStack->widget(IdLoading);
}

void BosonStartupWidget::showLoadingWidget()
{
 showWidget(IdLoading);
}

void BosonStartupWidget::setLocalPlayer(Player* player)
{
 mPlayer = player;
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

void BosonStartupWidget::slotLoadingProgress(int progress)
{
 if (loadingWidget()) {
	loadingWidget()->setProgress(progress);
 }
}

void BosonStartupWidget::slotLoadingTileProgress(int base, int tiles)
{
 if (loadingWidget()) {
	loadingWidget()->setTileProgress(base, tiles);
 }
}

void BosonStartupWidget::slotLoadingUnitProgress(int progress, int current, int total)
{
 if (loadingWidget()) {
	loadingWidget()->setUnitProgress(progress, current, total);
 }
}

void BosonStartupWidget::slotShowNetworkOptions()
{
 showWidget(IdNetwork);
}

void BosonStartupWidget::slotHideNetworkOptions()
{
 BosonNewGameWidget* newGame = (BosonNewGameWidget*)d->mWidgetStack->widget(IdNewGame);
 if (!newGame) {
	// strange as network widget gets shown for newgame widget only
	boError() << k_funcinfo << "NULL new game widget??" << endl;
	return;
 }
 removeWidget(IdNetwork);
 newGame->slotSetAdmin(boGame->isAdmin());
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

void BosonStartupWidget::slotShowWelcomeWidget()
{
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

