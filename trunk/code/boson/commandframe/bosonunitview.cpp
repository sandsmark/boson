/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "../../bomemory/bodummymemory.h"
#include "../unitbase.h"
#include "../player.h"
#include "../unitproperties.h"
#include "../speciestheme.h"
#include "../unit.h"
#include "../bosonweapon.h"
#include "bodebug.h"

#include <klocale.h>

class BosonUnitViewPrivate
{
public:
	BosonUnitViewPrivate()
	{
		mName = 0;
		mHealth = 0;
		mOwner = 0;
		mId = 0;
		mArmor = 0;
		mShields = 0;

	}

	BoUfoLabel* mName;
	BoUfoLabel* mHealth;
	BoUfoLabel* mOwner;
	BoUfoLabel* mId; // mostly for debugging
	BoUfoLabel* mArmor;
	BoUfoLabel* mShields;
	BoUfoLabel* mSight;
};

// this unit view consists of 2 parts 
// - the big overview (aka 'preview') pixmap
// - the actual "unit view" - some QLabels describing the selected unit
BosonUnitView::BosonUnitView()
	: BosonOrderButton()
{
 d = new BosonUnitViewPrivate;

 BoUfoVBox* v = new BoUfoVBox();
 addWidget(v);

 d->mName = new BoUfoLabel();
 d->mHealth = new BoUfoLabel();
 d->mOwner = new BoUfoLabel();
 d->mId = new BoUfoLabel();
 d->mArmor = new BoUfoLabel();
 d->mShields = new BoUfoLabel();
 d->mSight = new BoUfoLabel();
 v->addWidget(d->mName);
 v->addWidget(d->mHealth);
 v->addWidget(d->mOwner);
 v->addWidget(d->mId);
 v->addWidget(d->mArmor);
 v->addWidget(d->mShields);
 v->addWidget(d->mSight);

#if 0
 QFont f1, f2;
 f1 = d->mName->font();
 f2.setPointSize(10);
 f2 = d->mName->font();
 f2.setPointSize(9);

 d->mName->setFont(f1);
 d->mHealth->setFont(f1);
 d->mOwner->setFont(f2);
 d->mId->setFont(f2);
 d->mArmor->setFont(f2);
 d->mShields->setFont(f2);
 d->mSight->setFont(f2);
#endif

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
 const Player* owner = unit->owner();
 if (!owner) {
	boError(220) << k_funcinfo << "no owner" << endl;
	return;
 }
 if (!owner->speciesTheme()) {
	boError(220) << k_funcinfo << "No speciesTheme" << endl;
	return;
 }
 d->mName->setText(unit->unitProperties()->name());
 d->mHealth->setText(i18n("Health: %1 (%2)").arg(unit->health()).arg(unit->maxHealth()));
 d->mOwner->setText(i18n("Player: %1").arg(owner->name()));
 d->mId->setText(i18n("Id: %1").arg(unit->id()));
 d->mArmor->setText(i18n("Armor: %1 (%2)").arg(unit->armor()).arg(unit->maxArmor()));
 d->mShields->setText(i18n("Shields: %1 (%2)").arg(unit->shields()).arg(unit->maxShields()));
 d->mSight->setText(i18n("Sight range: %1").arg(unit->sightRange()));
 showGeneral();
}

void BosonUnitView::displayUnitPixmap(unsigned long int unitType, const Player* owner)
{
 if (!owner) {
	boError(220) << k_funcinfo << "NULL owner" << endl;
	return;
 }
 QPixmap* big = owner->speciesTheme()->bigOverview(unitType);
 if (!big) {
	boError(220) << "Cannot find Big Overview for " << unitType << endl;
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
 d->mArmor->hide();
 d->mShields->hide();
 d->mSight->hide();
}

void BosonUnitView::showGeneral()
{
 d->mName->show();
 d->mHealth->show();
 d->mOwner->show();
 d->mId->show();
 d->mArmor->show();
 d->mShields->show();
 d->mSight->show();
}

void BosonUnitView::slotUnitChanged(Unit* u)
{
 if(unit() != u) {
	return;
 }
 d->mHealth->setText(i18n("Health: %1").arg(u->health()));
 d->mArmor->setText(i18n("Armor: %1").arg(u->armor()));
 d->mShields->setText(i18n("Shields: %1 (%2)").arg(u->shields()).arg(u->maxShields()));
 d->mSight->setText(i18n("Sight range: %1").arg(u->sightRange()));
 BosonOrderButton::slotUnitChanged(u);
}

