/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "../unit.h"
#include "../unitplugins.h"
#include "../unitproperties.h"
#include "../defines.h"
#include "bodebug.h"

#include <klocale.h>
#include <knuminput.h>

#include <qhbox.h>
#include <qlayout.h>
#include <qlabel.h>

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
	QLabel* mName;
	QLabel* mId;
	KIntNumInput* mHealth;
	KIntNumInput* mShields;
	KIntNumInput* mConstructionStep;

	QLabel* mProductionList;

//	KIntNumInput* mHarvesterMinerals; // TODO once the plugin is ready
//	KIntNumInput* mHarvesterOil; // TODO once the plugin is ready
	KIntNumInput* mResourceMineMinerals;
	KIntNumInput* mResourceMineOil;

	KIntNumInput* mRotation;
};

EditorUnitConfigWidget::EditorUnitConfigWidget(BosonCommandFrameBase* frame, QWidget* parent)
	: BoUnitDisplayBase(frame, parent)
{
 d = new EditorUnitConfigWidgetPrivate;
 d->mUnitId = 0;
 QVBoxLayout* layout = new QVBoxLayout(this);

 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("Name: "), hbox);
 d->mName = new QLabel(hbox);
 layout->addWidget(hbox);

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Id: "), hbox);
 d->mId = new QLabel(hbox);
 layout->addWidget(hbox);

 d->mHealth = new KIntNumInput(this);
 d->mHealth->setLabel(i18n("Health: "), AlignVCenter);
 connect(d->mHealth, SIGNAL(valueChanged(int)), this, SIGNAL(signalUpdateUnit()));
 layout->addWidget(d->mHealth);

 d->mShields = new KIntNumInput(this);
 d->mShields->setLabel(i18n("Shields: "), AlignVCenter);
 connect(d->mShields, SIGNAL(valueChanged(int)), this, SIGNAL(signalUpdateUnit()));
 layout->addWidget(d->mShields);

 d->mConstructionStep = new KIntNumInput(this);
 d->mConstructionStep->setLabel(i18n("Construction step: "), AlignVCenter);
 connect(d->mConstructionStep, SIGNAL(valueChanged(int)), this, SIGNAL(signalUpdateUnit()));
 layout->addWidget(d->mConstructionStep);

 d->mProductionList = new QLabel(i18n("Here would be the production list, if it was implemented"), this);
 layout->addWidget(d->mProductionList);

 d->mResourceMineMinerals = new KIntNumInput(this);
 d->mResourceMineMinerals->setLabel(i18n("Minerals: "), AlignVCenter);
 d->mResourceMineMinerals->setRange(-1, 200000);
 connect(d->mResourceMineMinerals, SIGNAL(valueChanged(int)), this, SIGNAL(signalUpdateUnit()));
 layout->addWidget(d->mResourceMineMinerals);

 d->mResourceMineOil = new KIntNumInput(this);
 d->mResourceMineOil->setLabel(i18n("Oil: "), AlignVCenter);
 connect(d->mResourceMineOil, SIGNAL(valueChanged(int)), this, SIGNAL(signalUpdateUnit()));
 d->mResourceMineOil->setRange(-1, 200000);
 layout->addWidget(d->mResourceMineOil);


 d->mRotation = new KIntNumInput(this);
 d->mRotation->setLabel(i18n("Rotation: "), AlignVCenter);
 d->mRotation->setRange(0, 359);
 connect(d->mRotation, SIGNAL(valueChanged(int)), this, SIGNAL(signalUpdateUnit()));
 layout->addWidget(d->mRotation);

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
 Facility* fac = 0;
 MobileUnit* mob = 0;
 if (unit->isFacility()) {
	fac = (Facility*)unit;
 } else {
	mob = (MobileUnit*)unit;
 }

 d->mHealth->setRange(1, prop->health());
 d->mHealth->setValue(unit->health());
 if (!fac || (fac && fac->constructionSteps() == 0)) {
	d->mConstructionStep->hide();
	d->mConstructionStep->setRange(0, 0);
	d->mConstructionStep->setValue(0);
 } else {
	d->mConstructionStep->show();
	d->mConstructionStep->setRange(0, fac->constructionSteps());
	d->mConstructionStep->setValue(fac->currentConstructionStep());
 }

 d->mShields->setRange(0, prop->shields());
 d->mShields->setValue(unit->shields());
 if (prop->shields() != 0) {
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
	boDebug() << "no resource mine plugin" << endl;
	// hide all widgets related to ResourceMine
	d->mResourceMineMinerals->hide();
	d->mResourceMineOil->hide();
	return;
 }
 if (p->canProvideMinerals()) {
	boDebug() << "ok" << endl;
	d->mResourceMineMinerals->show();
	d->mResourceMineMinerals->setValue(p->minerals());
 } else {
	boDebug() << "cant provide minerals" << endl;
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
 unit->setHealth(d->mHealth->value());
 unit->setShields(d->mShields->value());
 if (unit->isFacility()) {
	Facility* fac = (Facility*)unit;
	if (fac->constructionSteps() != 0) {
		fac->setConstructionStep(d->mConstructionStep->value());
		// If the facility isn't fully constructed yet, we need to set it's work
		//  back to WorkConstructed, so that it's construction will be completed in
		//  the game
		if (fac->currentConstructionStep() < fac->constructionSteps()) {
			fac->setWork(UnitBase::WorkConstructed);
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
	p->setMinerals(d->mResourceMineMinerals->value());
 }
 if (p->canProvideOil()) {
	p->setOil(d->mResourceMineOil->value());
 }
}

