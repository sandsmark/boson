/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include "bosonunitdialog.h"

#include "unitproperties.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <knuminput.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include <qvbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>

#include "bosonunitdialog.moc"

class BosonUnitDialog::BosonUnitDialogPrivate
{
public:
	BosonUnitDialogPrivate()
	{
		mUnit = 0;
		
		mUnitDir = 0;
		mCreateUnit = 0;
		
		mUnitType = 0;
		
		mUnitProperties = 0;
		mUnitId = 0;
		mUnitName = 0;
		mHealth = 0;
		mPrize = 0;
		mWeaponRange = 0;
		mWeaponDamage = 0;
		mWeaponReload = 0;
		mTerrainType = 0;
		
		mUnitMobileProperties = 0;
		mSpeed = 0;
		
		mUnitFacilityProperties = 0;
	}

	UnitProperties* mUnit;
	
	QPushButton* mUnitDir;
	QPushButton* mCreateUnit;

	QVButtonGroup* mUnitType;
	QWidget* mUnitProperties;
	KIntNumInput* mUnitId;
	QLineEdit* mUnitName;
	KIntNumInput* mHealth;
	KIntNumInput* mPrize;
	KIntNumInput* mWeaponRange;
	KIntNumInput* mWeaponDamage;
	KIntNumInput* mWeaponReload;
	QComboBox* mTerrainType;

	QWidget* mUnitMobileProperties;
	KDoubleNumInput* mSpeed;
	
	QWidget* mUnitFacilityProperties;
};

BosonUnitDialog::BosonUnitDialog(QWidget* parent) 
		: KDialogBase(Tabbed, i18n("Create Unit"), Close, Close, parent,
		0, true, true)
{
 d = new BosonUnitDialogPrivate;
 
 initDirectories();
 initProperties();
}


BosonUnitDialog::~BosonUnitDialog()
{
 if (d->mUnit) {
	delete d->mUnit;
 }
 delete d;
}

void BosonUnitDialog::initDirectories()
{
 QVBox* dirs = addVBoxPage(i18n("&Directories and Files"));
 (void)new QLabel(i18n("Directory where the unit is stored:"), dirs);
 d->mUnitDir = new QPushButton(dirs);
 connect(d->mUnitDir, SIGNAL(pressed()), this, SLOT(slotChangeUnitDir()));

}

void BosonUnitDialog::initProperties()
{
 QVBox* props = addVBoxPage(i18n("&Properties"));

 d->mUnitType = new QVButtonGroup(i18n("Unit type"), props);
 (void)new QRadioButton(i18n("Is facility"), d->mUnitType);
 (void)new QRadioButton(i18n("Is Mobile"), d->mUnitType);
 d->mUnitType->setRadioButtonExclusive(true);
 connect(d->mUnitType, SIGNAL(pressed(int)), this, SLOT(slotTypeChanged(int)));

 d->mUnitProperties = new QWidget(props);
 QGridLayout* layout = new QGridLayout(d->mUnitProperties, -1, 2, marginHint(), spacingHint());
 
// TODO: range of numimnputs!
// Unit Properties 
 d->mUnitId = new KIntNumInput(d->mUnitProperties);
 d->mUnitId->setLabel(i18n("Unique (!) ID of this unit"), AlignVCenter);
 layout->addMultiCellWidget(d->mUnitId, 0, 0, 0, 1);
 
 QLabel* unitNameLabel = new QLabel(i18n("Name of the unit"), d->mUnitProperties);
 layout->addWidget(unitNameLabel, 1, 0);
 d->mUnitName = new QLineEdit(d->mUnitProperties);
 layout->addWidget(d->mUnitName, 1, 1);
 
 d->mHealth = new KIntNumInput(100, d->mUnitProperties);
 d->mHealth->setLabel(i18n("Health/Power of the unit"), AlignVCenter);
 layout->addMultiCellWidget(d->mHealth, 2, 2, 0, 1);

 d->mPrize = new KIntNumInput(d->mUnitProperties);
 d->mPrize->setLabel(i18n("Prize"), AlignVCenter);
 layout->addMultiCellWidget(d->mPrize, 3, 3, 0, 1);
 
 d->mWeaponRange = new KIntNumInput(d->mUnitProperties);
 d->mWeaponRange->setLabel(i18n("Range"), AlignVCenter);
 layout->addMultiCellWidget(d->mWeaponRange, 4, 4, 0, 1);

 d->mWeaponDamage = new KIntNumInput(d->mUnitProperties);
 d->mWeaponDamage->setLabel(i18n("Damage this unit causes"), AlignVCenter);
 layout->addMultiCellWidget(d->mWeaponDamage, 5, 5, 0, 1);
 
 d->mWeaponReload = new KIntNumInput(d->mUnitProperties);
 d->mWeaponReload->setLabel(i18n("Weapon reload"), AlignVCenter);
 layout->addMultiCellWidget(d->mWeaponReload, 6, 6, 0, 1);

 QLabel* terrainTypeLabel = new QLabel(i18n("Terrain"), d->mUnitProperties);
 layout->addWidget(terrainTypeLabel, 7, 0);
 d->mTerrainType = new QComboBox(d->mUnitProperties);
 d->mTerrainType->insertItem(i18n("Land"));
 d->mTerrainType->insertItem(i18n("Water"));
 d->mTerrainType->insertItem(i18n("Air"));
 layout->addWidget(d->mTerrainType, 7, 1);
 

// Mobile Unit Properties 
 d->mUnitMobileProperties = new QWidget(props);
 d->mUnitMobileProperties->setEnabled(false);
 QGridLayout* mobileLayout = new QGridLayout(d->mUnitMobileProperties, -1, 2, marginHint(), spacingHint());
 d->mSpeed = new KDoubleNumInput(d->mUnitMobileProperties);
 d->mSpeed->setLabel(i18n("Speed"), AlignVCenter);
 mobileLayout->addMultiCellWidget(d->mSpeed, 0, 0, 0, 1);


// Facility Properties 
 d->mUnitFacilityProperties = new QWidget(props);
 d->mUnitFacilityProperties->setEnabled(false);

 
 d->mCreateUnit = new QPushButton(i18n("Create Unit"), props);
 connect(d->mCreateUnit, SIGNAL(pressed()), this, SLOT(slotCreateUnit()));
}

