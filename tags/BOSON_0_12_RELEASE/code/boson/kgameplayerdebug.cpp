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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "kgameplayerdebug.h"
#include "kgameplayerdebug.moc"

#include "../bomemory/bodummymemory.h"
#include "boson.h"
#include "player.h"
#include "speciestheme.h"
#include "bodebug.h"

#include <klocale.h>

#include <qvgroupbox.h>
#include <qptrdict.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

class PlayerGroupBox : public QVGroupBox
{
public:
	PlayerGroupBox(QWidget* parent) : QVGroupBox(parent) 
	{
		mSpecies = new QLabel(this);
		mTeamColor = new QLabel(this);

		setSpeciesIdentifier(i18n("Unknown"));
		setTeamColor(QColor(0,0,0));
	}
	void setSpeciesIdentifier(const QString& s)
	{
		mSpecies->setText(i18n("Species Identifier: %1").arg(s));
	}
	void setTeamColor(const QColor& c)
	{
		mTeamColor->setText(i18n("Teamcolor (RGB): %1,%2,%3").
				arg(QString::number(c.red())).
				arg(QString::number(c.green())).
				arg(QString::number(c.blue())));
	}
private:
	QLabel* mSpecies;
	QLabel* mTeamColor;
};

class KGamePlayerDebug::KGamePlayerDebugPrivate
{
public:
	KGamePlayerDebugPrivate()
	{
		mBoson = 0;
		mLocalPlayer = 0;
	}

	Boson* mBoson;
	Player* mLocalPlayer;
	QPtrDict<PlayerGroupBox> mPlayerBoxes;
	QHBoxLayout* mPlayerBoxLayout;
};

KGamePlayerDebug::KGamePlayerDebug(QWidget* parent) : QWidget(parent)
{
 d = new KGamePlayerDebugPrivate;
 d->mPlayerBoxes.setAutoDelete(true);
 QVBoxLayout* topLayout = new QVBoxLayout(this);

 QPushButton* update = new QPushButton(this);
 update->setText(i18n("Update"));
 connect(update, SIGNAL(clicked()), this, SLOT(slotUpdate()));

 d->mPlayerBoxLayout = new QHBoxLayout(topLayout);
 topLayout->addWidget(update);
}

KGamePlayerDebug::~KGamePlayerDebug()
{
 d->mPlayerBoxes.clear();
 delete d;
}

void KGamePlayerDebug::setBoson(Boson* b)
{
 d->mBoson = b;
 slotUpdate();
}

void KGamePlayerDebug::setLocalPlayer(Player* p)
{
 d->mLocalPlayer = p;
 slotUpdate();
}

void KGamePlayerDebug::slotUpdate()
{
 if (!d->mBoson) {
	return;
 }
 /*
 if (!d->mLocalPlayer) {
	return;
 }
 */
 QPtrList<Player> list = *d->mBoson->allPlayerList();
 for (unsigned int i = 0; i < list.count(); i++) {
	Player* player = (Player*)list.at(i);
	PlayerGroupBox* box = d->mPlayerBoxes[player];
	if (!box) {
		box = new PlayerGroupBox(this);
		d->mPlayerBoxes.insert(player, box);
		box->show();
		d->mPlayerBoxLayout->addWidget(box);
	}
	box->setTitle(player->name());
	box->setSpeciesIdentifier(player->speciesTheme()->identifier());
	box->setTeamColor(player->teamColor());
 }
}

