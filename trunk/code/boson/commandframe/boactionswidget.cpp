/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "boactionswidget.h"
#include "boactionswidget.moc"

#include "bosonorderbutton.h"
#include "../unit.h"
#include "../unitplugins.h"
#include "../unitproperties.h"
#include "../bosonconfig.h"
#include "../defines.h"

#include <klocale.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qintdict.h>

class BoActionsWidget::BoActionsWidgetPrivate
{
public:
	BoActionsWidgetPrivate()
	{
		mTopLayout = 0;
		mOrderLayout = 0;
	}

	QIntDict<BosonOrderButton> mOrderButton;
	QVBoxLayout* mTopLayout;
	QGridLayout* mOrderLayout;
};

BoActionsWidget::BoActionsWidget(QWidget* parent) : QWidget(parent)
{
 d = new BoActionsWidgetPrivate;
}

BoActionsWidget::~BoActionsWidget()
{
 delete d;
}

void BoActionsWidget::ensureButtons(unsigned int number)
{
 if (d->mOrderButton.count() >= number) {
	return;
 }
 for (unsigned int i = 0; i < number; i++) {
	if (!d->mOrderButton[i]) {
		BosonOrderButton* b = new BosonOrderButton(this);
		b->hide();
		b->setBackgroundOrigin(WindowOrigin);
		d->mOrderButton.insert(i, b);
		connect(b, SIGNAL(signalAction(int)),
				this, SIGNAL(signalAction(int)));
	}
 }
 resetLayout();
}

void BoActionsWidget::resetLayout()
{
 delete d->mOrderLayout;
 delete d->mTopLayout;
 d->mTopLayout = new QVBoxLayout(this);
 int buttons = boConfig->commandButtonsPerRow();
 d->mOrderLayout = new QGridLayout(d->mTopLayout, -1, -1);
 d->mTopLayout->addStretch(1);
 for (unsigned int i = 0; i < d->mOrderButton.count(); i++) {
	BosonOrderButton* b = d->mOrderButton[i];
	d->mOrderLayout->addWidget(b, i / buttons, i % buttons, AlignHCenter);
 }
 int row = ((d->mOrderButton.count() - 1) / buttons) + 1;
 d->mOrderLayout->setRowStretch(row, 1);
d->mOrderLayout->activate();
}

void BoActionsWidget::setButtonsPerRow(int b)
{
 boConfig->setCommandButtonsPerRow(b);
 resetLayout();
}

void BoActionsWidget::hideButtons()
{
 QIntDictIterator<BosonOrderButton> it(d->mOrderButton);
 while (it.current()) {
	it.current()->setUnit(0);
	++it;
 }
}

void BoActionsWidget::showUnitActions(Unit* unit)
{
 kdDebug() << k_funcinfo << endl;
 if (!unit) {
	return;
 }
 int button = 0;
 ensureButtons(6); // 6 is maximum number of actions for now
 hideButtons();

 // Order of action buttons: move, attack, stop. Nothing else yet.
 if(unit->isMobile()) {
	// If it's mobile, it can move
	resetButton(d->mOrderButton[button]);
	d->mOrderButton[button]->setAction(ActionMove, unit->owner());
	button++;
	// and if it can move, it can follow other units
	resetButton(d->mOrderButton[button]);
	d->mOrderButton[button]->setAction(ActionFollow, unit->owner());
	button++;
 }

 if(unit->unitProperties()->canShoot() && unit->weaponDamage() > 0) {
	// It can shoot
	resetButton(d->mOrderButton[button]);
	d->mOrderButton[button]->setAction(ActionAttack, unit->owner());
	button++;
 }

 if(unit->plugin(UnitPlugin::Harvester)) {
	// it can harvest
	/// IDEA: maybe have different icons for mine oil and mine minerals
	resetButton(d->mOrderButton[button]);
	d->mOrderButton[button]->setAction(ActionMine, unit->owner());
	button++;
 }

 // If it can't move or attack, then there's no sense in having stop
 if(button != 0) {
	resetButton(d->mOrderButton[button]);
	d->mOrderButton[button]->setAction(ActionStop, unit->owner());
	button++;
 }

 if(unit->plugin(UnitPlugin::Repair)) {
	// it can harvest
	/// IDEA: maybe have different icons for mine oil and mine minerals
	resetButton(d->mOrderButton[button]);
	d->mOrderButton[button]->setAction(ActionRepair, unit->owner());
	button++;
 }

 d->mTopLayout->activate();
}

void BoActionsWidget::resetButton(BosonOrderButton* button)
{
 button->setProductionCount(0);
 button->setGrayOut(false);
}


