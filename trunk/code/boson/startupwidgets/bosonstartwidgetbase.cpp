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

#include "bosonstartwidgetbase.h"
#include "bosonstartwidgetbase.moc"

#include "../defines.h"
#include "../bosonconfig.h"
#include "../bosonmessage.h"
#include "../player.h"
#include "../speciestheme.h"
#include "../bosoncomputerio.h"
#include "../boson.h"
#include "../top.h"
#include "../bosonplayfield.h"
#include "../bosonscenario.h"
#include "bodebug.h"

#include <klocale.h>
#include <kgame/kgameproperty.h>
#include <kgame/kgamechat.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>

#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>

// AB: FIXME: the code design here is BAAAD. lots of strange names for the
// layouts, strange names for the spacers (which might be replaced by normal
// stretches).
// short: qt designer code
BosonStartWidgetBase::BosonStartWidgetBase(TopWidget* top, QWidget* parent)
    : QWidget(parent)
{
// FIXME: add a "widget" to class name
 mTop = top; // AB: i dislike this

 if (!boGame) {
	boError() << k_funcinfo << "NULL Boson object" << endl;
	return;
 }

 initKGame();

 initPlayFields();
}

BosonStartWidgetBase::~BosonStartWidgetBase()
{
 // Save stuff like player name, color etc.
 boConfig->saveLocalPlayerMap(playFieldIdentifier());
}

void BosonStartWidgetBase::initKGame()
{
 connect(boGame, SIGNAL(signalPlayFieldChanged(const QString&)), this, SLOT(slotPlayFieldChanged(const QString&)));

 // We must manually set maximum players number to some bigger value, because
 //  KGame in KDE 3.0.0 (what about 3.0.1?) doesn't support infinite number of
 //  players (it's a bug actually)
 boGame->setMaxPlayers(BOSON_MAX_PLAYERS);
 boDebug() << k_funcinfo << " minPlayers(): " << boGame->minPlayers() << endl;
 boDebug() << k_funcinfo << " maxPlayers(): " << boGame->maxPlayers() << endl;
}

void BosonStartWidgetBase::initPlayFields()
{
 BosonPlayField::preLoadAllPlayFields();
 QStringList list = BosonPlayField::availablePlayFields();
 QString mapId = boConfig->readLocalPlayerMap();
 if (mapId.isNull() || !list.contains(mapId)) {
	mapId = BosonPlayField::defaultPlayField();
 }
 int mapIndex = list.findIndex(mapId);
#warning use it!
// TODO: make mapIndex the current index!
/*
 if (boGame->isAdmin()) {
	slotSendPlayFieldChanged(mapIndex);
 }
 */
}

void BosonStartWidgetBase::slotSendPlayFieldChanged(int index)
{
 if (!boGame->isAdmin()) {
	boWarning() << "Only admin can change the playfield" << endl;
	//TODO: revert the change
	return;
 }
 QString identifier;
 if (index < 0) {
	// warning: valid in editor mode only!
	identifier = QString::null;
 } else {
	QStringList list = BosonPlayField::availablePlayFields();
	if (index >= (int)list.count()) {
		boError() << k_funcinfo << "invalid index " << index << endl;
		return;
	}
	identifier = list[index];
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 // transmit the identifier/name so that the remote newgame dialogs will be able
 // to display the newly selected playfield
 stream << identifier;
 boGame->sendMessage(buffer, BosonMessage::ChangePlayField);
}

void BosonStartWidgetBase::slotPlayFieldChanged(const QString& id)
{
 mMapId = id;
 BosonPlayField* field = 0;
 if (!mMapId.isEmpty()) {
	field = BosonPlayField::playField(mMapId);
 }
 setCurrentPlayField(field);
}

Player* BosonStartWidgetBase::player() const
{
  return mTop->player();
}

void BosonStartWidgetBase::sendNewGame()
{
 boGame->sendMessage(0, BosonMessage::IdNewGame);
}

