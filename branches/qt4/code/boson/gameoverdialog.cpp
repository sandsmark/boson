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

#include "gameoverdialog.h"
#include "gameoverdialog.moc"

#include "../bomemory/bodummymemory.h"
#include "gameengine/boson.h"
#include "gameengine/player.h"
#include "gameengine/bosonstatistics.h"

#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>
#include <q3ptrdict.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <Q3PtrList>

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
	Q3HBoxLayout* mPlayerLayout;
	Q3PtrDict<PlayerBox> mPlayers;
};

GameOverDialog::GameOverDialog(QWidget* parent, bool modal) 
		: KDialog(parent)
{
 d = new GameOverDialogPrivate;
 setWindowTitle(KDialog::makeStandardCaption(i18n("Game Over")));
 setButtons(KDialog::Ok);
 setDefaultButton(KDialog::Ok);
 setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

 Q3VBoxLayout* l = new Q3VBoxLayout(mainWidget(), KDialog::spacingHint(), 
		KDialog::marginHint());
 d->mWinnerLabel = new QLabel(mainWidget());
 l->addWidget(d->mWinnerLabel);

 d->mPlayerLayout = new Q3HBoxLayout(l);
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

 foreach (Player* p, d->mBoson->gamePlayerList()) {
	PlayerBox* winnerBox = addPlayer(p);
	if (p == winner) {
		winnerBox->setFrameStyle(Q3Frame::Box | Q3Frame::Raised);
		winnerBox->setWinner(true);
	} else {
		winnerBox->setWinner(false);
	}
 }
}

GameOverDialog::PlayerBox* GameOverDialog::addPlayer(Player* p)
{
 if (d->mPlayers[p]) {
	return d->mPlayers[p];
 }
 PlayerBox* box = new PlayerBox(p, mainWidget());
 d->mPlayerLayout->addWidget(box);
 box->show();
 d->mPlayers.insert(p, box);
 return box;
}

GameOverDialog::PlayerBox::PlayerBox(Player* p, QWidget* parent) : Q3Frame(parent)
{
 mPlayer = p;
 setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
 setFrameStyle(Box | Sunken);
 setLineWidth(1);
 Q3VBoxLayout* l = new Q3VBoxLayout(this, KDialog::spacingHint(), 
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
 mDestroyedOwnUnits = new QLabel(this);
 l->addWidget(mDestroyedOwnUnits);
 mLostUnits = new QLabel(this);
 l->addWidget(mLostUnits);
 mShots = new QLabel(this);
 l->addWidget(mShots);
 mStatus = new QLabel(this);
 l->addWidget(mStatus);
 mPoints = new QLabel(this);
 l->addWidget(mPoints);


 BosonStatistics* statistics = p->statistics();
 mMinedMinerals->setText(i18n("Mined Minerals: %1").arg(statistics->minedMinerals()));
 mMinedOil->setText(i18n("Mined Oil: %1").arg(statistics->minedOil()));
 mRefinedMinerals->setText(i18n("Refined Minerals: %1").arg(statistics->refinedMinerals()));
 mRefinedOil->setText(i18n("Refined Oil: %1").arg(statistics->refinedOil()));
 mProducedUnits->setText(i18n("Produced Units: %1").arg(statistics->producedUnits()));
 mDestroyedUnits->setText(i18n("Destroyed Units: %1").arg(statistics->destroyedUnits()));
 mDestroyedOwnUnits->setText(i18n("Own Units Destroyed: %1").arg(statistics->destroyedOwnUnits()));
 mLostUnits->setText(i18n("Lost Units: %1").arg(statistics->lostUnits()));

 // FIXME: this might be > 4 billion and therefore greater than unsigned long
 // int!!
 mShots->setText(i18n("Shots: %1").arg(statistics->shots()));
}

GameOverDialog::PlayerBox::~PlayerBox()
{
}

void GameOverDialog::PlayerBox::setWinner(bool w)
{
 BosonStatistics* statistics = mPlayer->statistics();
 long int winnerPoints;
 if (w) {
	mStatus->setText(i18n("Won"));
	winnerPoints = BosonStatistics::winningPoints();
 } else {
	mStatus->setText(i18n("Defeated"));
	winnerPoints = 0;
 }
 // TODO: ranking!
 // e.g.:
 // player achieved 2nd place out of 8 -> the inverse is 7, so he received 7000
 // (or so) points. 
 mPoints->setText(i18n("Points: %1").arg(statistics->points() + winnerPoints));
}

