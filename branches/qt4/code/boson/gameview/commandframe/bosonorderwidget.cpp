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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosonorderwidget.h"
#include "bosonorderwidget.moc"

#include "../../../bomemory/bodummymemory.h"
#include "bosonorderbutton.h"
#include "bosoncommandframe.h"
#include "../../gameengine/unit.h"
#include "../../gameengine/unitplugins/unitplugins.h"
#include "../../gameengine/unitproperties.h"
#include "../../gameengine/bosongroundtheme.h"
#include "../../gameengine/playerio.h"
#include "../../boaction.h"
#include "bodebug.h"

#include <klocale.h>

#include <qintdict.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qtimer.h>


class BosonOrderWidgetPrivate
{
public:
	BosonOrderWidgetPrivate()
	{
		mGroundTheme = 0;
		mButtonTimer = 0;
		mCommandFrame = 0;
	}

	QIntDict<BosonOrderButton> mOrderButton;

	BosonGroundTheme* mGroundTheme;
	BosonCommandFrame* mCommandFrame;

	bool mIsProduceAction;

	QTimer* mButtonTimer;
};

BosonOrderWidget::BosonOrderWidget(BosonCommandFrame* cmdframe)
	: BoUfoWidget()
{
 d = new BosonOrderWidgetPrivate;
 d->mIsProduceAction = false;
 d->mCommandFrame = cmdframe;

 setLayoutClass(UGridLayout);
 setGridLayoutColumns(3);
 setMouseEventsEnabled(true, true);

 connect(this, SIGNAL(signalMouseMoved(QMouseEvent*)), this, SLOT(slotMouseMoved(QMouseEvent*)));

 d->mButtonTimer = new QTimer(this);
 connect(d->mButtonTimer, SIGNAL(timeout()), this, SLOT(slotCheckCursor()));
}

BosonOrderWidget::~BosonOrderWidget()
{
 delete d;
}

void BosonOrderWidget::ensureButtons(unsigned int number)
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

		connect(b, SIGNAL(signalPlaceGround(unsigned int)),
				this, SLOT(slotPlaceGround(unsigned int)));
		connect(b, SIGNAL(signalAction(const BoSpecificAction&)),
				this, SIGNAL(signalAction(const BoSpecificAction&)));
		connect(b, SIGNAL(signalSelectUnit(Unit*)),
				this, SIGNAL(signalSelectUnit(Unit*)));
		connect(b, SIGNAL(signalMouseEntered()),
				this, SLOT(slotMouseEnteredButton()));
		connect(b, SIGNAL(signalMouseLeft()),
				this, SLOT(slotMouseLeftButton()));
	}
 }
}

void BosonOrderWidget::setOrderButtons(const QValueList<BoSpecificAction>& actions)
{
 setOrderButtons(actions, QValueList<int>());
}

void BosonOrderWidget::setOrderButtons(const QValueList<BoSpecificAction>& actions, const QValueList<int>& unavailableActions)
{
 boDebug(220) << k_funcinfo << actions.count() << " actions" << endl;

 hideCellConfigWidgets();
 ensureButtons(actions.count());
 hideOrderButtons();

 unsigned long int id = 0;
 ProductionType type = ProduceNothing;
 ProductionPlugin* production = 0;

 Unit* factory = actions.first().unit();
 if (factory) {
	production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
	if (!production) {
		boDebug(220) << k_funcinfo << "factory cannot produce" << endl;
	} else if (production->hasProduction()) {
		type = production->currentProductionType();
		id = production->currentProductionId();
	}
 }

 QPair<ProductionType, long unsigned int> pair;
 for (unsigned int i = 0; i < actions.count(); i++) {
	resetButton(d->mOrderButton[i]);
	d->mOrderButton[i]->setAction(actions[i]);
	if (id > 0 && production) { // production has already started
		pair.first = actions[i].productionType();
		pair.second = actions[i].productionId();
		int count = production->productionList().contains(pair);
		if ((actions[i].productionType() == type) && (actions[i].productionId() == id)) {
			d->mOrderButton[i]->advanceProduction(production->productionProgress());
			if (factory->currentPluginType() != UnitPlugin::Production) {
				d->mOrderButton[i]->setProductionCount(-1);
			} else {
				d->mOrderButton[i]->setProductionCount(count);
			}
			d->mOrderButton[i]->setProductionStatus(BosonOrderButton::CanProduce);
		} else {
			d->mOrderButton[i]->setProductionCount(count);
			d->mOrderButton[i]->setProductionStatus(BosonOrderButton::Producing);
		}
	} else { // no production started yet
		resetButton(d->mOrderButton[i]);
	}
 }

 for (QValueList<int>::const_iterator it = unavailableActions.begin(); it != unavailableActions.end(); ++it) {
	int index = *it;
	if (index < 0 || (unsigned int)index >= actions.count()) {
		boError() << k_funcinfo << "invalid unavailableActions index " << index << endl;
		continue;
	}
	d->mOrderButton[index]->setProductionStatus(BosonOrderButton::CannotProduce);
 }

 d->mIsProduceAction = true;
}

