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
#include "topbase.h"

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

#include "topbase.moc"

class TopBase::TopBasePrivate
{
public:
	TopBasePrivate()
	{
		mToolbarAction = 0;
		mStatusbarAction = 0;
		mZoomAction = 0;
	}

	KToggleAction* mToolbarAction;
	KToggleAction* mStatusbarAction;
	KSelectAction* mZoomAction;
};

TopBase::TopBase()
    : KMainWindow( 0 )
{
 d = new TopBasePrivate;

 mBosonWidget = new BosonWidget(this);
 connect(mBosonWidget, SIGNAL(signalUnitCount(int, int)), 
		this, SLOT(slotUnitCount(int, int)));

 // tell the KMainWindow that this is indeed the main widget
 setCentralWidget(mBosonWidget);

 initKAction();

 initStatusBar();

 showMaximized();

// mBosonWidget->slotNewGame(); // adds a local player, too
}

TopBase::~TopBase()
{
 delete d;
}

void TopBase::initKAction()
{
// Debug - no i18n!
 (void)new KAction("Debug", QKeySequence(), mBosonWidget, SLOT(slotDebug()), actionCollection(), "game_debug");

 d->mToolbarAction = KStdAction::showToolbar(this, SLOT(slotShowToolbar()), actionCollection());
 d->mStatusbarAction = KStdAction::showStatusbar(this, SLOT(slotShowStatusbar()), actionCollection());

 KStdAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection());

 d->mZoomAction = new KSelectAction(i18n("&Zoom"), QKeySequence(), actionCollection(), "options_zoom");
 connect(d->mZoomAction, SIGNAL(activated(int)), 
		this, SLOT(slotZoom(int)));
 QStringList items;
 items.append(QString::number(50));
 items.append(QString::number(100));
 items.append(QString::number(150));
 d->mZoomAction->setItems(items);

// createGUI();
}

void TopBase::initStatusBar()
{
 statusBar()->show();
}

void TopBase::saveProperties(KConfig *config)
{
 // the 'config' object points to the session managed
 // config file.  anything you write here will be available
 // later when this app is restored
 if (!config) {
	return;
 }
    
}

void TopBase::readProperties(KConfig *config)
{
 // the 'config' object points to the session managed
 // config file.  this function is automatically called whenever
 // the app is being restored.  read in here whatever you wrote
 // in 'saveProperties'
 if (!config) {
	return;
 }
}

void TopBase::slotShowToolbar()
{
 if (d->mToolbarAction->isChecked()) {
	toolBar()->show();
 } else {
	toolBar()->hide();
 }
}

void TopBase::slotShowStatusbar()
{
 if (d->mStatusbarAction->isChecked()) {
        statusBar()->show();
 } else {
        statusBar()->hide();
 }
}

void TopBase::slotConfigureToolbars()
{
 // use the standard toolbar editor
 KEditToolbar dlg(actionCollection());
 if (dlg.exec()) {
	// recreate our GUI
	createGUI();
 } 
}

void TopBase::slotZoom(int index)
{
kdDebug() << "zoom index=" << index << endl;
 double percent = d->mZoomAction->items()[index].toDouble();
 double factor = (double)percent / 100;
 QWMatrix m;
 m.scale(factor, factor);
 mBosonWidget->zoom(m);
}

void TopBase::slotUnitCount(int mobiles, int facilities)
{
}

