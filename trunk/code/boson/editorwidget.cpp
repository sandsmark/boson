/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "editorwidget.h"

#include "defines.h"
#include "bosonminimap.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "player.h"
#include "bosoncommandframe.h"
#include "bosonmessage.h"
#include "bosonplayfield.h"
#include "bosonscenario.h"
#include "bosonconfig.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "global.h"
#include "bosonbigdisplay.h"
#include "commandinput.h"
#include "sound/bosonmusic.h"

#include <klocale.h>
#include <kaction.h>
#include <kdeversion.h>
#include <kdebug.h>

#include <qtimer.h>
#include <qregexp.h>

#include "editorwidget.moc"

class EditorWidget::EditorWidgetPrivate
{
public:
	EditorWidgetPrivate()
	{
		mCmdInput = 0;
	}

	CommandInput* mCmdInput;
};

EditorWidget::EditorWidget(TopWidget* top, QWidget* parent, bool loading)
    : BosonWidgetBase(top, parent, loading)
{
 d = new EditorWidgetPrivate;
}

EditorWidget::~EditorWidget()
{
 kdDebug() << k_funcinfo << endl;
 delete d;
 kdDebug() << k_funcinfo << "done" << endl;
}

void EditorWidget::initConnections()
{
 BosonWidgetBase::initConnections();
// connect(canvas(), SIGNAL(signalOutOfGame(Player*)),
//		this, SLOT(slotOutOfGame(Player*)));

}

void EditorWidget::initPlayer()
{
 BosonWidgetBase::initPlayer();
 player()->addGameIO(d->mCmdInput);// AB: for editor mode, too? maybe place into base widget

 minimap()->slotShowMap(true);
}

BosonCommandFrame* EditorWidget::createCommandFrame(QWidget* parent)
{
 BosonCommandFrame* frame = new BosonCommandFrame(parent, true);
 connect(game(), SIGNAL(signalUpdateProduction(Unit*)),
		frame, SLOT(slotUpdateProduction(Unit*)));

 //AB: can we use the same input for the editor?
 d->mCmdInput = new CommandInput;
 d->mCmdInput->setCommandFrame(frame);
 return frame;
}

void EditorWidget::slotChangeCursor(int , const QString& )
{
 // editor mode
 changeCursor(new BosonKDECursor());
}

void EditorWidget::slotOutOfGame(Player* p)
{
}

void EditorWidget::initKActions()
{
 BosonWidgetBase::initKActions();
}

void EditorWidget::slotEndGame()
{
// this needs to be done first, before the players are removed
 displayManager()->quitGame();
 canvas()->deleteDestroyed();
 game()->quitGame();
}

void EditorWidget::saveConfig()
{
  // note: the game is *not* saved here! just general settings like game speed,
  // player name, ...
 kdDebug() << k_funcinfo << endl;
 if (!game()) {
	kdError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!player()) {
	kdError() << k_funcinfo << "NULL local player" << endl;
	return;
 }
 BosonWidgetBase::saveConfig();

// boConfig->save(editor); //FIXME
 kdDebug() << k_funcinfo << "done" << endl;
}