void BosonOrderWidget::hideOrderButtons()
{
 QIntDictIterator<BosonOrderButton> it(d->mOrderButton);
 while (it.current()) {
	it.current()->setUnit(0);
	++it;
 }
}

void BosonOrderWidget::setOrderButtonsGround()
{
 boDebug(220) << k_funcinfo << endl;
 BO_CHECK_NULL_RET(d->mGroundTheme);
 showCellConfigWidgets();
 hideOrderButtons();
 for (unsigned int i = 0; i < d->mGroundTheme->groundTypeCount(); i++) {
	d->mOrderButton[i]->setGround(i, d->mGroundTheme);
 }
}

void BosonOrderWidget::hideCellConfigWidgets()
{
 // we should hide widgets for groundtexture mixing here (if they are created)
}

void BosonOrderWidget::showCellConfigWidgets()
{
 // we should show widgets for groundtexture mixing here (if they are created)
}

void BosonOrderWidget::showUnits(const QPtrList<Unit>& units)
{
 ensureButtons(units.count());
 unsigned int i;
 if (d->mOrderButton.count() > units.count()) {
	for (i = units.count(); i < d->mOrderButton.count(); i++) {
		d->mOrderButton[i]->setUnit(0);
	}
 }
 i = 0;
 QPtrListIterator<Unit> it(units);
 for (; it.current(); ++it, i++) {
	if ((d->mOrderButton[i]->type() == BosonOrderButton::ShowUnit) && (d->mOrderButton[i]->unit() == it.current())) {
		boDebug(220) << "unit already displayed - update..." << endl;
		d->mOrderButton[i]->slotUnitChanged(it.current());
	} else {
//		boDebug(220) << "show unit at " << i << endl;
		d->mOrderButton[i]->setUnit(it.current());
	}
 }
 d->mIsProduceAction = false;
}

void BosonOrderWidget::productionAdvanced(Unit* factory, double percentage)
{
 if (!factory->isFacility()) {
	boError(220) << k_lineinfo << "NOT factory" << endl;
	return;
 }
 ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
 if (!production) {
	boError(220) << k_funcinfo << factory->id() << " cannot produce" << endl;
	return;
 }
 if (!production->hasProduction()) {
	boDebug(220) << k_funcinfo << "no production" << endl;
	return;
 }
 for (unsigned int i = 0; i < d->mOrderButton.count(); i++) {
	BosonOrderButton* c = d->mOrderButton[i];
	if (c->type() == BosonOrderButton::ShowAction && c->action().isProduceAction()) {
		if ((c->productionType() == production->currentProductionType()) && (c->productionId() == production->currentProductionId())) {
			c->advanceProduction(percentage);
		}
	}
 }
}

void BosonOrderWidget::setGroundTheme(BosonGroundTheme* theme)
{
 d->mGroundTheme = theme;
}

void BosonOrderWidget::resetButton(BosonOrderButton* button)
{
 button->setProductionCount(0);
 button->setProductionStatus(BosonOrderButton::CanProduce);
}

bool BosonOrderWidget::isProduceAction() const
{
 return d->mIsProduceAction;
}

