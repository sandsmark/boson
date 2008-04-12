/*
    This file is part of the Boson game
    Copyright (C) 2003-2008 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosonstartupnetwork.h"
#include "bosonstartupnetwork.moc"

#include "../../bomemory/bodummymemory.h"
#include "../gameengine/bpfloader.h"
#include "../gameengine/boson.h"
#include "../gameengine/bosonplayfield.h"
#include "../gameengine/player.h"
#include "../gameengine/bosonmessageids.h"
#include "../bosondata.h"
#include "../defines.h"
#include "../gameview/bosonlocalplayerinput.h" // ugly. we should not include stuff from the gameview in here.
#include "bodebug.h"

#include <kgame/kplayer.h>

#include <QColor>
//Added by qt3to4:
#include <Q3PtrList>

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
 connect(mGame, SIGNAL(signalSideChanged(Player*)),
		this, SIGNAL(signalSideChanged(Player*)));
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

 foreach (Player* p, mGame->allPlayerList()) {
	slotPlayerJoinedGame(p);
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
 boDebug() << k_funcinfo << "there are " << boGame->allPlayerCount() << " players in game now with " << boGame->gamePlayerCount() << " game players" << endl;
 connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*)));

 emit signalPlayerJoinedGame(p);
}

void BosonStartupNetwork::slotPlayerLeftGame(KPlayer* p)
{
 disconnect(p, 0, this, 0);
 emit signalPlayerLeftGame(p);
}

bool BosonStartupNetwork::sendNewGame(BPFPreview* preview, bool editor, const QByteArray* newPlayField)
{
 if (!mGame) {
	BO_NULL_ERROR(mGame);
	return false;
 }
 if (!preview && !newPlayField) {
	BO_NULL_ERROR(preview);
	BO_NULL_ERROR(newPlayField);
	return false;
 }
 if (!mGame->isAdmin()) {
	boError() << k_funcinfo << "only ADMIN is allowed to send this message" << endl;
	return false;
 }
 QByteArray data;
 if (preview) {
	if (!preview->isLoaded()) {
		boError() << k_funcinfo << "playfieldpreview " << preview->identifier() << " has not yet been loaded" << endl;
		return false;
	}
	data = BPFLoader::loadFromDiskToStream(preview->fileName());
	if (data.size() == 0) {
		boError() << k_funcinfo << "no data - saving to stream failed" << endl;
		return false;
	}
 } else {
	data = *newPlayField;
	if (data.size() == 0) {
		boError() << k_funcinfo << "no data provided" << endl;
		return false;
	}
 }

 return sendNewGame(data, editor);
}

bool BosonStartupNetwork::sendNewGame(const QByteArray& data, bool editor)
{
 if (!mGame) {
	BO_NULL_ERROR(mGame);
	return false;
 }
 if (!mGame->isAdmin()) {
	boError() << k_funcinfo << "only ADMIN is allowed to send this message" << endl;
	return false;
 }
 if (data.size() == 0) {
	boError() << k_funcinfo << "empty data. cannot start." << endl;
	return false;
 }

 QByteArray compresseddata = qCompress(data);

 QByteArray buffer;
 QDataStream stream(&buffer, QIODevice::WriteOnly);
 if (editor) {
	stream << (qint8)0;
 } else {
	stream << (qint8)1;
 }
 stream << compresseddata;
 mGame->sendMessage(buffer, BosonMessageIds::IdNewGame);
 return true;
}

bool BosonStartupNetwork::sendLoadGame(const QByteArray& data)
{
 return sendNewGame(data, false);
}

bool BosonStartupNetwork::addNeutralPlayer(bool editor)
{
 if (!mGame) {
	BO_NULL_ERROR(mGame);
	return false;
 }
 if (!mGame->isAdmin()) {
	boError() << k_funcinfo << "only ADMIN is allowed to call this" << endl;
	return false;
 }
 if (mGame->maxPlayers() >= 0) {
	// we increase the limit for the neutral player only!
	mGame->setMaxPlayers(mGame->maxPlayers() + 1);
 }
 Player* neutralPlayer = mGame->addNeutralPlayer();
 if (!neutralPlayer) {
	boError() << k_funcinfo << "unable to add neutral player. cannot send newgame message." << endl;
	return false;
 }

 if (editor) {
	BosonLocalPlayerInput* io = new BosonLocalPlayerInput(false);
	neutralPlayer->addGameIO(io);
	if (!io->initializeIO()) {
		neutralPlayer->removeGameIO(io, true);

		boError() << k_funcinfo << "could not initialize IO for neutral player" << endl;
		return false;
	}
 }
 return true;
}

void BosonStartupNetwork::sendChangeTeamColor(Player* p, const QColor& color)
{
 BO_CHECK_NULL_RET(mGame);
 BO_CHECK_NULL_RET(p);
 QByteArray b;
 QDataStream stream(&b, QIODevice::WriteOnly);
 stream << (quint32)p->bosonId();
 stream << (quint32)color.rgb();
 mGame->sendMessage(b, BosonMessageIds::ChangeTeamColor);
}

void BosonStartupNetwork::sendChangeSpecies(Player* p, const QString& species, const QColor& color)
{
 BO_CHECK_NULL_RET(mGame);
 BO_CHECK_NULL_RET(p);
 QByteArray b;
 QDataStream stream(&b, QIODevice::WriteOnly);
 stream << (quint32)p->bosonId();
 stream << species;
 stream << (quint32)color.rgb();
 mGame->sendMessage(b, BosonMessageIds::ChangeSpecies);
}

void BosonStartupNetwork::sendChangeSide(Player* p, unsigned int sideId)
{
 BO_CHECK_NULL_RET(mGame);
 BO_CHECK_NULL_RET(p);
 QByteArray b;
 QDataStream stream(&b, QIODevice::WriteOnly);
 stream << (quint32)p->bosonId();
 stream << (quint32)sideId;
 mGame->sendMessage(b, BosonMessageIds::ChangeSide);
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
 QDataStream stream(&buffer, QIODevice::WriteOnly);
 // transmit the identifier/name so that the remote newgame dialogs will be able
 // to display the newly selected playfield
 stream << identifier;
 mGame->sendMessage(buffer, BosonMessageIds::ChangePlayField);
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
 mGame->sendMessage(0, BosonMessageIds::IdStartGameClicked);
}

void BosonStartupNetwork::sendChangePlayField(int index)
{
 QStringList list = boData->availablePlayFields();
 QString identifier;
 if (index < 0) {
	// warning: valid in editor mode only!
	identifier = QString();
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
 BPFPreview* preview = 0;
 if (!id.isEmpty()) {
	preview = boData->playFieldPreview(id);
 }

 emit signalPlayFieldChanged(id);
 emit signalPlayFieldChanged(preview);
}

