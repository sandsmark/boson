/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "topbase.h"

#include "bosonwidget.h"
#include "bosonconfig.h"

#include <klocale.h>
#include <kmenubar.h>
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kdebug.h>
#if HAVE_KSHORTCUT
#include <kshortcut.h>
#endif

#include <qwmatrix.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>

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
	KToggleAction* mChatAction;
	KSelectAction* mZoomAction;

	QToolBar* mCommandBar;
	QVBox* mCommandFrame; // kind of.. also contains the minimap
};

TopBase::TopBase()
    : KMainWindow( 0 )
{
 d = new TopBasePrivate;

 mBosonWidget = new BosonWidget(this);

 // tell the KMainWindow that this is indeed the main widget
 setCentralWidget(mBosonWidget);

 initKAction();
 initStatusBar();

 d->mCommandBar = new QToolBar(i18n("CommandFrame"), this, QMainWindow::Left);
 d->mCommandFrame = new QVBox(d->mCommandBar);
 mBosonWidget->addMiniMap(d->mCommandFrame);

 setDockEnabled(d->mCommandBar, DockTop, false);
 setDockEnabled(d->mCommandBar, DockBottom, false);
 connect(mBosonWidget, SIGNAL(signalMoveCommandFrame(int)),
		this, SLOT(slotMoveCommandFrame(int)));
 slotMoveCommandFrame(BosonConfig::readCommandFramePosition());
 
 showMaximized();
}

TopBase::~TopBase()
{
 int pos;
/* if (!d->mCommandBar->area()) {
	pos = DockUnmanaged; // not yet supported for startup
 } else */
 if (d->mCommandBar->area() == rightDock()) {
	pos = DockRight;
 } else {
	pos = DockLeft;
 }
 BosonConfig::saveCommandFramePosition(pos);

 delete d->mCommandFrame;
 delete d->mCommandBar;
 delete d;
}

void TopBase::initKAction()
{
// Debug - no i18n!
#if HAVE_KSHORTCUT
 (void)new KAction("Debug", KShortcut(), mBosonWidget, SLOT(slotDebug()), actionCollection(), "game_debug");
#else
 (void)new KAction("Debug", QKeySequence(), mBosonWidget, SLOT(slotDebug()), actionCollection(), "game_debug");
#endif

 d->mToolbarAction = KStdAction::showToolbar(this, SLOT(slotShowToolbar()), actionCollection());
 d->mStatusbarAction = KStdAction::showStatusbar(this, SLOT(slotShowStatusbar()), actionCollection());

 KStdAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection());

 d->mChatAction = new KToggleAction(i18n("Show &Chat"), 
#if HAVE_KSHORTCUT
		KShortcut(Qt::CTRL+Qt::Key_H), this, SLOT(slotShowChat()),
#else
		QKeySequence(Qt::CTRL+Qt::Key_H), this, SLOT(slotShowChat()),
#endif
		actionCollection(), "options_show_chat");
 d->mChatAction->setChecked(false);
 slotShowChat();

 KToggleAction* sound = new KToggleAction(i18n("Soun&d"), 0, mBosonWidget, 
		SLOT(slotToggleSound()), actionCollection(), "options_sound");
 sound->setChecked(mBosonWidget->sound());
 
 KToggleAction* music = new KToggleAction(i18n("&Music"), 0, mBosonWidget, 
		SLOT(slotToggleMusic()), actionCollection(), "options_music");
 music->setChecked(mBosonWidget->music());

#if HAVE_KSHORTCUT
 d->mZoomAction = new KSelectAction(i18n("&Zoom"), KShortcut(), actionCollection(), "options_zoom");
#else
 d->mZoomAction = new KSelectAction(i18n("&Zoom"), QKeySequence(), actionCollection(), "options_zoom");
#endif
 connect(d->mZoomAction, SIGNAL(activated(int)), 
		this, SLOT(slotZoom(int)));
 QStringList items;
 items.append(QString::number(50));
 items.append(QString::number(100));
 items.append(QString::number(150));
 d->mZoomAction->setItems(items);

 // no createGUI() - will be done in derived classes
}

void TopBase::initStatusBar()
{
 KStatusBar* bar = statusBar();
 QHBox* box = new QHBox(bar);
 (void)new QLabel(i18n("Mobiles: "), box);
 QLabel* mobilesLabel = new QLabel(QString::number(0), box); 
 connect(mBosonWidget, SIGNAL(signalMobilesCount(int)),
		mobilesLabel, SLOT(setNum(int)));
 (void)new QLabel(i18n("Facilities: "), box);
 QLabel* facilitiesLabel = new QLabel(QString::number(0), box);
 connect(mBosonWidget, SIGNAL(signalFacilitiesCount(int)),
		facilitiesLabel, SLOT(setNum(int)));
 bar->addWidget(box);

 // AB: does the editor use the resources box? maybe move it to Top (the class)
 QHBox* resources = new QHBox(bar);
 (void)new QLabel(i18n("Minerals: "), resources);
 QLabel* mineralLabel = new QLabel(QString::number(0), resources);
 connect(mBosonWidget, SIGNAL(signalMineralsUpdated(int)), 
		mineralLabel, SLOT(setNum(int)));
 (void)new QLabel(i18n("Oil: "), resources);
 QLabel* oilLabel = new QLabel(QString::number(0), resources);
 connect(mBosonWidget, SIGNAL(signalOilUpdated(int)), 
		oilLabel, SLOT(setNum(int)));
 bar->addWidget(resources);

 bar->show();
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

void TopBase::slotShowChat()
{
 mBosonWidget->setShowChat(d->mChatAction->isChecked());
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

QFrame* TopBase::commandFrame() const
{
 return d->mCommandFrame;
}

void TopBase::slotMoveCommandFrame(int pos)
{
 if (pos == DockUnmanaged) {
	d->mCommandBar->undock();
 } else {
	moveDockWindow(d->mCommandBar, (Dock)pos);
 }
}
