/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonunitview.h"
#include "bosonunitview.moc"

#include "../unitbase.h"
#include "../player.h"
#include "../unitproperties.h"
#include "../speciestheme.h"
#include "../unit.h"
#include "bodebug.h"

#include <klocale.h>

#include <qlabel.h>
#include <qvbox.h>

class BosonUnitView::BosonUnitViewPrivate
{
public:
	BosonUnitViewPrivate()
	{
		mName = 0;
		mHealth = 0;
		mOwner = 0;
		mId = 0;

	}

	QLabel* mName;
	QLabel* mHealth;
	QLabel* mOwner;
	QLabel* mId; // mostly for debugging
};

// this unit view consists of 2 parts 
// - the big overview (aka 'preview') pixmap
// - the actual "unit view" - some QLabels describing the selected unit
BosonUnitView::BosonUnitView(QWidget* parent) : BosonOrderButton(parent)
{
 d = new BosonUnitViewPrivate;
 QVBox* v = new QVBox(this);
 d->mName = new QLabel(v);
 d->mHealth = new QLabel(v);
 d->mOwner = new QLabel(v);
 d->mId = new QLabel(v);

 setUnit(0);
}

BosonUnitView::~BosonUnitView()
{
 delete d;
}

void BosonUnitView::setUnit(Unit* u)
{
 UnitBase* unit = (UnitBase*)u;
 BosonOrderButton::setUnit(u);
 if (!unit) {
	return;
 }
 if (!unit->owner()) {
	boError() << k_funcinfo << "no owner" << endl;
	return;
 }
 if (!unit->owner()->speciesTheme()) {
	boError() << k_funcinfo << "No speciesTheme" << endl;
	return;
 }
 d->mName->setText(unit->unitProperties()->name());
 d->mHealth->setText(i18n("Health: %1").arg(unit->health()));
 d->mOwner->setText(i18n("Player: %1").arg(unit->owner()->name()));
 d->mId->setText(i18n("Id : %1").arg(unit->id()));
 showGeneral();
}

void BosonUnitView::displayUnitPixmap(int unitType, Player* owner)
{
 if (!owner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 QPixmap* big = owner->speciesTheme()->bigOverview(unitType);
 if (!big) {
	boError() << "Cannot find Big Overview for " << unitType << endl;
	return;
 }
 setPixmap(*big);
}

void BosonUnitView::hideAll()
{
 d->mName->hide();
 d->mHealth->hide();
 d->mOwner->hide();
 d->mId->hide();
}

void BosonUnitView::showGeneral()
{
 d->mName->show();
 d->mHealth->show();
 d->mOwner->show();
 d->mId->show();
}

void BosonUnitView::slotUnitChanged(Unit* u)
{
 if(unit() != u) {
	return;
 }
 d->mHealth->setText(i18n("Health: %1").arg(u->health()));
 BosonOrderButton::slotUnitChanged(u);
}
