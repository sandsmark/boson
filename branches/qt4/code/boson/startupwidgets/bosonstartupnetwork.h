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

#ifndef BOSONSTARTUPNETWORK_H
#define BOSONSTARTUPNETWORK_H

#include <qobject.h>

class Boson;
class Player;
class KGamePropertyBase;
class KPlayer;
class BosonPlayField;
class BPFPreview;
class QColor;

// does all the network stuff for startup widgets
class BosonStartupNetwork : public QObject
{
	Q_OBJECT
public:
	BosonStartupNetwork(QObject* parent);
	~BosonStartupNetwork();

	void setGame(Boson* game);

	/**
	 * Add the neutral player that is required before calling @ref
	 * sendNewGame
	 **/
	bool addNeutralPlayer(bool editor);

	/**
	 * @param newPlayField Used for creating new playfields only. This
	 * should contain all the data necessary for starting a game in editor
	 * mode. @p field must be NULL then.
	 **/
	bool sendNewGame(BPFPreview* field, bool editor, const QByteArray* newPlayField = 0);
	bool sendLoadGame(const QByteArray& data);
	void sendChangeTeamColor(Player* p, const QColor& color);
	void sendChangeSpecies(Player* p, const QString& species, const QColor& color);
	void sendChangeSide(Player* p, unsigned int sideId);
	void sendChangePlayerName(Player* p, const QString& name);
	void sendChangePlayField(const QString& playFieldIdentifier);

	/**
	 * Conveninece method. Will send out the identifier of the playfield at
	 * @p index in @ref BosonPlayField::availablePlayFields
	 **/
	void sendChangePlayField(int index);

	void sendStartGameClicked();

	/**
	 * Remove a player from the game. Note that you should not remove the
	 * local player!
	 **/
	void removePlayer(KPlayer* p);

protected slots:
	void slotNetworkMessage(const QByteArray& buffer);
	void slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*);
	void slotPlayerLeftGame(KPlayer*);
	void slotPlayerJoinedGame(KPlayer*);
	void slotPlayFieldChanged(const QString& id);

	void slotUnsetKGame();

protected:
	bool sendNewGame(const QByteArray& data, bool editor);

signals:
	void signalPlayerNameChanged(Player* player);
	void signalPlayerJoinedGame(KPlayer* player);
	void signalPlayerLeftGame(KPlayer* player);
	void signalSpeciesChanged(Player* player);
	void signalSideChanged(Player* player);
	void signalTeamColorChanged(Player* player);
	void signalPlayFieldChanged(const QString& identifier); // obsolete
	void signalPlayFieldChanged(BPFPreview*);
	void signalStartGameClicked();
	void signalSetLocalPlayer(Player* p);
	void signalSetAdmin(bool isAdmin);

private:
	Boson* mGame;
};

#endif