void BosonUnitDialog::slotChangeUnitDir()
{
 QString dir = KFileDialog::getExistingDirectory();
 if (dir == QString::null) {
	return;
 }
 d->mUnitDir->setText(dir);

 if (!dir.right(1) != QString::fromLatin1("/")) {
	dir += QString::fromLatin1("/");
 }
 QString file = dir + QString::fromLatin1("index.desktop");
 if (!QFile::exists(file)) {
	return;
 }
 if (d->mUnit) {
	delete d->mUnit;
 }
 d->mUnit = new UnitProperties;
 d->mUnit->loadUnitType(file);

 //TODO set the labels/lineedits/...
 if (d->mUnit->isFacility()) {
	d->mUnitType->setButton(0);
	slotTypeChanged(0);
 } else {
	d->mUnitType->setButton(1);
	slotTypeChanged(1);
 }
 d->mUnitId->setValue(d->mUnit->typeId());
 d->mUnitName->setText(d->mUnit->name());
 d->mHealth->setValue(d->mUnit->health());
 d->mPrize->setValue(d->mUnit->prize());
 d->mWeaponRange->setValue(d->mUnit->range());
 d->mWeaponDamage->setValue(d->mUnit->damage());
 d->mWeaponReload->setValue(d->mUnit->reload());
 d->mTerrainType->setCurrentItem(d->mUnit->isLand() ? 0 : d->mUnit->isShip() ?
		1 : d->mUnit->isAircraft() ? 2 : 0);
 d->mSpeed->setValue(d->mUnit->speed());
 kdDebug() << "hopefully successfully loaded " << d->mUnit->typeId() << endl;
}

void BosonUnitDialog::slotCreateUnit()
{
// I don't want to put these into UnitProperties, as this should be read only.
// perhaps a new class structure is necessary.
 QString dir = d->mUnitDir->text();
 if (dir.isNull()) {
	kdError() << "Select a directory first" << endl;
	return;
 }
 if (!QDir(dir).exists()) {
	kdError() << "Directory " << dir << " does not exist" << endl;
	return;
 }
 if (!dir.right(1) != QString::fromLatin1("/")) {
	dir += QString::fromLatin1("/");
 }
 bool isFacility = d->mUnitType->find(0)->isDown();
 int terrainType = d->mTerrainType->currentItem();

 KSimpleConfig cfg(dir + QString::fromLatin1("index.desktop"));
 cfg.setGroup(QString::fromLatin1("Boson Unit"));
 cfg.writeEntry("IsFacility", isFacility);
 cfg.writeEntry("Name", d->mUnitName->text());
 cfg.writeEntry("Id", (int)d->mUnitId->value());
 cfg.writeEntry("Health", (unsigned long int)d->mHealth->value());
 cfg.writeEntry("Prize", (unsigned long int)d->mPrize->value());
 cfg.writeEntry("Range", (unsigned long int)d->mWeaponRange->value());
 cfg.writeEntry("Damage", (long int)d->mWeaponDamage->value());
 cfg.writeEntry("Reload", (unsigned int)d->mWeaponReload->value());
 cfg.writeEntry("TerrainType", (int)d->mTerrainType->currentItem());

 if (isFacility) {
	kdDebug() << "Save facility" << endl;
	cfg.setGroup(QString::fromLatin1("Boson Facility"));
	cfg.writeEntry("CanProduce", false); // TODO
//	cfg.writeEntry("ProduceList"); // TODO
 } else {
	kdDebug() << "Save mobile unit" << endl;
	cfg.setGroup(QString::fromLatin1("Boson Mobile Unit"));
	cfg.writeEntry("Speed", (double)d->mSpeed->value());
	cfg.writeEntry("CanGoOnLand", (terrainType == 0 || terrainType == 2));
	cfg.writeEntry("CanGoOnWater", (terrainType == 1 || terrainType == 2));
 }

}

void BosonUnitDialog::slotTypeChanged(int id)
{
 if (id == 0) { // is facility
	d->mUnitFacilityProperties->setEnabled(true);
	d->mUnitMobileProperties->setEnabled(false);
 } else { // is mobile unit
	d->mUnitFacilityProperties->setEnabled(false);
	d->mUnitMobileProperties->setEnabled(true);
 }
}