void BosonOrderWidget::slotPlaceGround(unsigned int groundtype)
{
 boDebug(220) << k_funcinfo << endl;
 BO_CHECK_NULL_RET(d->mGroundTheme);
 if (groundtype >= d->mGroundTheme->groundTypeCount()) {
	boError(220) << k_funcinfo << "invalid groundtype " << groundtype << " groundTypeCount="
			<< d->mGroundTheme->groundTypeCount() << endl;
	return;
 }
 unsigned char* alpha = new unsigned char[d->mGroundTheme->groundTypeCount()];
 for (unsigned int i = 0; i < d->mGroundTheme->groundTypeCount(); i++) {
	alpha[i] = 0;
 }
 alpha[groundtype] = 255;
 emit signalPlaceGround(d->mGroundTheme->groundTypeCount(), alpha);
 delete[] alpha;
}

void BosonOrderWidget::slotMouseEnteredButton()
{
 BosonOrderButton* button = (BosonOrderButton*)sender();
 boDebug() << k_funcinfo << "button: " << button << endl;
 if (button->type() == BosonOrderButton::ShowAction) {
	const BoSpecificAction& action = button->action();
	if (isProducibleOrPlaceable(action)) {
		const UnitProperties* prop = action.productionOwner()->unitProperties(action.productionId());
		emit signalUnitTypeHighlighted(action.productionOwner(), prop);
	}
 }
}

void BosonOrderWidget::slotMouseLeftButton()
{
 BosonOrderButton* button = (BosonOrderButton*)sender();
 boDebug() << k_funcinfo << "button: " << button << endl;
 if (button->type() == BosonOrderButton::ShowAction) {
	const BoSpecificAction& action = button->action();
	if (isProducibleOrPlaceable(action)) {
		emit signalUnitTypeHighlighted(0, 0);
	}
 }
}

void BosonOrderWidget::slotMouseMoved(QMouseEvent* e)
{
 for (unsigned int i = 0; i < d->mOrderButton.count(); i++) {
	BosonOrderButton* button = d->mOrderButton[i];
	QRect r(QPoint(button->x(), button->y()), QSize(button->width(), button->height()));
	if (!r.contains(e->x(), e->y())) {
		continue;
	}
	if (button->type() != BosonOrderButton::ShowAction) {
		continue;
	}

	const BoSpecificAction& action = button->action();
	if (!isProducibleOrPlaceable(action)) {
		continue;
	}

	BO_CHECK_NULL_RET(action.productionOwner());
	if (action.isProduceAction()) {
		if (action.productionType() == ProduceUnit) {
			const UnitProperties* prop = action.productionOwner()->unitProperties(action.productionId());
			emit signalUnitTypeHighlighted(action.productionOwner(), prop);
			d->mButtonTimer->start(200);
			return;
		} else if (action.productionType() == ProduceTech) {
			const UpgradeProperties* prop = action.productionOwner()->technologyProperties(action.productionId());
			emit signalTechnologyHighlighted(action.productionOwner(), prop);
			d->mButtonTimer->start(200);
			return;
		}
	} else if (action.type() == ActionPlacementPreview) {
		if (action.productionOwner()) { // unit placement
			const UnitProperties* prop = action.productionOwner()->unitProperties(action.productionId());
			emit signalUnitTypeHighlighted(action.productionOwner(), prop);
			d->mButtonTimer->start(200);
			return;
		} else { // ground placement
			// AB: nothing yet.
		}
	}
 }
 emit signalUnitTypeHighlighted(0, 0);
 d->mButtonTimer->stop();
}

void BosonOrderWidget::slotCheckCursor()
{
 BO_CHECK_NULL_RET(d->mCommandFrame->cursorRootPos());
 QRect r(rootLocation(), QSize(width(), height()));
 if (r.contains(*d->mCommandFrame->cursorRootPos())) {
	// TODO: check for children?
	return;
 }
/* for (unsigned int i = 0; i < d->mOrderButton.count(); i++) {
	BosonOrderButton* button = d->mOrderButton[i];
	if (button->hasMouse()) {
		return;
	}
 }*/

 // None of the order buttons has mouse. Disable unit info
 emit signalUnitTypeHighlighted(0, 0);
 d->mButtonTimer->stop();
}

// AB: the name sucks
bool BosonOrderWidget::isProducibleOrPlaceable(const BoSpecificAction& action) const
{
 if (action.isProduceAction() || action.type() == ActionPlacementPreview) {
	return true;
 }
 return false;
}


