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
	}

};

Top::Top() : TopBase()
{
 d = new TopPrivate;

 bosonWidget()->addGameCommandFrame();

 initKAction();
 initStatusBar();

 showMaximized();

 bosonWidget()->slotNewGame(); // adds a local player, too
}

Top::~Top()
{
 bosonWidget()->saveConfig();
 delete d;
}

void Top::initKAction()
{
 KStdGameAction::gameNew(bosonWidget(), SLOT(slotNewGame()), actionCollection());
 KStdGameAction::quit(kapp, SLOT(quit()), actionCollection());
 KStdGameAction::end(bosonWidget(), SLOT(slotEndGame()), actionCollection());

 KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
 KStdAction::preferences(bosonWidget(), SLOT(slotGamePreferences()), actionCollection()); // FIXME: for game only - not editor!

 createGUI();
}

void Top::initStatusBar()
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
 bosonWidget()->slotNewGame();
}

void Top::slotConfigureKeys()
{
 KKeyDialog::configureKeys(actionCollection(), "bosonui.rc");
}

