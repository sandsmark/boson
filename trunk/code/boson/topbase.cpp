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
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <ktoolbar.h>
#include <kpopupmenu.h>
#include <kshortcut.h>

#include <qwmatrix.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>

#include "topbase.moc"

#define MAX_BUTTONS_PER_ROW 4

BosonCommandBar::BosonCommandBar(QMainWindow* parent, QMainWindow::ToolBarDock dock) 
		: KToolBar(parent, dock, false, i18n("CommandFrame"), false, false)
{
 mContext = 0;
}

BosonCommandBar::~BosonCommandBar()
{
 if (mContext) {
	delete mContext;
 }
}

void BosonCommandBar::mousePressEvent(QMouseEvent* e)
{
 // most parts here are from KToolBar::mousePressEvent()
 // we want a customized toolbar - so wee need to replace it
 QMainWindow* m = mainWindow();
 if (!m) {
	return;
 }
 if (!m->dockWindowsMovable()) {
	return;
 }
 if (e->button() == RightButton) {
	int id = contextMenu()->exec(e->globalPos(), 0);
	if (id == -1) {
		return;
	}
	switch (id) {
		case CONTEXT_LEFT:
			m->moveDockWindow(this, QMainWindow::DockLeft);
			break;
		case CONTEXT_RIGHT:
			m->moveDockWindow(this, QMainWindow::DockRight);
			break;
		case CONTEXT_FLAT:
			m->moveDockWindow(this, QMainWindow::DockMinimized);
			break;
		default:
			if (id >= CONTEXT_BUTTONS_ROW) {
				int buttons = id - CONTEXT_BUTTONS_ROW;
				emit signalButtonsPerRow(buttons);
				break;
			}
			kdWarning() << "Unknown id " << id << endl;
			break;
	}
 }
}

KPopupMenu* BosonCommandBar::contextMenu()
{
 if (mContext) {
	for (int i = CONTEXT_LEFT; i <= CONTEXT_FLAT; i++) {
		mContext->setItemChecked(i, false);
	}
	switch (barPos()) {
		case KToolBar::Flat:
			mContext->setItemChecked(CONTEXT_FLAT, true);
			break;
		case KToolBar::Right:
			mContext->setItemChecked(CONTEXT_RIGHT, true);
			break;
		case KToolBar::Left:
			mContext->setItemChecked(CONTEXT_LEFT, true);
			break;
		default:
			break;
	}
		for (unsigned int i = 0; i <= MAX_BUTTONS_PER_ROW; i++) { 
			mContext->setItemChecked(CONTEXT_BUTTONS_ROW + i, 
					false);
		}
		mContext->setItemChecked(CONTEXT_BUTTONS_ROW + 
				boConfig->commandButtonsPerRow(), true);
	return mContext;
 }
 mContext = new KPopupMenu(0, "context");
 mContext->insertTitle(i18n("Toolbar Menu")); // CommandFrame menu?
 KPopupMenu* orient = new KPopupMenu(mContext, "orient");
 orient->insertItem(i18n("Left"), CONTEXT_LEFT);
 orient->insertItem(i18n("Right"), CONTEXT_RIGHT);
 orient->insertSeparator(-1);
 orient->insertItem(i18n("Flat"), CONTEXT_FLAT);

 // TODO: implement this using KAction! -> put them into the
 // menubar, too!
 KPopupMenu* size = new KPopupMenu(mContext, "size");
 for (int i = 1; i <= MAX_BUTTONS_PER_ROW; i++) {
	size->insertItem(QString::number(i), CONTEXT_BUTTONS_ROW + i);
 }

 mContext->insertItem(i18n("Oritentation"), orient);
 mContext->insertItem(i18n("Buttons per row"), size);

 return contextMenu(); // call again to set defaults
}

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

	KToolBar* mCommandBar;
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
}

TopBase::~TopBase()
{
 CommandFramePosition pos;
/* if (!d->mCommandBar->area()) {
	pos = DockUnmanaged; // not yet supported for startup
 } else */
 if (d->mCommandBar->area() == rightDock()) {
	pos = CmdFrameRight;
 } else {
	pos = CmdFrameLeft;
 }
 BosonConfig::saveCommandFramePosition(pos);

 delete d->mCommandFrame;
 delete d->mCommandBar;
 delete d;
}

