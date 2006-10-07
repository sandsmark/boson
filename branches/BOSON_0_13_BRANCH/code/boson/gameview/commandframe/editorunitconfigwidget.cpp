/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "editorunitconfigwidget.h"
#include "editorunitconfigwidget.moc"

#include "../../../bomemory/bodummymemory.h"
#include "../../gameengine/unit.h"
#include "../../gameengine/unitplugins.h"
#include "../../gameengine/unitproperties.h"
#include "../../defines.h"
#include "../../boufo/boufolabel.h"
#include "../../boufo/boufonuminput.h"
#include "bodebug.h"

#include <klocale.h>

class EditorUnitConfigWidgetPrivate
{
public:
	EditorUnitConfigWidgetPrivate()
	{
		mName = 0;
		mId = 0;
		mHealth = 0;
		mShields = 0;
		mConstructionStep = 0;

		mProductionList = 0;
		mResourceMineMinerals = 0;
		mResourceMineOil = 0;

		mRotation = 0;
	}

	unsigned long int mUnitId; // used to check whether we actually have values for the selected unit in updateUnit()
	BoUfoLabel* mName;
	BoUfoLabel* mId;
	BoUfoNumInput* mHealth;
	BoUfoNumInput* mShields;
	BoUfoNumInput* mConstructionStep;

	BoUfoLabel* mProductionList;

//	BoUfoNumInput* mHarvesterMinerals; // TODO
//	BoUfoNumInput* mHarvesterOil; // TODO
	BoUfoNumInput* mResourceMineMinerals;
	BoUfoNumInput* mResourceMineOil;

	BoUfoNumInput* mRotation;
};

EditorUnitConfigWidget::EditorUnitConfigWidget(BosonCommandFrame* frame)
	: BoUnitDisplayBase(frame)
{
 d = new EditorUnitConfigWidgetPrivate;
 d->mUnitId = 0;
 setLayoutClass(UVBoxLayout);

 BoUfoHBox* hbox = new BoUfoHBox();
 addWidget(hbox);
 BoUfoLabel* label = new BoUfoLabel(i18n("Name: "));
 d->mName = new BoUfoLabel();
 hbox->addWidget(label);
 hbox->addWidget(d->mName);

 hbox = new BoUfoHBox();
 addWidget(hbox);
 label = new BoUfoLabel(i18n("Id: "));
 d->mId = new BoUfoLabel();
 hbox->addWidget(label);
 hbox->addWidget(d->mId);

 d->mHealth = new BoUfoNumInput(); // AB: should be an int input
 d->mHealth->setLabel(i18n("Health: "), AlignVCenter);
 connect(d->mHealth, SIGNAL(signalValueChanged(float)), this, SIGNAL(signalUpdateUnit()));
 addWidget(d->mHealth);

 d->mShields = new BoUfoNumInput(); // AB: should be an int input
 d->mShields->setLabel(i18n("Shields: "), AlignVCenter);
 connect(d->mShields, SIGNAL(signalValueChanged(float)), this, SIGNAL(signalUpdateUnit()));
 addWidget(d->mShields);

 d->mConstructionStep = new BoUfoNumInput(); // AB: should be an int input
 d->mConstructionStep->setLabel(i18n("Construction step: "), AlignVCenter);
 connect(d->mConstructionStep, SIGNAL(signalValueChanged(float)), this, SIGNAL(signalUpdateUnit()));
 addWidget(d->mConstructionStep);

 d->mProductionList = new BoUfoLabel(i18n("Here would be the production list, if it was implemented"));
 addWidget(d->mProductionList);

 d->mResourceMineMinerals = new BoUfoNumInput(); // AB: should be an int input
 d->mResourceMineMinerals->setLabel(i18n("Minerals: "), AlignVCenter);
 d->mResourceMineMinerals->setRange(-1, 200000);
 connect(d->mResourceMineMinerals, SIGNAL(signalValueChanged(float)), this, SIGNAL(signalUpdateUnit()));
 addWidget(d->mResourceMineMinerals);

 d->mResourceMineOil = new BoUfoNumInput(); // AB: should be an int input
 d->mResourceMineOil->setLabel(i18n("Oil: "), AlignVCenter);
 connect(d->mResourceMineOil, SIGNAL(signalValueChanged(float)), this, SIGNAL(signalUpdateUnit()));
 d->mResourceMineOil->setRange(-1, 200000);
 addWidget(d->mResourceMineOil);


 d->mRotation = new BoUfoNumInput();
 d->mRotation->setLabel(i18n("Rotation: "), AlignVCenter);
 d->mRotation->setRange(0, 359);
 connect(d->mRotation, SIGNAL(signalValueChanged(float)),
		this, SIGNAL(signalUpdateUnit()));
 addWidget(d->mRotation);

 // AB: some interesting things: configure plugins (e.g. production lists),
 // configure waypoints, configure work (use combobox with real names!)
}

EditorUnitConfigWidget::~EditorUnitConfigWidget()
{
 delete d;
}

