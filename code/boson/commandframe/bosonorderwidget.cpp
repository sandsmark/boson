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

#include "bosonorderwidget.h"
#include "bosonorderwidget.moc"

#include "bosonorderbutton.h"
#include "../unit.h"
#include "../unitplugins.h"
#include "../unitproperties.h"
#include "../bosongroundtheme.h"
#include "../bosonconfig.h"
#include "../defines.h"
#include "bodebug.h"

#include <kstandarddirs.h>
#include <klocale.h>

#include <qlayout.h>
#include <qintdict.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qstring.h>


class BosonOrderWidget::BosonOrderWidgetPrivate
{
public:
	BosonOrderWidgetPrivate()
	{
		mTopLayout = 0;
		mOrderLayout = 0;

		mGroundTheme = 0;
	}

	QIntDict<BosonOrderButton> mOrderButton;
	QVBoxLayout* mTopLayout;
	QGridLayout* mOrderLayout;
	
	BosonGroundTheme* mGroundTheme;

	bool mIsProduceAction;
};

BosonOrderWidget::BosonOrderWidget(QWidget* parent, const char* name) : QWidget(parent, name)
{
 d = new BosonOrderWidgetPrivate;
 d->mIsProduceAction = false;
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
		BosonOrderButton* b = new BosonOrderButton(this);
		b->hide();
		b->setBackgroundOrigin(WindowOrigin);
		d->mOrderButton.insert(i, b);
		connect(b, SIGNAL(signalPlaceGround(unsigned int)),
				this, SLOT(slotPlaceGround(unsigned int)));
		connect(b, SIGNAL(signalAction(const BoSpecificAction&)),
				this, SIGNAL(signalAction(const BoSpecificAction&)));
		connect(b, SIGNAL(signalSelectUnit(Unit*)),
				this, SIGNAL(signalSelectUnit(Unit*)));
	}
 }
 resetLayout();
}

void BosonOrderWidget::resetLayout()
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

void BosonOrderWidget::setButtonsPerRow(int b)
{
 boConfig->setCommandButtonsPerRow(b);
 resetLayout();
}

void BosonOrderWidget::setOrderButtons(QValueList<BoSpecificAction> actions)
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
	d->mOrderButton[i]->setAction(actions[i]);
	d->mTopLayout->activate();
	if (id > 0 && production) {
		pair.first = actions[i].productionType();
		pair.second = actions[i].productionId();
		int count = production->productionList().contains(pair);
		if((actions[i].productionType() == type) && (actions[i].productionId() == id)) {
			d->mOrderButton[i]->advanceProduction(production->productionProgress());
			if (factory->currentPluginType() != UnitPlugin::Production) {
				d->mOrderButton[i]->setProductionCount(-1);
			} else {
				d->mOrderButton[i]->setProductionCount(count);
			}
			d->mOrderButton[i]->setGrayOut(false);
		} else {
			d->mOrderButton[i]->setProductionCount(count);
			d->mOrderButton[i]->setGrayOut(true);
		}
	} else {
		resetButton(d->mOrderButton[i]);
	}
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
 for (unsigned int i = 0; i < d->mGroundTheme->textureCount(); i++) {
	d->mOrderButton[i]->setGround(i, d->mGroundTheme);
 }
}

void BosonOrderWidget::initEditor()
{
 // old code new'ed mInverted and mTransRef here
}

void BosonOrderWidget::hideCellConfigWidgets()
{
 // we should hide widgets for groundtexture mixing here (if they are created)
}

void BosonOrderWidget::showCellConfigWidgets()
{
 // we should show widgets for groundtexture mixing here (if they are created)
}

void BosonOrderWidget::showUnits(QPtrList<Unit> units)
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
 button->setGrayOut(false);
}

bool BosonOrderWidget::isProduceAction() const
{
 return d->mIsProduceAction;
}

void BosonOrderWidget::slotPlaceGround(unsigned int texture)
{
 boDebug(220) << k_funcinfo << endl;
 BO_CHECK_NULL_RET(d->mGroundTheme);
 if (texture >= d->mGroundTheme->textureCount()) {
	boError(220) << k_funcinfo << "invalid texture " << texture << " textureCount="
			<< d->mGroundTheme->textureCount() << endl;
	return;
 }
 unsigned char* alpha = new unsigned char[d->mGroundTheme->textureCount()];
 for (unsigned int i = 0; i < d->mGroundTheme->textureCount(); i++) {
	alpha[i] = 0;
 }
 alpha[texture] = 255;
 emit signalPlaceGround(d->mGroundTheme->textureCount(), alpha);
 delete[] alpha;
}

