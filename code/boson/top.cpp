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
#include "top.h"

#include "bosonwidget.h"

#include <kapplication.h>
#include <klocale.h>
#include <kkeydialog.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kstdgameaction.h>
#include <kaction.h>
#include <kdebug.h>

#include "top.moc"

Top::Top() : TopBase()
{
 initKAction();
 initCommandFrame();

 bosonWidget()->addGameCommandFrame(commandFrame());

 initStatusBar();

 bosonWidget()->startGameMode();

 bosonWidget()->slotNewGame(); // adds a local player, too
}

Top::~Top()
{
 bosonWidget()->saveConfig();
}

void Top::initKAction()
{
 KStdGameAction::gameNew(bosonWidget(), SLOT(slotNewGame()), actionCollection());
 KStdGameAction::quit(kapp, SLOT(quit()), actionCollection());
 KStdGameAction::end(bosonWidget(), SLOT(slotEndGame()), actionCollection());

 KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
 KStdAction::preferences(bosonWidget(), SLOT(slotGamePreferences()), actionCollection()); // FIXME: for game only - not editor!

 bosonWidget()->initKeys(false);
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
 KKeyDialog dlg(this);
 dlg.insert(actionCollection());
 dlg.insert(bosonWidget()->actionCollection());
 dlg.configure(true);
}