bool EditorUnitConfigWidget::display(Unit* unit)
{
 if (!unit) {
	return false;
 }
 blockSignals(true);
 d->mUnitId = unit->id();
 d->mName->setText(unit->name());
 d->mId->setText(QString::number(unit->id()));

 const UnitProperties* prop = unit->unitProperties();
 Unit* facility = 0;
 if (unit->isFacility()) {
	facility = unit;
 }

 // AB: there cases when we _cannot_ set the correct health, due to internal
 // rounding errors. for example consider maxHealth==3
 // -> you cannot set health==1 correctly, as this would require
 //    healthFactor==1/3, which cannot be represented using fixed or float point
 //    variables (neither single nor double precision).
 // --> so if healthFactor is _any_ value < 1/3 (even if it is _very_ close to
 //     1/3) then maxHealth*healthFactor will be 0, not 1. (as casting to
 //     integer just throughs the digits after the decimal point away)
 // in most cases this impreciseness is harmless. but health == 0
 // causes the unit to be unselected and unselectable. so we must avoid that
 // case.
 //
 // we do so by setting a minimum value >= 2.
 // here we use 10 for cosmetic reasons.
 d->mHealth->setRange(QMIN(10, unit->maxHealth()), unit->maxHealth());

 d->mHealth->setValue(unit->health());
 if (!facility || (facility && facility->construction()->constructionSteps() == 0)) {
	d->mConstructionStep->hide();
	d->mConstructionStep->setRange(0, 0);
	d->mConstructionStep->setValue(0);
 } else {
	d->mConstructionStep->show();
	d->mConstructionStep->setRange(0, facility->construction()->constructionSteps());
	d->mConstructionStep->setValue(facility->construction()->currentConstructionStep());
 }

 d->mShields->setRange(0, unit->maxShields());
 d->mShields->setValue(unit->shields());
 if (unit->maxShields() != 0) {
	d->mShields->show();
 } else {
	d->mShields->hide();
 }

 d->mRotation->setValue(unit->rotation());

 displayProductionPlugin(unit);
 displayHarvesterPlugin(unit);
 displayResourceMinePlugin(unit);


 blockSignals(false);
 return true;
}

void EditorUnitConfigWidget::displayProductionPlugin(Unit* unit)
{
 BO_CHECK_NULL_RET(unit);
 ProductionPlugin* p = (ProductionPlugin*)unit->plugin(UnitPlugin::Production);
 if (!p) {
	// hide all widgets related to production
	d->mProductionList->hide();
	return;
 }
 d->mProductionList->show();
 // TODO: display production list widget...
}

void EditorUnitConfigWidget::displayHarvesterPlugin(Unit* unit)
{
 BO_CHECK_NULL_RET(unit);
 HarvesterPlugin* p = (HarvesterPlugin*)unit->plugin(UnitPlugin::Harvester);

 // TODO the HarvesterPlugin will be modified soon, so I dont implement this now
 // (03/12/14)
 // (AB: this comment is obsolete)

 if (!p) {
	// hide all widgets related to harvesting
	return;
 }

}

void EditorUnitConfigWidget::displayResourceMinePlugin(Unit* unit)
{
 BO_CHECK_NULL_RET(unit);
 ResourceMinePlugin* p = (ResourceMinePlugin*)unit->plugin(UnitPlugin::ResourceMine);
 if (!p) {
	// hide all widgets related to ResourceMine
	d->mResourceMineMinerals->hide();
	d->mResourceMineOil->hide();
	return;
 }
 if (p->canProvideMinerals()) {
	d->mResourceMineMinerals->show();
	d->mResourceMineMinerals->setValue(p->minerals());
 } else {
	d->mResourceMineMinerals->hide();
	d->mResourceMineMinerals->setValue(0);
 }
 if (p->canProvideOil()) {
	d->mResourceMineOil->show();
	d->mResourceMineOil->setValue(p->oil());
 } else {
	d->mResourceMineOil->hide();
	d->mResourceMineOil->setValue(0);
 }
}

void EditorUnitConfigWidget::updateUnit(Unit* unit)
{
 BO_CHECK_NULL_RET(unit);
 boDebug(220) << k_funcinfo << endl;
 if (d->mUnitId != unit->id()) {
	boError(220) << k_funcinfo << "Data are for not for the correct unit! data id=" << d->mUnitId << " selected unit: " << unit->id() << endl;
	return;
 }
 unit->setHealth((unsigned long int)d->mHealth->value());
 unit->setShields((unsigned long int)d->mShields->value());
 if (unit->isFacility()) {
	if (unit->construction()->constructionSteps() != 0) {
		unit->construction()->setConstructionStep((unsigned int)d->mConstructionStep->value());
		// If the facility isn't fully constructed yet, we need to set it's work
		//  back to WorkConstructed, so that it's construction will be completed in
		//  the game
		if (unit->construction()->isConstructionComplete()) {
			unit->setAdvanceWork(UnitBase::WorkConstructed);
		}
	}
 }

 unit->setRotation(d->mRotation->value());
 unit->updateRotation();

 updateProductionPlugin(unit);
 updateHarvesterPlugin(unit);
 updateResourceMinePlugin(unit);
}

void EditorUnitConfigWidget::updateProductionPlugin(Unit* unit)
{
 BO_CHECK_NULL_RET(unit);
 ProductionPlugin* p = (ProductionPlugin*)unit->plugin(UnitPlugin::Production);
 if (!p) {
	return;
 }
 // TODO
}

void EditorUnitConfigWidget::updateHarvesterPlugin(Unit* unit)
{
 BO_CHECK_NULL_RET(unit);
 HarvesterPlugin* p = (HarvesterPlugin*)unit->plugin(UnitPlugin::Harvester);

 // TODO the HarvesterPlugin will be modified soon, so I dont implement this now
 // (03/12/14)
 // (AB: this comment is obsolete)

 if (!p) {
	return;
 }
}

void EditorUnitConfigWidget::updateResourceMinePlugin(Unit* unit)
{
 BO_CHECK_NULL_RET(unit);
 ResourceMinePlugin* p = (ResourceMinePlugin*)unit->plugin(UnitPlugin::ResourceMine);
 if (!p) {
	return;
 }
 if (p->canProvideMinerals()) {
	p->setMinerals((unsigned int)d->mResourceMineMinerals->value());
 }
 if (p->canProvideOil()) {
	p->setOil((unsigned int)d->mResourceMineOil->value());
 }
}

