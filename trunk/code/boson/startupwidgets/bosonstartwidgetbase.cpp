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
#include "../boson.h"
#include "../bosonplayfield.h"
#include "bodebug.h"

#include <klocale.h>

BosonStartWidgetBase::BosonStartWidgetBase(QWidget* parent)
    : QWidget(parent)
{
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
 // AB: not much to do here, since we use BosonPlayField::availablePlayFields()
 // everywhere. if we manage something like mIndex2Identifier one day we should
 // init it here (not in derived classes)
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

