/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOSONNEWGAMEWIDGET_H
#define BOSONNEWGAMEWIDGET_H

#include "bosonnewgamewidgetbase.h"

#include <qwidget.h>
#include <qptrdict.h>
#include <qcolor.h>
#include <qvaluelist.h>

class BosonStartupNetwork;
class BosonPlayField;
class Player;
class KPlayer;
class KGamePropertyBase;

class BosonNewGameWidgetPrivate;
class BosonNewGameWidget : public BosonNewGameWidgetBase
{
	Q_OBJECT
public:
	BosonNewGameWidget(BosonStartupNetwork* interface, QWidget* parent);
	~BosonNewGameWidget();

	/**
	 * This is the command line interface for adding a computer player. This
	 * is not transferred over network, as the player is added and run on
	 * one computer only, but once that player enters the game it'll appear
	 * on all clients.
	 *
	 * For changing color/species/... of a player or something else use @ref
	 * BosonStartupNetork or @ref networkInterface instead!
	 **/
	void addDummyComputerPlayer();

protected slots: // implementations for the .ui slots
	// these slots describe actions that the local player has executed in
	// his widget. nearly all must be transferred over network before the
	// actual action is performed!
	virtual void slotLocalPlayerNameChanged();
	virtual void slotLocalPlayerColorChanged(int);
	virtual void slotLocalPlayerPlayFieldChanged(QListViewItem*);
	virtual void slotLocalPlayerSpeciesChanged(int);
	virtual void slotLocalPlayerAddedComputerPlayer();
	virtual void slotLocalPlayerRemovedPlayer();
	virtual void slotLocalPlayerHighlightedPlayer(QListBoxItem*);

private slots:
	void slotNetStart();

	/**
	 * Called when some player joins the game
	 * This adds player's name to players' listbox
	 **/
	void slotNetPlayerJoinedGame(KPlayer*);

	/**
	 * Called when some player leaves the game
	 * This removes player's name from players' listbox
	 **/
	void slotNetPlayerLeftGame(KPlayer*);

	/**
	 * Called when some player changes his species
	 * This does nothing at the moment
	 **/
	void slotNetSpeciesChanged(Player*);

	/**
	 * Called when some player changes his color
	 * this calls @ref initColors()
	 **/
	void slotNetColorChanged(Player*);

	void slotNetPlayFieldChanged(BosonPlayField* field);

	/**
	 * Called when the name of a player has been changed
	 **/
	void slotNetPlayerNameChanged(Player*);

	void slotNetSetLocalPlayer(Player*);

	/**
	 * This is used to enable or disable some widgets when user's admin 
	 * status changes
	 **/
	void slotNetSetAdmin(bool);

protected:
	// AB: do NOT move to public!
	Player* localPlayer() const { return mPlayer; }

	BosonStartupNetwork* networkInterface() const { return mNetworkInterface; }
private:
	void initPlayer();
	void initPlayFields();
	void initSpecies();
	void initColors();

private:
	BosonNewGameWidgetPrivate* d;
	KPlayer* mHighlightedPlayer;
	QColor mPlayerColor;
	QValueList<QColor> mAvailableColors;
	int mMap;
	int mMinPlayers;
	int mMaxPlayers;


	Player* mPlayer;

	BosonStartupNetwork* mNetworkInterface;
};

#endif

