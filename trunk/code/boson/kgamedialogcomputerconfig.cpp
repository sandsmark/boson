/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "kgamedialogcomputerconfig.h"

#include "player.h"
#include "bosoncomputerio.h"
#include "speciestheme.h"
#include "boson.h"

#include <klocale.h>
#include <kdebug.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>

#include "kgamedialogcomputerconfig.moc"

class KGameDialogComputerConfig::KGameDialogComputerConfigPrivate
{
public:
	KGameDialogComputerConfigPrivate()
	{
		mName = 0;
	}
	QLineEdit* mName;
};


KGameDialogComputerConfig::KGameDialogComputerConfig(QWidget* parent) : KGameDialogConfig(parent)
{
 d = new KGameDialogComputerConfigPrivate;
 QGridLayout* layout = new QGridLayout(this, -1, 2);

 QLabel* nameLabel = new QLabel(i18n("Name of the player"), this);
 d->mName = new QLineEdit(this);
 d->mName->setText(i18n("Computer"));
 layout->addWidget(nameLabel, 0, 0);
 layout->addWidget(d->mName, 0, 1);
 
 QPushButton* addComputerPlayer = new QPushButton(i18n("&Add Computer Player"), this);
 connect(addComputerPlayer, SIGNAL(pressed()), 
		this, SLOT(slotAddComputerPlayer())); // TODO: name, difficulty, ...
 layout->addMultiCellWidget(addComputerPlayer, 1, 1, 0, 1);
}

KGameDialogComputerConfig::~KGameDialogComputerConfig()
{
 delete d;
}

void KGameDialogComputerConfig::slotAddComputerPlayer()
{
 Boson* g = (Boson*)game();
 if (!g) {
	kdError() << k_funcinfo << "NULL game" << endl;
	return;
 }

 Player* p = new Player;
 QString name = d->mName->text();
 if (name.isNull()) {
	name = i18n("Computer");
 }
 p->setName(name);
 
 // FIXME: MUST be sent over network! problem: at this point the ID of the
 // player is unknown. Possible solution would be to load the theme as soon as
 // the player is being added.
 QRgb color = g->availableTeamColors().first();
 p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), color);

 BosonComputerIO* io = new BosonComputerIO();
 io->setReactionPeriod(50);
 p->addGameIO(io);
 emit signalAddComputerPlayer(p);
}

void KGameDialogComputerConfig::slotSpeciesChanged(int index)
{
 if (index < 0) {
	return;
 }
// emit signalSpeciesChanged(d->mSpeciesIndex2Directory[index]);
}

