/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include "top.h"

#include "bosonwidget.h"

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kconfig.h>

#include <kedittoolbar.h>

#include <kstatusbar.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kstdgameaction.h>
#include <kdebug.h>

#include <qwmatrix.h>

#include "top.moc"

class Top::TopPrivate
{
public:
	TopPrivate()
	{
		mBosonWidget = 0;
		mToolbarAction = 0;
		mStatusbarAction = 0;
		mZoomAction = 0;
	}

	BosonWidget* mBosonWidget;
	KToggleAction* mToolbarAction;
	KToggleAction* mStatusbarAction;
	KSelectAction* mZoomAction;
};

Top::Top()
    : KMainWindow( 0 )
{
 d = new TopPrivate;

 d->mBosonWidget = new BosonWidget(this);
 d->mBosonWidget->addGameCommandFrame();

 // tell the KMainWindow that this is indeed the main widget
 setCentralWidget(d->mBosonWidget);

 // then, setup our actions
 setupActions();

 // and a status bar
 setupStatusBar();

 showMaximized();

 d->mBosonWidget->slotNewGame(); // adds a local player, too
}

Top::~Top()
{
 d->mBosonWidget->saveConfig();
 delete d;
}

void Top::setupActions()
{
 KStdGameAction::gameNew(d->mBosonWidget, SLOT(slotNewGame()), actionCollection());
// KStdGameAction::save(this, SLOT(fileSave()), actionCollection());
// KStdGameAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
 KStdGameAction::quit(kapp, SLOT(quit()), actionCollection());
 KStdGameAction::end(d->mBosonWidget, SLOT(slotEndGame()), actionCollection());
// (void)new KAction(i18n("Connect To localhost"), 0, d->mBosonWidget, SLOT(slotConnect()), actionCollection(), "game_connect");

// Debug - no i18n!
 (void)new KAction("Debug", QKeySequence(), d->mBosonWidget, SLOT(slotDebug()), actionCollection(), "game_debug");

 d->mToolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
 d->mStatusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

 KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
 KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
 KStdAction::preferences(d->mBosonWidget, SLOT(slotGamePreferences()), actionCollection()); // FIXME: for game only - not editor!

 d->mZoomAction = new KSelectAction(i18n("&Zoom"), QKeySequence(), actionCollection(), "options_zoom");
 connect(d->mZoomAction, SIGNAL(activated(int)), 
		this, SLOT(slotZoom(int)));
 QStringList items;
 items.append(QString::number(50));
 items.append(QString::number(100));
 items.append(QString::number(150));
 d->mZoomAction->setItems(items);

 createGUI();
}

void Top::setupStatusBar()
{
 statusBar()->show();
}

void Top::saveProperties(KConfig *config)
{
 // the 'config' object points to the session managed
 // config file.  anything you write here will be available
 // later when this app is restored
 if (!config) {
	return;
 }
    
}

void Top::readProperties(KConfig *config)
{
 // the 'config' object points to the session managed
 // config file.  this function is automatically called whenever
 // the app is being restored.  read in here whatever you wrote
 // in 'saveProperties'
 if (!config) {
	return;
 }
}

void Top::slotGameNew()
{
 d->mBosonWidget->slotNewGame();
}

void Top::fileSave()
{
}

void Top::fileSaveAs()
{
}

void Top::optionsShowToolbar()
{
 if (d->mToolbarAction->isChecked()) {
	toolBar()->show();
 } else {
	toolBar()->hide();
 }
}

void Top::optionsShowStatusbar()
{
 if (d->mStatusbarAction->isChecked()) {
        statusBar()->show();
 } else {
        statusBar()->hide();
 }
}

void Top::optionsConfigureKeys()
{
 KKeyDialog::configureKeys(actionCollection(), "bosonui.rc");
}

void Top::optionsConfigureToolbars()
{
 // use the standard toolbar editor
 KEditToolbar dlg(actionCollection());
 if (dlg.exec()) {
	// recreate our GUI
	createGUI();
 } 
}

void Top::slotZoom(int index)
{
kdDebug() << "zoom index=" << index << endl;
 double percent = d->mZoomAction->items()[index].toDouble();
 double factor = (double)percent / 100;
 QWMatrix m;
 m.scale(factor, factor);
 d->mBosonWidget->zoom(m);
}

