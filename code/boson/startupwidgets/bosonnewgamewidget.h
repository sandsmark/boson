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

#ifndef BOSONNEWGAMEWIDGET_H
#define BOSONNEWGAMEWIDGET_H

#include "bosonstartwidgetbase.h"

#include <qwidget.h>
#include <qptrdict.h>
#include <qcolor.h>
#include <qvaluelist.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KColorCombo;
class KGameChat;
class QComboBox;
class QFrame;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QPushButton;
class QColor;

class Boson;
class Player;
class KPlayer;
class KGamePropertyBase;
class BosonPlayField;

class BosonNewGameWidget : public BosonStartWidgetBase
{
	Q_OBJECT
public:
	BosonNewGameWidget(QWidget* parent);
	~BosonNewGameWidget();

private:
	QComboBox* mColorCombo;
	QLineEdit* mNameEdit;
	QComboBox* mSpeciesCombo;
	QComboBox* mMapCombo;
	QLabel* mMapName;
	QLabel* mSpeciesLabel;
	QGroupBox* mAddAIGroup;
	QLabel* mAddAINameLabel;
	QLineEdit* mAddAIName;
	QPushButton* mAddAIButton;
	QListBox* mPlayersList;
	QPushButton* mRemovePlayerButton;
	QFrame* mLine1;
	KGameChat* mChatWidget;
	QFrame* mLine2;
	QPushButton* mCancelButton;
	QPushButton* mNetworkButton;
	QPushButton* mStartGameButton;


public slots:
	/**
	 * :Called when user clicks on "Start game" button
	 * This widget should then be hided and game should be started
	 **/
	virtual void slotStart();

	/** Adds AI (computer-controlled) player to game
	 * At the moment it uses default species theme and first available color,
	 * in the future, they will be selectable
	 **/
	void slotAddAIPlayer();

	/**
	 * Called when user clicks "Remove player" button
	 * Currently it removes mHighlightedPlayer from game
	 **/
	void slotRemovePlayer();

	/** Called when user clicks "Network Options" button
	 * Hides this widget and shows Network Options widget
	 **/
	void slotNetworkOptions();

	/**
	 * Called when local player's name is changed 
	 **/
	void slotMyNameChanged();

	/**
	 * Called when local player's name is changed 
	 **/
	void slotMyColorChanged(int);

	/**
	 * Called when local player's name is changed 
	 **/
	void slotMySpeciesChanged(int);

	/**
	 * Called when some player joins the game
	 * This adds player's name to players' listbox
	 **/
	void slotPlayerJoinedGame(KPlayer*);

	/**
	 * Called when some player leaves the game
	 * This removes player's name from players' listbox
	 **/
	void slotPlayerLeftGame(KPlayer*);

	/**
	 * Called when some property of some player has changed
	 * Currently it is only used to get notifyed when some player changes 
	 * his name. Player's name in players' listbox is then updated
	 **/
	void slotPropertyChanged(KGamePropertyBase*, KPlayer*);

	/**
	 * Called when user selects some player in players' listbox
	 * "Remove player" button is then disabled or enabled accordingly to if
	 * user can delete that player
	 **/
	void slotPlayerSelected(QListBoxItem*);

	/**
	 * Called when some player changes his species
	 * This does nothing at the moment
	 **/
	void slotSpeciesChanged(Player*);

	/**
	 * Called when some player changes his color
	 * this calls @ref initColors()
	 **/
	void slotColorChanged(Player*);

	/**
	 * This is used to enable or disable some widgets when user's admin 
	 * status changes
	 **/
	void slotSetAdmin(bool);

signals:
	void signalShowNetworkOptions();

	void signalSetLocalPlayer(Player* player);

protected:
	void sendNewGame();
	virtual void setCurrentPlayField(BosonPlayField* field);

	// AB: do NOT move to public!
	Player* localPlayer() const { return mPlayer; }

private:
	QVBoxLayout* mBosonNewGameWidgetLayout;
	QGridLayout* mYourOptionsLayout;

private:
	void initKGame();
	void initPlayer();
	void initPlayFields();
	void initSpecies();
	void initColors();

private:
	QPtrDict<KPlayer> mItem2Player;
	KPlayer* mHighlightedPlayer;
	QColor mPlayercolor;
	QValueList<QColor> mAvailableColors;
	bool mAdmin;
	int mMap;
	int mMinPlayers;
	int mMaxPlayers;

	QMap<int, QString> mSpeciesIndex2Identifier; // index -> species identifier

	Player* mPlayer;
};

#endif