void TopBase::initKAction()
{
// Debug - no i18n!
 (void)new KAction("Debug", KShortcut(), mBosonWidget, SLOT(slotDebug()), actionCollection(), "debug_kgame");
 (void)new KAction("Unfog", KShortcut(), mBosonWidget, SLOT(slotUnfogAll()), actionCollection(), "debug_unfog");
 KSelectAction* s = new KSelectAction("Mode", KShortcut(), actionCollection(), "debug_mode");
 s = new KSelectAction("Mode", KShortcut(), actionCollection(), "debug_mode");
 connect(s, SIGNAL(activated(int)), this, SLOT(slotDebugMode(int)));
 QStringList l;
 l.append("Normal");
 l.append("Debug Selection");
 s->setItems(l);
 s->setCurrentItem(0);

 d->mToolbarAction = KStdAction::showToolbar(this, SLOT(slotShowToolbar()), actionCollection());
 d->mStatusbarAction = KStdAction::showStatusbar(this, SLOT(slotShowStatusbar()), actionCollection());

 KStdAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection());

 d->mChatAction = new KToggleAction(i18n("Show &Chat"), 
		KShortcut(Qt::CTRL+Qt::Key_H), this, SLOT(slotShowChat()),
		actionCollection(), "options_show_chat");
 d->mChatAction->setChecked(false);
 slotShowChat();

 KToggleAction* sound = new KToggleAction(i18n("Soun&d"), 0, mBosonWidget, 
		SLOT(slotToggleSound()), actionCollection(), "options_sound");
 sound->setChecked(mBosonWidget->sound());
 
 KToggleAction* music = new KToggleAction(i18n("&Music"), 0, mBosonWidget, 
		SLOT(slotToggleMusic()), actionCollection(), "options_music");
 music->setChecked(mBosonWidget->music());

 d->mZoomAction = new KSelectAction(i18n("&Zoom"), KShortcut(), actionCollection(), "options_zoom");
 connect(d->mZoomAction, SIGNAL(activated(int)), 
		this, SLOT(slotZoom(int)));
 QStringList items;
 items.append(QString::number(50));
 items.append(QString::number(100));
 items.append(QString::number(150));
 d->mZoomAction->setItems(items);

 // note: the icons for these action need to have konqueror installed!
 (void)new KAction(i18n( "Split View &Left/Right"), "view_left_right",
		   CTRL+SHIFT+Key_L, mBosonWidget, SLOT(slotSplitViewHorizontal()),
		   actionCollection(), "splitviewh");
 (void)new KAction(i18n("Split View &Top/Bottom"), "view_top_bottom",
		   CTRL+SHIFT+Key_T, mBosonWidget, SLOT(slotSplitViewVertical()),
		   actionCollection(), "splitviewv");
// (void)new KAction(i18n("&Remove Active View"), "view_remove", 
//		  CTRL+SHIFT+Key_R, mBosonWidget, SLOT(slotRemoveView()),
//		  actionCollection(), "removeview");
     
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

void TopBase::initCommandFrame()
{
kdDebug() << k_funcinfo << endl;
 d->mCommandBar = new BosonCommandBar(this, QMainWindow::Left);
 connect(d->mCommandBar, SIGNAL(signalButtonsPerRow(int)),
		mBosonWidget, SLOT(slotSetCommandButtonsPerRow(int)));
 d->mCommandBar->setTitle(i18n("Command Frame"));
 d->mCommandBar->setEnableContextMenu(false);
 d->mCommandFrame = new QVBox(d->mCommandBar);
 mBosonWidget->reparentMiniMap(d->mCommandFrame);

 setDockEnabled(d->mCommandBar, DockTop, false);
 setDockEnabled(d->mCommandBar, DockBottom, false);

 connect(mBosonWidget, SIGNAL(signalMoveCommandFrame(int)),
		this, SLOT(slotMoveCommandFrame(int)));
 slotMoveCommandFrame(BosonConfig::readCommandFramePosition());
 
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
 if (pos == CmdFrameUndocked) {
	d->mCommandBar->undock();
 } else if (pos == CmdFrameRight) {
	moveDockWindow(d->mCommandBar, DockRight);
 } else {
	moveDockWindow(d->mCommandBar, DockLeft);
 }
}

void TopBase::slotDebugMode(int index)
{
 kdDebug() << index << endl;
 boConfig->setDebugMode((BosonConfig::DebugMode)index);
}
