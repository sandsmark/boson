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
#include "../bosondata.h"
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

void BosonStartupNetwork::slotUnsetKGame()
{
 setGame(0);
}

void BosonStartupNetwork::setGame(Boson* game)
{
 if (mGame) {
	disconnect(mGame, 0, this, 0);
 }
 mGame = game;
 if (!mGame) {
	return;
 }
// connect(mGame, SIGNAL(signalStartupWidgetMessage(const QByteArray&)),
//		this, SLOT(slotNetworkMessage(const QByteArray&)));
 connect(mGame, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(mGame, SIGNAL(signalPlayerLeftGame(KPlayer*)),
		this, SLOT(slotPlayerLeftGame(KPlayer*)));
 connect(mGame, SIGNAL(signalSpeciesChanged(Player*)),
		this, SIGNAL(signalSpeciesChanged(Player*)));
 connect(mGame, SIGNAL(signalTeamColorChanged(Player*)),
		this, SIGNAL(signalTeamColorChanged(Player*)));
 connect(mGame, SIGNAL(signalPlayFieldChanged(const QString&)),
		this, SLOT(slotPlayFieldChanged(const QString&)));
 connect(mGame, SIGNAL(signalStartGameClicked()),
		this, SIGNAL(signalStartGameClicked()));
 connect(mGame, SIGNAL(signalAdminStatusChanged(bool)),
		this, SIGNAL(signalSetAdmin(bool)));
 connect(mGame, SIGNAL(destroyed()),
		this, SLOT(slotUnsetKGame()));

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
 BO_CHECK_NULL_RET(p);
 boDebug() << k_funcinfo << "there are " << boGame->playerList()->count() << " players in game now" << endl;
 connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*)));

 // warning: we are assuming here that if playerCount() == 1 then the *local*
 // player just entered the game.
 // currently we can safely assume this, but it might be dangerous for network
 // games. im not really sure how playerCount() gets handled for deactivated
 // players in KGame (i.e. when the local player enters another game and so)
 if (boGame->playerCount() == 1) {
	boDebug() << k_funcinfo << "first player entered game - this must be the local player!" << endl;
	if (boGame->localPlayer() && (Player*)p != boGame->localPlayer()) {
		boWarning() << k_funcinfo << "seems not to be the local player!! expecting another player..." << endl;
	} else {
		// first unset any previous local player, then set the new local player
		emit signalSetLocalPlayer(0);
		emit signalSetLocalPlayer((Player*)p);
	}
 }

 emit signalPlayerJoinedGame(p);
}

void BosonStartupNetwork::slotPlayerLeftGame(KPlayer* p)
{
 disconnect(p, 0, this, 0);
 emit signalPlayerLeftGame(p);
}

void BosonStartupNetwork::sendNewGame(bool editor)
{
 BO_CHECK_NULL_RET(mGame);
 if (!mGame->isAdmin()) {
	boError() << k_funcinfo << "only ADMIN is allowed to send this message" << endl;
	return;
 }
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
 if (!mGame->isAdmin()) {
	boError() << k_funcinfo << "not ADMIN" << endl;
	return;
 }
 if (!identifier.isNull() && !boData->availablePlayFields().contains(identifier)) {
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

void BosonStartupNetwork::sendChangePlayField(int index)
{
 QStringList list = boData->availablePlayFields();
 QString identifier;
 if (index < 0) {
	// warning: valid in editor mode only!
	identifier = QString::null;
 } else {
	QStringList list = boData->availablePlayFields();
	if (index >= (int)list.count()) {
		boError() << k_funcinfo << "invalid index " << index << endl;
		return;
	}
	identifier = list[index];
 }
 sendChangePlayField(identifier);
}

void BosonStartupNetwork::slotPlayFieldChanged(const QString& id)
{
 BosonPlayField* field = 0;
 if (!id.isEmpty()) {
	field = boData->playField(id);
 }

 emit signalPlayFieldChanged(id);
 emit signalPlayFieldChanged(field);
}

