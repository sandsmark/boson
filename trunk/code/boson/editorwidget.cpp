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
#include "boson.h"
#include "bosonmap.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "global.h"
#include "commandinput.h"
#include "bodebug.h"
#include "bpfdescriptiondialog.h"
#include "commandframe/editorcommandframe.h"
#include "sound/bosonmusic.h"

#include <kfiledialog.h>
#include <klocale.h>
#include <kaction.h>
#include <kdeversion.h>
#include <kmessagebox.h>

#include <qptrlist.h>

#include "editorwidget.moc"

class EditorWidget::EditorWidgetPrivate
{
public:
	EditorWidgetPrivate()
	{
		mCmdInput = 0;

		mPlayerAction = 0;
		mPlaceAction = 0;
	}

	CommandInput* mCmdInput;

	KSelectAction* mPlayerAction;
	KSelectAction* mPlaceAction;

	QPtrList<Player> mPlayers;
};

EditorWidget::EditorWidget(TopWidget* top, QWidget* parent)
    : BosonWidgetBase(top, parent)
{
 d = new EditorWidgetPrivate;
}

EditorWidget::~EditorWidget()
{
 boDebug() << k_funcinfo << endl;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void EditorWidget::initDisplayManager()
{
 BosonWidgetBase::initDisplayManager();
 connect(cmdFrame(), SIGNAL(signalPlaceUnit(unsigned long int, Player*)),
		displayManager(), SLOT(slotPlaceUnit(unsigned long int, Player*)));
 connect(cmdFrame(), SIGNAL(signalPlaceCell(int)),
		displayManager(), SLOT(slotPlaceCell(int)));
}

void EditorWidget::initConnections()
{
 BosonWidgetBase::initConnections();
// connect(canvas(), SIGNAL(signalOutOfGame(Player*)),
//		this, SLOT(slotOutOfGame(Player*)));

 connect(boGame, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(boGame, SIGNAL(signalPlayerLeftGame(KPlayer*)),
		this, SLOT(slotPlayerLeftGame(KPlayer*)));
 connect(boGame, SIGNAL(signalGameStarted()),
		this, SLOT(slotGameStarted()));
}

void EditorWidget::initMap()
{
 BosonWidgetBase::initMap();
 if (!boGame->playField() || !boGame->playField()->map()) {
	boError() << k_funcinfo << endl;
	return;
 }
 connect(boGame->playField()->map(), SIGNAL(signalTileSetChanged(BosonTiles*)),
		this, SLOT(slotTileSetChanged(BosonTiles*)));
 connect(boGame, SIGNAL(signalChangeCell(int,int,int,unsigned char)),
		boGame->playField()->map(), SLOT(slotChangeCell(int,int,int,unsigned char)));
 connect(boGame, SIGNAL(signalChangeCell(int,int,int,unsigned char)),
		minimap(), SLOT(slotChangeCell(int,int,int,unsigned char)));
}

void EditorWidget::initPlayer()
{
 BosonWidgetBase::initPlayer();
 /*
 if (!d->mCmdInput) {
	boError() << k_funcinfo << "NULL command input" << endl;
 } else {
	localPlayer()->addGameIO(d->mCmdInput);
 }
 */

 minimap()->slotShowMap(true);
}

BosonCommandFrameBase* EditorWidget::createCommandFrame(QWidget* parent)
{
 EditorCommandFrame* frame = new EditorCommandFrame(parent);
// connect(boGame, SIGNAL(signalUpdateProduction(Unit*)),
//		frame, SLOT(slotUpdateProduction(Unit*)));

// d->mCmdInput = new EditorCommandInput;
// d->mCmdInput->setCommandFrame(frame);
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
// KStdAction::save(this, SLOT(slotSavePlayField()), actionCollection(), "file_save_playfield");
 KStdAction::saveAs(this, SLOT(slotSavePlayFieldAs()), actionCollection(), "file_save_playfield_as");
 KStdAction::quit(this, SIGNAL(signalQuit()), actionCollection());

 d->mPlayerAction = new KSelectAction(i18n("&Player"), KShortcut(), actionCollection(), "editor_player");
 connect(d->mPlayerAction, SIGNAL(activated(int)),
		this, SLOT(slotChangeLocalPlayer(int)));

 QStringList list;
 list.append(i18n("&Facilities"));
 list.append(i18n("&Mobiles"));
 list.append(i18n("&Small"));
 list.append(i18n("&Plain"));
 list.append(i18n("&Big1"));
 list.append(i18n("B&ig1"));
 d->mPlaceAction = new KSelectAction(i18n("Place"), KShortcut(), actionCollection(), "editor_place");
 connect(d->mPlaceAction, SIGNAL(activated(int)),
		this, SLOT(slotPlace(int)));
 d->mPlaceAction->setItems(list);

 KShortcut s;
 s.append(QKeySequence(Qt::Key_Delete));
 s.append(QKeySequence(Qt::Key_D));
 (void)new KAction(i18n("Delete selected unit"), KShortcut(s), displayManager(),
		SLOT(slotDeleteSelectedUnits()), actionCollection(),
		"editor_delete_selected_unit");

 (void)new KAction(i18n("Map &description"), KShortcut(), this,
		SLOT(slotEditMapDescription()), actionCollection(),
		"editor_map_description");

 (void)new KAction(i18n("Player &Settings"), KShortcut(), this,
		SLOT(slotEditPlayerSettings()), actionCollection(),
		"editor_player_settings");

// KStdAction::preferences(bosonWidget(), SLOT(slotGamePreferences()), actionCollection()); // FIXME: slotEditorPreferences()
}

void EditorWidget::saveConfig()
{
  // note: the game is *not* saved here! just general settings like game speed,
  // player name, ...
 boDebug() << k_funcinfo << endl;
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }
 BosonWidgetBase::saveConfig();

// boConfig->save(editor); //FIXME
 boDebug() << k_funcinfo << "done" << endl;
}

void EditorWidget::slotSavePlayFieldAs()
{
 boDebug() << k_funcinfo << endl;
 QString startIn; // shall we provide this??
 QString fileName = KFileDialog::getSaveFileName(startIn, "*.bpf", this);
 if (fileName.isNull()) {
	return;
 }
 QFileInfo info(fileName);
 if (info.extension().isEmpty()) {
	fileName += ".bpf";
 }
 if (info.exists()) {
	int r = KMessageBox::warningYesNoCancel(this, i18n("The file \"%1\" already exists. Are you sure you want to overwrite it?").arg(info.fileName()), i18n("Overwrite File?"));
	if (r != KMessageBox::Yes) {
		return;
	}
 }
 savePlayField(fileName);
}

void EditorWidget::savePlayField(const QString& fileName)
{
 boGame->playField()->applyScenario(boGame); // this must be called before we are able to save the playfield! otherwise the old units will be used
 bool ok = boGame->playField()->savePlayField(fileName);
 if (!ok) {
	boError() << k_funcinfo << "An error occured" << endl;

	// TODO: get an error message from the playfield and display the
	// reason for the error
	KMessageBox::sorry(this, i18n("Could not save to %1").arg(fileName));
 } else {
	boDebug() << k_funcinfo << "Saved successful to " << fileName << endl;
 }
}

void EditorWidget::slotSavePlayField()
{
 boWarning() << k_funcinfo << "not yet implemented" << endl;
}

void EditorWidget::slotChangeLocalPlayer(int index)
{
 Player* p = 0;
 p = d->mPlayers.at(index);
 if (p) {
	emit signalChangeLocalPlayer(p);
	if (d->mPlaceAction->currentItem() >= 0) {
		slotPlace(d->mPlaceAction->currentItem());
	}
 } else {
	boWarning() << k_funcinfo << "NULL player for index " << index << endl;
 }
}

void EditorWidget::slotPlace(int index)
{
 EditorCommandFrame* cmd = editorCmdFrame();
 if (!cmd) {
	boError() << k_funcinfo << "NULL cmd frame" << endl;
	return;
 }
 switch (index) {
	case 0:
		cmd->placeFacilities(localPlayer());
		break;
	case 1:
		cmd->placeMobiles(localPlayer());
		break;
	case 2:
		cmd->placeCells(CellSmall);
		break;
	case 3:
		cmd->placeCells(CellPlain);
		break;
	case 4:
		cmd->placeCells(CellBig1);
		break;
	case 5:
		cmd->placeCells(CellBig2);
		break;
	default:
		boError() << k_funcinfo << "Invalid index " << index << endl;
		return;
 }
}

void EditorWidget::setBosonXMLFile()
{
 BosonWidgetBase::setBosonXMLFile();
 setXMLFile("editorui.rc", true);
}

void EditorWidget::slotPlayerJoinedGame(KPlayer* player)
{
 boDebug() << k_funcinfo << endl;
 if (!player) {
	return;
 }
 BosonWidgetBase::slotPlayerJoinedGame(player);
 Player* p = (Player*)player;
 QStringList players = d->mPlayerAction->items();
 d->mPlayers.append(p);
 players.append(p->name());
 d->mPlayerAction->setItems(players);

 // dunno if this makes sense - but currently one cannot add more players so we
 // just activate the player that was added last.
 d->mPlayerAction->setCurrentItem(players.count() - 1);
 slotChangeLocalPlayer(d->mPlayerAction->currentItem());
}

EditorCommandFrame* EditorWidget::editorCmdFrame() const
{
 return (EditorCommandFrame*)cmdFrame();
}

void EditorWidget::slotPlayerLeftGame(KPlayer* player)
{
 if (!player) {
	return;
 }
 boDebug() << k_funcinfo << player->id() << endl;
 BosonWidgetBase::slotPlayerLeftGame(player);
 Player* p = (Player*)p;
 if (d->mPlayers.count() == 0) {
	return;
 }
 int index = d->mPlayers.find((Player*)player);
 if (index < 0) {
	boError() << k_funcinfo << "player not found" << endl;
	return;
 }
 QStringList players = d->mPlayerAction->items();

 players.remove(players.at(index));
 d->mPlayers.removeRef((Player*)player);

 d->mPlayerAction->setItems(players);
}

void EditorWidget::slotTileSetChanged(BosonTiles* t)
{
 if (!editorCmdFrame()) {
	return;
 }
 editorCmdFrame()->setTileSet(t);
}

void EditorWidget::slotGameStarted()
{
 d->mPlaceAction->setCurrentItem(0);
 slotPlace(0);
}

void EditorWidget::startScenarioAndGame()
{
 BosonWidgetBase::startScenarioAndGame();
 slotChangeLocalPlayer(0);
 d->mPlayerAction->setCurrentItem(0);
}

void EditorWidget::slotEditMapDescription()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->description());


// TODO: non-modal might be fine. one could use that for translations (one
// dialog the original language, one the translated language)
 BPFDescriptionDialog* dialog = new BPFDescriptionDialog(this, true);
 dialog->setDescription(boGame->playField()->description());
 dialog->exec();

 delete dialog;
}

void EditorWidget::slotEditPlayerSettings()
{
 BO_CHECK_NULL_RET(boGame);

}

