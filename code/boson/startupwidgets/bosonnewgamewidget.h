/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
class BosonCampaign;
class QListViewItem;

class BosonNewGameWidgetPrivate;
class BosonNewGameWidget : public BosonNewGameWidgetBase
{
	Q_OBJECT
public:
	BosonNewGameWidget(BosonStartupNetwork* interface, QWidget* parent);
	~BosonNewGameWidget();

	void setLocalPlayer(Player* p);

	/**
	 * This is the command line interface for adding a computer player. This
	 * is not transferred over network, as the player is added and run on
	 * one computer only, but once that player enters the game it'll appear
	 * on all clients.
	 *
	 * For changing color/species/... of a player or something else use @ref
	 * BosonStartupNetork or @ref networkInterface instead!
	 **/
	void addAIPlayer();

public slots:
	/**
	 * This is used to enable or disable some widgets when user's admin
	 * status changes
	 **/
	void slotSetAdmin(bool);

	virtual void slotCancel();
	virtual void slotStartGame();
	virtual void slotNetworkOptions();

	/**
	 * Called when server has been intied and is offering connections
	 **/
	void slotOfferingConnections();
	/**
	 * Called before trying to connect to server
	 **/
	void slotConnectingToServer();
	/**
	 * Called when you have successfully connected to server
	 **/
	void slotConnectedToServer();

protected slots: // implementations for the .ui slots
	// these slots describe actions that the local player has executed in
	// his widget. nearly all must be transferred over network before the
	// actual action is performed!
	virtual void slotPlayerNameChanged();
	virtual void slotPlayerColorChanged(int);
	virtual void slotPlayFieldSelected(QListViewItem*);
	virtual void slotPlayerSpeciesChanged(int);
	virtual void slotAddComputerPlayer();
	virtual void slotRemovePlayer();
	virtual void slotPlayerSelected(QListBoxItem*);

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

	/**
	 * This is called when connection to server is broken. It displays chat
	 * message
	 **/
	void slotNetConnectionBroken();

signals:
	void signalShowNetworkOptions();
//	void signalSetLocalPlayer(Player* player);

	/**
	 * Emitted when the player clicks on cancel. The widget should get
	 * hidden now. (back to welcome widget)
	 **/
	void signalCancelled();

protected:
	// AB: do NOT move to public!
	Player* localPlayer() const;

	BosonStartupNetwork* networkInterface() const { return mNetworkInterface; }
private:
	void initLocalPlayer();
	void initPlayFields();
	void initPlayFields(BosonCampaign*, QListViewItem*);
	void initSpecies();
	void updateColors();

	void playersChanged();
	void playerCountChanged();

private:
	BosonNewGameWidgetPrivate* d;
	Player* mSelectedPlayer;
	QColor mLocalPlayerColor;
	unsigned int mMinPlayers;
	unsigned int mMaxPlayers;
	bool mInited;

	BosonStartupNetwork* mNetworkInterface;
};

#endif

