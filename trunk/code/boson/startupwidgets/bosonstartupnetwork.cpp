/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonstartupnetwork.h"
#include "bosonstartupnetwork.moc"

#include "../boson.h"
#include "../bosonplayfield.h"
#include "../player.h"
#include "../bosonmessage.h"
#include "../defines.h"
#include "bodebug.h"

#include <kgame/kplayer.h>

BosonStartupNetwork::BosonStartupNetwork(QObject* parent) : QObject(parent, "BosonStartupNetwork")
{
 mGame = 0;
}

BosonStartupNetwork::~BosonStartupNetwork()
{
}

void BosonStartupNetwork::setGame(Boson* game)
{
 if (mGame) {
	// disconnect game and players
	disconnect(mGame, 0, this, 0);
	QPtrList<KPlayer> list = *mGame->playerList();
	QPtrListIterator<KPlayer> it(list);;
	for (; it.current(); ++it) {
		slotPlayerLeftGame(it.current());
	}
 }
 mGame = game;
 if (!mGame) {
	return;
 }
// connect(mGame, SIGNAL(signalStartupWidgetMessage(const QByteArray&)),
//		this, SLOT(slotNetworkMessage(const QByteArray&)));

 QPtrList<KPlayer> list = *mGame->playerList();
 QPtrListIterator<KPlayer> it(list);;
 for (; it.current(); ++it) {
	slotPlayerJoinedGame(it.current());
 }
}

void BosonStartupNetwork::slotNetworkMessage(const QByteArray& buffer)
{
}

void BosonStartupNetwork::slotPlayerPropertyChanged(KGamePropertyBase* prop, KPlayer* p)
{
 // TODO: make sure this doesn't get called during the game anymore! 
 // -> performance
 switch (prop->id()) {
	case KGamePropertyBase::IdName:
		emit signalPlayerNameChanged((Player*)p);
		break;
	default:
		break;
 }
}

void BosonStartupNetwork::slotPlayerJoinedGame(KPlayer* p)
{
 connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPlayerPropertyChanged(KGamePropertyBase, KPlayer*)));
}

void BosonStartupNetwork::slotPlayerLeftGame(KPlayer* p)
{
 disconnect(p, 0, this, 0);
}

void BosonStartupNetwork::sendNewGame(bool editor)
{
 BO_CHECK_NULL_RET(mGame);
 if (editor) {
	mGame->sendMessage(0, BosonMessage::IdNewEditor);
 } else {
	mGame->sendMessage(0, BosonMessage::IdNewGame);
 }
}

void BosonStartupNetwork::sendChangeTeamColor(Player* p, const QColor& color)
{
 BO_CHECK_NULL_RET(mGame);
 BO_CHECK_NULL_RET(p);
 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)p->id();
 stream << (Q_UINT32)color.rgb();
 mGame->sendMessage(b, BosonMessage::ChangeTeamColor);
}

void BosonStartupNetwork::sendChangeSpecies(Player* p, const QString& species, const QColor& color)
{
 BO_CHECK_NULL_RET(mGame);
 BO_CHECK_NULL_RET(p);
 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)p->id();
 stream << species;
 stream << (Q_UINT32)color.rgb();
 mGame->sendMessage(b, BosonMessage::ChangeSpecies);
}

void BosonStartupNetwork::sendChangePlayerName(Player* p, const QString& name)
{
 // player name uses KGamePropertyBase::PolicyClean, so we can simply set the
 // new name. the variable will change once it is received from network.
 BO_CHECK_NULL_RET(p);
 p->setName(name);
}

void BosonStartupNetwork::sendChangePlayField(const QString& identifier)
{
 BO_CHECK_NULL_RET(mGame);
 if (!identifier.isNull() && !BosonPlayField::availablePlayFields().contains(identifier)) {
	boError() << k_funcinfo << "Invalid playfield identifier " << identifier << endl;
	return;
 }
 // a NULL identifier means "new map", for the start editor widget
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 // transmit the identifier/name so that the remote newgame dialogs will be able
 // to display the newly selected playfield
 stream << identifier;
 mGame->sendMessage(buffer, BosonMessage::ChangePlayField);
}

void BosonStartupNetwork::removePlayer(KPlayer* p)
{
 BO_CHECK_NULL_RET(mGame);
 BO_CHECK_NULL_RET(p);
 mGame->removePlayer(p);
}

void BosonStartupNetwork::sendStartGameClicked()
{
 BO_CHECK_NULL_RET(mGame);
 mGame->sendMessage(0, BosonMessage::IdStartGameClicked);
}

