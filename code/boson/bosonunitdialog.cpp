/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonunitdialog.h"

#include "unitproperties.h"
#include "unit.h" // for Directions

#include "defines.h"

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

class BosonUnitPixmap : public QWidget
{
public:
	BosonUnitPixmap(QWidget* parent) : QWidget(parent)
	{
		QVBoxLayout* l = new QVBoxLayout(this);
		l->setAutoAdd(true);
		mLabel = new QLabel(this);
		mPixmap = new QLabel(this);
		mFileName= new QPushButton(this);
	}
	
	~BosonUnitPixmap()
	{
	}

	void setPixmap(const QString& fileName)
	{
		mPixmap->setPixmap(QPixmap(fileName));
		int pos = fileName.findRev('/') + 1;
		setFileName(fileName.right(fileName.length() - pos));
	}
	
	void setFileName(const QString& fileName)
	{
		mFileName->setText(fileName);
	}
	
	void setPixmap(const QPixmap& pix)
	{
		mPixmap->setPixmap(pix);
	}
	
	void setLabel(const QString& text)
	{
		mLabel->setText(text);
	}

private:
	QLabel* mLabel;
	QLabel* mPixmap;
	QPushButton* mFileName;
};

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
//		mMineralCosts = 0;
		mWeaponRange = 0;
		mWeaponDamage = 0;
		mWeaponReload = 0;
		mTerrainType = 0;
		
		mUnitMobileProperties = 0;
		mSpeed = 0;
		
		mUnitFacilityProperties = 0;

		mDestroyedPixmap = 0;
	}

	UnitProperties* mUnit;
	
	QPushButton* mUnitDir;
	QPushButton* mCreateUnit;

	QVButtonGroup* mUnitType;
	QWidget* mUnitProperties;
	KIntNumInput* mUnitId;
	QLineEdit* mUnitName;
	KIntNumInput* mHealth;
//	KIntNumInput* mMineralCosts;
	KIntNumInput* mWeaponRange;
	KIntNumInput* mWeaponDamage;
	KIntNumInput* mWeaponReload;
	QComboBox* mTerrainType;

	QWidget* mUnitMobileProperties;
	KDoubleNumInput* mSpeed;
	
	QWidget* mUnitFacilityProperties;

	BosonUnitPixmap* mDestroyedPixmap;
	QPtrList<BosonUnitPixmap> mUnitPixmaps;
};

BosonUnitDialog::BosonUnitDialog(QWidget* parent) 
		: KDialogBase(Tabbed, i18n("Create Unit"), Close, Close, parent,
		0, true, true)
{
 d = new BosonUnitDialogPrivate;
 
 initDirectoriesPage();
 initPropertiesPage();
 initPixmapsPage();
}


BosonUnitDialog::~BosonUnitDialog()
{
 if (d->mUnit) {
	delete d->mUnit;
 }
 delete d;
}

void BosonUnitDialog::initDirectoriesPage()
{
 QVBox* dirs = addVBoxPage(i18n("&Directories and Files"));
 (void)new QLabel(i18n("Directory where the unit is stored:"), dirs);
 d->mUnitDir = new QPushButton(dirs);
 connect(d->mUnitDir, SIGNAL(pressed()), this, SLOT(slotChangeUnitDir()));

}

void BosonUnitDialog::initPropertiesPage()
{
 QVBox* page = addVBoxPage(i18n("&Properties"));

 d->mUnitType = new QVButtonGroup(i18n("Unit type"), page);
 (void)new QRadioButton(i18n("Is facility"), d->mUnitType);
 (void)new QRadioButton(i18n("Is Mobile"), d->mUnitType);
 d->mUnitType->setRadioButtonExclusive(true);
 connect(d->mUnitType, SIGNAL(pressed(int)), this, SLOT(slotTypeChanged(int)));

 d->mUnitProperties = new QWidget(page);
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

#warning costs
// d->mMineralCosts = new KIntNumInput(d->mUnitProperties);
// d->mMineralCosts->setLabel(i18n("Mineral Costs"), AlignVCenter);
// layout->addMultiCellWidget(d->mMineralCosts, 3, 3, 0, 1);
 
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
 d->mUnitMobileProperties = new QWidget(page);
 d->mUnitMobileProperties->setEnabled(false);
 QGridLayout* mobileLayout = new QGridLayout(d->mUnitMobileProperties, -1, 2, marginHint(), spacingHint());
 d->mSpeed = new KDoubleNumInput(d->mUnitMobileProperties);
 d->mSpeed->setLabel(i18n("Speed"), AlignVCenter);
 d->mSpeed->setRange(0.0, 100.0);
 mobileLayout->addMultiCellWidget(d->mSpeed, 0, 0, 0, 1);


// Facility Properties 
 d->mUnitFacilityProperties = new QWidget(page);
 d->mUnitFacilityProperties->setEnabled(false);

 
 d->mCreateUnit = new QPushButton(i18n("Create Unit"), page);
 connect(d->mCreateUnit, SIGNAL(pressed()), this, SLOT(slotCreateUnit()));
}

