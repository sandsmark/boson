/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "../boaction.h"
#include "../pluginproperties.h"
#include "../bosonweapon.h"
#include "bodebug.h"

#include <klocale.h>

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
		connect(b, SIGNAL(signalAction(const BoSpecificAction&)),
				this, SIGNAL(signalAction(const BoSpecificAction&)));
	}
 }
 resetLayout();
}

void BoActionsWidget::resetLayout()
{
 delete d->mOrderLayout;
 delete d->mTopLayout;
 d->mTopLayout = new QVBoxLayout(this);
 int buttons = DEFAULT_CMD_BUTTONS_PER_ROW;
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

void BoActionsWidget::hideButtons()
{
 QIntDictIterator<BosonOrderButton> it(d->mOrderButton);
 while (it.current()) {
	it.current()->setUnit(0);
	++it;
 }
}

void BoActionsWidget::showUnitActions(Unit* unit, const QPtrList<Unit>& allUnits)
{
 boDebug(220) << k_funcinfo << "unit: " << unit << endl;
 if (!unit) {
	boDebug(220) << k_funcinfo << "NULL unit, returning" << endl;
	return;
 }

 hideButtons();

 QValueList<BoSpecificAction> actions;

 // Add all unit actions
 QIntDictIterator<BoAction> it(*unit->unitProperties()->allActions());
 for (; it.current(); ++it) {
	boDebug(220) << k_funcinfo << "Adding action: type: " << it.currentKey() << "; id: " <<
			it.current()->id() << "; text: " << it.current()->text() << endl;
	BoSpecificAction a(it.current());
	a.setAllUnits(allUnits);
	a.setUnit(unit);
	a.setType((UnitAction)it.currentKey());
	actions.append(a);
 }

 // Add weapon actions
 QPtrListIterator<PluginProperties> wit(*(unit->unitProperties()->plugins()));
 for (; wit.current(); ++wit) {
	if (wit.current()->pluginType() == PluginProperties::Weapon) {
		BosonWeaponProperties* w = (BosonWeaponProperties*)wit.current();
		// wait = Weapon Actions ITerator ;-)
		QIntDictIterator<BoAction> wait(*w->actions());
		for (; wait.current(); ++wait) {
			BoSpecificAction a(wait.current());
			a.setUnit(unit);
			a.setWeapon(w);
			a.setType((UnitAction)wait.currentKey());
			actions.append(a);
		}
	}
 }

 boDebug(220) << k_funcinfo << "actions count: " << actions.count() << endl;

 if (!actions.count()) {
	return;
 }

 ensureButtons(actions.count());
 for (unsigned int button = 0; button < actions.count(); button++) {
	boDebug(220) << k_funcinfo << "Setting action for button " << button << endl;
	resetButton(d->mOrderButton[button]);
	d->mOrderButton[button]->setAction(actions[button]);
 }
 boDebug(220) << k_funcinfo << "Activating topLayout" << endl;
 d->mTopLayout->activate();
}

void BoActionsWidget::resetButton(BosonOrderButton* button)
{
 button->setProductionCount(0);
 button->setGrayOut(false);
}


