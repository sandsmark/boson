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

#include "gameoverdialog.h"
#include "gameoverdialog.moc"

#include "boson.h"
#include "player.h"
#include "bosonstatistics.h"

#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qptrdict.h>

class GameOverDialog::GameOverDialogPrivate
{
public:
	GameOverDialogPrivate()
	{
		mBoson = 0;
		mLocalPlayer = 0;

		mWinnerLabel = 0;
		mPlayerLayout = 0;
	}

	Player* mLocalPlayer;
	Boson* mBoson;

	QLabel* mWinnerLabel;
	QHBoxLayout* mPlayerLayout;
	QPtrDict<PlayerBox> mPlayers;
};

GameOverDialog::GameOverDialog(QWidget* parent, bool modal) 
		: KDialogBase(Plain, i18n("Game Over"), Ok, Ok, parent,
		"gameoverdialog", modal, true)
{
 d = new GameOverDialogPrivate;
 setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

 QVBoxLayout* l = new QVBoxLayout(plainPage(), KDialog::spacingHint(), 
		KDialog::marginHint());
 d->mWinnerLabel = new QLabel(plainPage());
 l->addWidget(d->mWinnerLabel);

 d->mPlayerLayout = new QHBoxLayout(l);
}

GameOverDialog::~GameOverDialog()
{
 delete d;
}

void GameOverDialog::createStatistics(Boson* boson, Player* winner, Player* p)
{
 d->mBoson = boson;
 d->mLocalPlayer = p;

 d->mWinnerLabel->setText(i18n("And the winner is: %1").arg(winner->name()));
 
 QPtrList<KPlayer> players = *d->mBoson->playerList();
 QPtrListIterator<KPlayer> it(players);
 while (it.current()) {
	addPlayer((Player*)it.current());
	++it;
 }
 d->mPlayers[winner]->setFrameStyle(QFrame::Box | QFrame::Raised);
 d->mPlayers[winner]->setWinner(true);
}

void GameOverDialog::addPlayer(Player* p)
{
 if (d->mPlayers[p]) {
	return;
 }
 PlayerBox* box = new PlayerBox(p, plainPage());
 d->mPlayerLayout->addWidget(box);
 box->show();
 d->mPlayers.insert(p, box);
}

GameOverDialog::PlayerBox::PlayerBox(Player* p, QWidget* parent) : QFrame(parent)
{
 mPlayer = p;
 setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
 setFrameStyle(Box | Sunken);
 setLineWidth(1);
 QVBoxLayout* l = new QVBoxLayout(this, KDialog::spacingHint(), 
		KDialog::marginHint());
 QLabel* name = new QLabel(p->name(), this);
 l->addWidget(name);

 mMinedMinerals = new QLabel(this);
 l->addWidget(mMinedMinerals);
 mMinedOil = new QLabel(this);
 l->addWidget(mMinedOil);
 mRefinedMinerals = new QLabel(this);
 l->addWidget(mRefinedMinerals);
 mRefinedOil = new QLabel(this);
 l->addWidget(mRefinedOil);
 mProducedUnits = new QLabel(this);
 l->addWidget(mProducedUnits);
 mDestroyedUnits = new QLabel(this);
 l->addWidget(mDestroyedUnits);
 mShots = new QLabel(this);
 l->addWidget(mShots);
 mStatus = new QLabel(this);
 l->addWidget(mStatus);


 BosonStatistics* statistics = p->statistics();
 mMinedMinerals->setText(i18n("Mined Minerals: %1").arg(statistics->minedMinerals()));
 mMinedOil->setText(i18n("Mined Oil: %1").arg(statistics->minedOil()));
 mRefinedMinerals->setText(i18n("Refined Minerals: %1").arg(statistics->refinedMinerals()));
 mRefinedOil->setText(i18n("Refined Oil: %1").arg(statistics->refinedOil()));
 mProducedUnits->setText(i18n("Produced Units: %1").arg(statistics->producedUnits()));
 mDestroyedUnits->setText(i18n("Destroyed Units: %1").arg(statistics->destroyedUnits()));

 // FIXME: this might be > 4 billion and therefore greater than unsigned long
 // int!!
 mShots->setText(i18n("Shots: %1").arg(statistics->shots()));

 setWinner(false);
}

GameOverDialog::PlayerBox::~PlayerBox()
{
}

void GameOverDialog::PlayerBox::setWinner(bool w)
{
 if (w) {
	mStatus->setText(i18n("Won"));
 } else {
	mStatus->setText(i18n("Defeated"));
 }
}