void BosonUnitDialog::initPixmapsPage()
{
 QFrame* page = addPage(i18n("P&ixmaps"));
 unsigned int count = QMAX(PIXMAP_PER_FIX, PIXMAP_PER_MOBILE) - 1;
 int rows = count/3 + 2;
 QGridLayout* l = new QGridLayout(page, rows,  3);
 for (unsigned int i = 0; i < count; i++) {
	BosonUnitPixmap* u = new BosonUnitPixmap(page);
	l->addWidget(u, i / 3, i % 3);
	d->mUnitPixmaps.append(u);
	u->hide();
 }
 d->mDestroyedPixmap = new BosonUnitPixmap(page);
 l->addWidget(d->mDestroyedPixmap, rows -1, 0);
 d->mDestroyedPixmap->setLabel(i18n("Destroyed"));
 
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
// d->mMineralCosts->setValue(d->mUnit->mineralCost());
 d->mWeaponRange->setValue(d->mUnit->range());
 d->mWeaponDamage->setValue(d->mUnit->damage());
 d->mWeaponReload->setValue(d->mUnit->reload());
 d->mTerrainType->setCurrentItem(d->mUnit->isLand() ? 0 : d->mUnit->isShip() ?
		1 : d->mUnit->isAircraft() ? 2 : 0);
 d->mSpeed->setValue(d->mUnit->speed());
 kdDebug() << d->mUnit->speed() << endl;

 QString fileName = d->mUnit->unitPath() + QString("field-%1.png");
 unsigned int pixmaps = d->mUnit->isFacility() ? PIXMAP_PER_FIX : PIXMAP_PER_MOBILE;
 for (unsigned int i = 0; i < pixmaps; i++) {
	QString number;
	number.sprintf("%04d", i);
	if (i != pixmaps - 1) {
		d->mUnitPixmaps.at(i)->setPixmap(fileName.arg(number));
	} else {
		d->mDestroyedPixmap->setPixmap(fileName.arg(number));
	}
 }

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
// cfg.writeEntry("MineralCost", (unsigned long int)d->mMineralCosts->value());
// cfg.writeEntry("OilCost", (unsigned long int)d->mOilCosts->value());
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

	for (unsigned int i = 0; i < d->mUnitPixmaps.count(); i++) {
		if (i < PIXMAP_PER_FIX - 1) {
			d->mUnitPixmaps.at(i)->show();
			d->mUnitPixmaps.at(i)->setLabel(i18n("Step %1").arg(i));
		} else {
			d->mUnitPixmaps.at(i)->hide();
		}
	}
 } else { // is mobile unit
	d->mUnitFacilityProperties->setEnabled(false);
	d->mUnitMobileProperties->setEnabled(true);
	for (unsigned int i = 0; i < d->mUnitPixmaps.count(); i++) {
		if (i < PIXMAP_PER_MOBILE - 1) {
			d->mUnitPixmaps.at(i)->show();
			QString text;
			switch ((Unit::Direction)i) {
				case Unit::North:
					text = i18n("North");
					break;
				case Unit::NorthEast:
					text = i18n("NorthEast");
					break;
				case Unit::East:
					text = i18n("East");
					break;
				case Unit::SouthEast:
					text = i18n("SouthEast");
					break;
				case Unit::South:
					text = i18n("South");
					break;
				case Unit::SouthWest:
					text = i18n("SouthWest");
					break;
				case Unit::West:
					text = i18n("West");
					break;
				case Unit::NorthWest:
					text = i18n("NorthWest");
					break;
				default:
					text = i18n("Unknown");
					break;
					
			}
			d->mUnitPixmaps.at(i)->setLabel(text);
			
		} else {
			d->mUnitPixmaps.at(i)->hide();
		}
	}
 }
}

