/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "../../../bomemory/bodummymemory.h"
#include "bosonorderbutton.h"
#include "../../gameengine/unit.h"
#include "../../gameengine/unitplugins/unitplugins.h"
#include "../../gameengine/unitproperties.h"
#include "../../bosonconfig.h"
#include "../../defines.h"
#include "../../boaction.h"
#include "../../gameengine/pluginproperties.h"
#include "../../gameengine/bosonweapon.h"
#include "../../gameengine/speciestheme.h"
#include "../../speciesdata.h"
#include "../../bosonviewdata.h"
#include "bodebug.h"

#include <klocale.h>

#include <qintdict.h>


class BoActionsWidgetPrivate
{
public:
	BoActionsWidgetPrivate()
	{
	}

	QIntDict<BosonOrderButton> mOrderButton;
};

BoActionsWidget::BoActionsWidget()
	: BoUfoWidget()
{
 d = new BoActionsWidgetPrivate;

 setLayoutClass(UGridLayout);
 setGridLayoutColumns(3);
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
		BosonOrderButton* b = new BosonOrderButton();
		b->hide();
		addWidget(b);
		d->mOrderButton.insert(i, b);
		connect(b, SIGNAL(signalAction(const BoSpecificAction&)),
				this, SIGNAL(signalAction(const BoSpecificAction&)));
	}
 }
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
 const QMap<int, QString>* allActionStrings = unit->unitProperties()->allActionStrings();
 for (QMap<int, QString>::const_iterator it = allActionStrings->begin(); it != allActionStrings->end(); ++it) {
	if (it.data().isEmpty()) {
		boError(220) << k_funcinfo << "action ID for " << it.key() << " is empty" << endl;
		continue;
	}
	const BoAction* action = boViewData->speciesData(unit->speciesTheme())->action(it.data());
	if (!action) {
		boError(220) << k_funcinfo << "NULL action for " << it.key() << " == " << it.data() << endl;
		continue;
	}
	boDebug(220) << k_funcinfo << "Adding action: type: " << it.key() << "; id: " <<
			action->id() << "; text: " << action->text() << endl;
	BoSpecificAction a(action);
	a.setAllUnits(allUnits);
	a.setUnit(unit);
	a.setType((UnitAction)it.key());
	actions.append(a);
 }

 // Add weapon actions
 QPtrListIterator<PluginProperties> wit(*(unit->unitProperties()->plugins()));
 for (; wit.current(); ++wit) {
	if (wit.current()->pluginType() == PluginProperties::Weapon) {
		BosonWeaponProperties* w = (BosonWeaponProperties*)wit.current();
		const QMap<int, QString>* actionStrings = w->actionStrings();
		for (QMap<int, QString>::const_iterator it = actionStrings->begin(); it != actionStrings->end(); ++it) {
			if (it.data().isEmpty()) {
				boError(220) << k_funcinfo << "weapon action string for " << it.key() << " is empty" << endl;
				continue;
			}
			const BoAction* action = boViewData->speciesData(w->speciesTheme())->action(it.data());
			if (!action) {
				boError(220) << k_funcinfo << "NULL action for " << it.key() << " == " << it.data() << endl;
				continue;
			}
			BoSpecificAction a(action);
			a.setUnit(unit);
			a.setWeapon(w);
			a.setType((UnitAction)it.key());
			actions.append(a);
		}
	}
 }

 boDebug(220) << k_funcinfo << "actions count: " << actions.count() << endl;

 if (!actions.count()) {
	return;
 }

 ensureButtons(actions.count());
 int button = 0;
 for (QValueList<BoSpecificAction>::iterator it = actions.begin(); it != actions.end(); ++it) {
	boDebug(220) << k_funcinfo << "Setting action for button " << button << endl;
	resetButton(d->mOrderButton[button]);
	d->mOrderButton[button]->setAction(*it);
	button++;
 }
}

void BoActionsWidget::resetButton(BosonOrderButton* button)
{
 button->setProductionCount(0);
 button->setProductionStatus(BosonOrderButton::CanProduce);
}


