/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef GAMEOVERDIALOG_H
#define GAMEOVERDIALOG_H

#include <kdialogbase.h>

class Player;
class Boson;

/**
 * @short Dialog that displays who won/lost and statistics about the players
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class GameOverDialog : public KDialogBase
{
	Q_OBJECT
public:
	GameOverDialog(QWidget* parent, bool modal = false);
	virtual ~GameOverDialog();

	void createStatistics(Boson* boson, Player* winner, Player* localPlayer);
	
public:
	class PlayerBox;

protected:
	PlayerBox* addPlayer(Player* p);

private:
	class GameOverDialogPrivate;
	GameOverDialogPrivate* d;
};

/**
 * @short A PlayerBox represents a player in the @ref GameOverDialog
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class GameOverDialog::PlayerBox : public QFrame
{
	Q_OBJECT
public:
	PlayerBox(Player* player, QWidget* parent);
	~PlayerBox();

	void setWinner(bool);

private:
	QLabel* mMinedMinerals;
	QLabel* mMinedOil;
	QLabel* mRefinedMinerals;
	QLabel* mRefinedOil;
	QLabel* mProducedUnits;
	QLabel* mShots;
	QLabel* mDestroyedUnits;
	QLabel* mDestroyedOwnUnits;
	QLabel* mLostUnits;
	QLabel* mStatus;

	QLabel* mPoints;
	Player* mPlayer;
	bool mIsWinner;
	
};

#endif
