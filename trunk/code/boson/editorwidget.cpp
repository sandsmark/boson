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
#include "bosonmessage.h"
#include "bosonplayfield.h"
#include "bosonscenario.h"
#include "bosonconfig.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "global.h"
#include "bosonbigdisplay.h"
#include "commandinput.h"
#include "commandframe/editorcommandframe.h"
#include "sound/bosonmusic.h"

#include <klocale.h>
#include <kaction.h>
#include <kdeversion.h>
#include <kdebug.h>

#include <qtimer.h>
#include <qregexp.h>
#include <qintdict.h>

#include "editorwidget.moc"

class EditorWidget::EditorWidgetPrivate
{
public:
	EditorWidgetPrivate()
	{
		mCmdInput = 0;

		mPlayerAction = 0;
		mCellsAction = 0;

		mActionFacilities = 0;
		mActionMobiles = 0;
		mActionCellPlain = 0;
		mActionCellSmall = 0;
		mActionCellBig1 = 0;
		mActionCellBig2 = 0;
	}

	CommandInput* mCmdInput;

	KSelectAction* mPlayerAction;
	KSelectAction* mCellsAction;

	KRadioAction* mActionFacilities;
	KRadioAction* mActionMobiles;
	KRadioAction* mActionCellPlain;
	KRadioAction* mActionCellSmall;
	KRadioAction* mActionCellBig1;
	KRadioAction* mActionCellBig2;

	QIntDict<Player> mPlayers;
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
 localPlayer()->addGameIO(d->mCmdInput);// AB: for editor mode, too? maybe place into base widget

 minimap()->slotShowMap(true);
}

BosonCommandFrameBase* EditorWidget::createCommandFrame(QWidget* parent)
{
 EditorCommandFrame* frame = new EditorCommandFrame(parent);
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
 (void)new KAction(i18n("Save &PlayField as..."), KShortcut(), this,
		SLOT(slotSavePlayFieldAs()), actionCollection(),
		"file_save_playfield_as");

 d->mPlayerAction = new KSelectAction(i18n("&Player"), KShortcut(), actionCollection(), "editor_player");
 connect(d->mPlayerAction, SIGNAL(activated(int)),
		this, SLOT(slotChangeLocalPlayer(int)));
 d->mActionFacilities = new KRadioAction(i18n("&Facilities"), KShortcut(),
		this, SLOT(slotPlaceFacilities()), actionCollection(),
		"editor_place_facilities");
 d->mActionFacilities->setExclusiveGroup("Place");
 d->mActionMobiles = new KRadioAction(i18n("&Mobiles"), KShortcut(),
		this, SLOT(slotPlaceMobiles()), actionCollection(),
		"editor_place_mobiles");
 d->mActionMobiles->setExclusiveGroup("Place");
 d->mActionCellSmall = new KRadioAction(i18n("&Small"), KShortcut(),
		this, SLOT(slotPlaceCellSmall()), actionCollection(),
		"editor_place_cell_small");
 d->mActionCellSmall->setExclusiveGroup("Place");
 d->mActionCellPlain = new KRadioAction(i18n("&Plain"), KShortcut(), 
		this, SLOT(slotPlaceCellPlain()), actionCollection(),
		"editor_place_cell_plain");
 d->mActionCellPlain->setExclusiveGroup("Place");
 d->mActionCellBig1 = new KRadioAction(i18n("&Big1"), KShortcut(), 
		this, SLOT(slotPlaceCellBig1()),actionCollection(),
		"editor_place_cell_big1");
 d->mActionCellBig1->setExclusiveGroup("Place");
 d->mActionCellBig2 = new KRadioAction(i18n("B&ig2"), KShortcut(),
		this, SLOT(slotPlaceCellBig2()),actionCollection(),
		"editor_place_cell_big2");
 d->mActionCellBig2->setExclusiveGroup("Place");

// (void)new KAction(i18n("&Create Custom Unit"), KShortcut(), this,
//		  SLOT(slotCreateUnit()), actionCollection(),
//		  "editor_create_unit");

// KStdAction::preferences(bosonWidget(), SLOT(slotGamePreferences()), actionCollection()); // FIXME: slotEditorPreferences()
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
 if (!localPlayer()) {
	kdError() << k_funcinfo << "NULL local player" << endl;
	return;
 }
 BosonWidgetBase::saveConfig();

// boConfig->save(editor); //FIXME
 kdDebug() << k_funcinfo << "done" << endl;
}

void EditorWidget::slotSavePlayFieldAs()
{
}

void EditorWidget::slotSavePlayField()
{
}

void EditorWidget::slotChangeLocalPlayer(int)
{
}

void EditorWidget::slotPlaceFacilities()
{
}

void EditorWidget::slotPlaceMobiles()
{
}

void EditorWidget::slotPlaceCellSmall()
{
}

void EditorWidget::slotPlaceCellPlain()
{
}

void EditorWidget::slotPlaceCellBig1()
{
}

void EditorWidget::slotPlaceCellBig2()
{
}

void EditorWidget::setBosonXMLFile()
{
 BosonWidgetBase::setBosonXMLFile();
 setXMLFile("editorui.rc", true);
}

