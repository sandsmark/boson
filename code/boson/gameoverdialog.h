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
#ifndef __GAMEOVERDIALOG_H__
#define __GAMEOVERDIALOG_H__

#include <kdialogbase.h>

class Player;
class Boson;

class GameOverDialog : public KDialogBase
{
	Q_OBJECT
public:
	GameOverDialog(QWidget* parent, bool modal = false);
	virtual ~GameOverDialog();

	void createStatistics(Boson* boson, Player* winner, Player* localPlayer);
	
protected:
	void addPlayer(Player* p);

private:
	class PlayerBox;
	class GameOverDialogPrivate;
	GameOverDialogPrivate* d;
};

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
	QLabel* mStatus;
	Player* mPlayer;
	
};

#endif
