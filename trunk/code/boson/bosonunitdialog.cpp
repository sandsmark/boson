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
#include "pluginproperties.h"

#include "global.h"
#include "defines.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <knuminput.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qvbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qvaluelist.h>
#include <qscrollview.h>
#include <qwhatsthis.h>

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
		mArmor = 0;
		mShield = 0;
		mMineralCosts = 0;
		mOilCosts = 0;
		mWeaponRange = 0;
		mSightRange = 0;
		mWeaponDamage = 0;
		mWeaponReload = 0;
		mTerrainType = 0;
		mProductionTime = 0;
		mCanShootAtAir = 0;
		mCanShootAtLand = 0;
		mProducer = 0;
		mSupportMiniMap = 0;
		
		mUnitMobileProperties = 0;
		mSpeed = 0;
		mCanGoOnLand = 0;
		mCanGoOnWater = 0;
		mCanMineMinerals = 0;
		mCanMineOil = 0;
		mMaxResources = 0;
		
		mUnitFacilityProperties = 0;
		mCanProduce = 0;
		mProducerList = 0;

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
	KIntNumInput* mShield;
	KIntNumInput* mArmor;
	KIntNumInput* mMineralCosts;
	KIntNumInput* mOilCosts;
	KIntNumInput* mWeaponRange;
	KIntNumInput* mSightRange;
	KIntNumInput* mWeaponDamage;
	KIntNumInput* mWeaponReload;
	QComboBox* mTerrainType;
	KIntNumInput* mProductionTime;
	QCheckBox* mCanShootAtAir;
	QCheckBox* mCanShootAtLand;
	QComboBox* mProducer;
	QCheckBox* mSupportMiniMap;

	QWidget* mUnitMobileProperties;
	KDoubleNumInput* mSpeed;
	QCheckBox* mCanGoOnLand;
	QCheckBox* mCanGoOnWater;
	QCheckBox* mCanMineMinerals;
	QCheckBox* mCanMineOil;
	KIntNumInput* mMaxResources;

	QWidget* mUnitFacilityProperties;
	QCheckBox* mCanProduce;
	QLineEdit* mProducerList;

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
 delete d->mUnit;
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
 QVBox* topPage = addVBoxPage(i18n("&Properties"));
 QScrollView* scroll = new QScrollView(topPage);
 scroll->setResizePolicy(QScrollView::AutoOneFit);
 QHBox* propertiesPage = new QHBox(scroll->viewport());
 scroll->addChild(propertiesPage);
 QVBox* page = new QVBox(propertiesPage);
 QVBox* specialProperties = new QVBox(propertiesPage);

 d->mUnitType = new QVButtonGroup(i18n("Unit type"), page);
 (void)new QRadioButton(i18n("Is facility"), d->mUnitType);
 (void)new QRadioButton(i18n("Is Mobile"), d->mUnitType);
 d->mUnitType->setRadioButtonExclusive(true);
 connect(d->mUnitType, SIGNAL(pressed(int)), this, SLOT(slotTypeChanged(int)));
 QWhatsThis::add(d->mUnitType, i18n("The type of the unit - either a facility or a mobile unit (like ships, aircrafts or normal ground units). You <b>must</b> choose one."));

 d->mUnitProperties = new QWidget(page);
 QGridLayout* layout = new QGridLayout(d->mUnitProperties, -1, 2, marginHint(), spacingHint());
 
// TODO: range of numimnputs!
// Unit Properties 
 d->mUnitId = new KIntNumInput(d->mUnitProperties);
 d->mUnitId->setLabel(i18n("Unique (!) ID of this unit"), AlignVCenter);
 layout->addMultiCellWidget(d->mUnitId, 0, 0, 0, 1);
 QWhatsThis::add(d->mUnitId, i18n("<p>The ID of the unit type, as used by the map file for example. Note that the ID <b>must</b> be unique for the species theme! You must not have the same ID twice in a single theme!</p>"
		"<p>The ID should be the same for comparable units in different species theme, e.g. an aircraft of a new theme should have the same ID as the aircraft of the basic theme (ID=10004).</p>"
		"<p>Not that except of these restrictions you can use just any positive number (unsigned integer).</p>"));
 
 QLabel* unitNameLabel = new QLabel(i18n("Name of the unit"), d->mUnitProperties);
 layout->addWidget(unitNameLabel, 1, 0);
 d->mUnitName = new QLineEdit(d->mUnitProperties);
 layout->addWidget(d->mUnitName, 1, 1);
 QWhatsThis::add(d->mUnitName, i18n("<p>The name of your unit. The name should be in english - so that it can be translated using one of the standard KDE tools (like kbabel). Keep it short :-)</p>"));
 
 d->mHealth = new KIntNumInput(100, d->mUnitProperties);
 d->mHealth->setLabel(i18n("Health/Power of the unit"), AlignVCenter);
 layout->addMultiCellWidget(d->mHealth, 2, 2, 0, 1);
 QWhatsThis::add(d->mHealth, i18n("<p>Health, power simply how much your unit can take during battle. Higher values mean (surprise) your unit can take more - 0 means your unit is destroyed (so don't use this ;-)).</p>"));

 d->mArmor = new KIntNumInput(0, d->mUnitProperties);
 d->mArmor->setLabel(i18n("Armor of the unit (unused)"), AlignVCenter);
 layout->addMultiCellWidget(d->mArmor, 3, 3, 0, 1);
 QWhatsThis::add(d->mArmor, i18n("<p>Armor is not yet used by Boson. But you can already set a value here - it won't hurt (simply ignored). </p>"));

 d->mShield= new KIntNumInput(0, d->mUnitProperties);
 d->mShield->setLabel(i18n("Shield of the unit (unsupported)"), AlignVCenter);
 layout->addMultiCellWidget(d->mShield, 4, 4, 0, 1);
 QWhatsThis::add(d->mShield, i18n("<p>Shields are not yet used by Boson. But you can already set a value here - it won't hurt (simply ignored). </p>"));

 d->mMineralCosts = new KIntNumInput(d->mUnitProperties);
 d->mMineralCosts->setLabel(i18n("Mineral Costs"), AlignVCenter);
 layout->addMultiCellWidget(d->mMineralCosts, 5, 5, 0, 1);
 QWhatsThis::add(d->mMineralCosts, i18n("<p>How many minerals a player has to pay to produce this unit.</p>"));

 d->mOilCosts = new KIntNumInput(d->mUnitProperties);
 d->mOilCosts->setLabel(i18n("Oil Costs"), AlignVCenter);
 layout->addMultiCellWidget(d->mOilCosts, 6, 6, 0, 1);
 QWhatsThis::add(d->mOilCosts, i18n("<p>How much oil a player has to pay to produce this unit.</p>"));
 
 d->mWeaponRange = new KIntNumInput(d->mUnitProperties);
 d->mWeaponRange->setLabel(i18n("Weapon Range"), AlignVCenter);
 layout->addMultiCellWidget(d->mWeaponRange, 7, 7, 0, 1);
 QWhatsThis::add(d->mWeaponRange, i18n("<p>How far can your unit shoot? This is a cell value, not pixel value. Use a value > 0 if your unit can shoot.</p>"
			"<p>Please note that repairing is also some kind of shooting (with negative damage). Repairing is not yet supported - but you can already specify a value here.</p>"));

 d->mSightRange = new KIntNumInput(d->mUnitProperties);
 d->mSightRange->setLabel(i18n("Sight Range"), AlignVCenter);
 layout->addMultiCellWidget(d->mSightRange, 8, 8, 0, 1);
 QWhatsThis::add(d->mSightRange, i18n("<p>How far can your unit see?</p>"
			"<p>Please note that this is <b>not</b> a pixel value, but a <b>cell number</b>, or tile number! A value of 1 means that 1 field of fog of war is removed around your unit, 2 means 2 fields of fog of war are removed and so on.</p>"
			"<p>A value of 5 (default) is ok - use higher values for rangers</p>"));

 d->mWeaponDamage = new KIntNumInput(d->mUnitProperties);
 d->mWeaponDamage->setLabel(i18n("Damage this unit causes"), AlignVCenter);
 layout->addMultiCellWidget(d->mWeaponDamage, 9, 9, 0, 1);
 QWhatsThis::add(d->mWeaponDamage, i18n("<p>This defines how much damages causes to other unit when shooting at them. Use higher values for better weapons - but you should also increase the <i>Weapon Realod</i> time then!</p>"
			 "<p>You can also use negative values, so that your unit will be ablee to repair (note that this is currently not yet supported)</p>"));
 
 d->mWeaponReload = new KIntNumInput(d->mUnitProperties);
 d->mWeaponReload->setLabel(i18n("Weapon reload"), AlignVCenter);
 layout->addMultiCellWidget(d->mWeaponReload, 10, 10, 0, 1);
 QWhatsThis::add(d->mWeaponReload, i18n("<p>The time your unit needs to reload. This is the number of advance calls which are needed until a unit can shoot again. It is very hard to say how long this actually is as it depends on the game speed.</p>"));

 // TODO: change the cangoon[water|land] values when this is changed!
 QLabel* terrainTypeLabel = new QLabel(i18n("Terrain"), d->mUnitProperties);
 layout->addWidget(terrainTypeLabel, 11, 0);
 d->mTerrainType = new QComboBox(d->mUnitProperties);
 d->mTerrainType->insertItem(i18n("Land"));
 d->mTerrainType->insertItem(i18n("Water"));
 d->mTerrainType->insertItem(i18n("Air"));
 layout->addWidget(d->mTerrainType, 11, 1);
 QWhatsThis::add(d->mTerrainType, i18n("<p>The main terrain your unit is thought to use. You can use only one. For facilities the terrain type is the only terrain it can be placed on. Mobile units use rather the <i>Can go on Land</i> / <i>Can go on Water</i> values.</p>"));

 d->mProductionTime = new KIntNumInput(d->mUnitProperties);
 d->mProductionTime->setLabel(i18n("Production Time"), AlignVCenter);
 layout->addMultiCellWidget(d->mProductionTime, 12, 12, 0, 1);
 QWhatsThis::add(d->mProductionTime, i18n("<p>The numbere of advance calls your unit takes until it is produced. How long this actually is is hard to say, as it depends on your game speed.</p>"));

 QLabel* shootAirLabel = new QLabel(i18n("Can Shoot at Air Units"), d->mUnitProperties);
 layout->addWidget(shootAirLabel, 13, 0);
 d->mCanShootAtAir = new QCheckBox(d->mUnitProperties);
 layout->addWidget(d->mCanShootAtAir, 13, 1);
 QWhatsThis::add(d->mCanShootAtAir, i18n("<p>Defines whether the unit can shoot at air units. This should usually be checked if your unit is an aircraft (except for bombers) or when it is an air defense (like a samsite).</p>"
			"<p>It is not a good idea to use this for every unit, as the aircrafts would loose their advantage then.</p>"));

 QLabel* shootLandLabel = new QLabel(i18n("Can Shoot at Land Units"), d->mUnitProperties);
 layout->addWidget(shootLandLabel, 14, 0);
 d->mCanShootAtLand = new QCheckBox(d->mUnitProperties);
 layout->addWidget(d->mCanShootAtLand, 14, 1);
 QWhatsThis::add(d->mCanShootAtLand, i18n("<p>Whether your unit can shoot at land units. This also means that your unit can shoot at water units</p>"));

 QLabel* producerLabel = new QLabel(i18n("Producer"), d->mUnitProperties);
 layout->addWidget(producerLabel, 15, 0);
 d->mProducer = new QComboBox(d->mUnitProperties);
 d->mProducer->insertItem(i18n("%1 (War Factory)").arg(0));
 d->mProducer->insertItem(i18n("%1 (Shipyard)").arg(1));
 d->mProducer->insertItem(i18n("%1 (Airport)").arg(2));
 d->mProducer->insertItem(i18n("%1 (Command Bunker)").arg(10));
 layout->addWidget(d->mProducer, 15, 1);
 QWhatsThis::add(d->mProducer, i18n("<p>Which type of unit can be used to produce your unit. You can use just any number - just make sure that the factory that is meant to produce this unit has the same number in its <i>Producer List</i>.</p>"));

 QLabel* miniMapLabel = new QLabel(i18n("Supports Mini Map"), d->mUnitProperties);
 layout->addWidget(miniMapLabel, 16, 0);
 d->mSupportMiniMap= new QCheckBox(d->mUnitProperties);
 layout->addWidget(d->mSupportMiniMap, 16, 1);
 QWhatsThis::add(d->mSupportMiniMap, i18n("<p>When this is checked your unit has some kind of radar. You need at least one unit/facility of this type to get a mini map. If the last unit with this option is destroyed you'll loose the mini map again.</p>"));

 
 initMobileProperties(specialProperties);
 initFacilityProperties(specialProperties);


 d->mCreateUnit = new QPushButton(i18n("Create Unit"), topPage);
 QWhatsThis::add(d->mCreateUnit, i18n("<p>This generates the index.desktop file in the specified directory (at the <i>Directories and Files</i> page) using the specified properties (at the <i>Properties</i> page).</p>"));
 connect(d->mCreateUnit, SIGNAL(pressed()), this, SLOT(slotCreateUnit()));
}

void BosonUnitDialog::initMobileProperties(QWidget* page)
{
// Mobile Unit Properties 
 d->mUnitMobileProperties = new QWidget(page);
 d->mUnitMobileProperties->setEnabled(false);
 QGridLayout* layout = new QGridLayout(d->mUnitMobileProperties, -1, 2, marginHint(), spacingHint());
 
 d->mSpeed = new KDoubleNumInput(d->mUnitMobileProperties);
 d->mSpeed->setLabel(i18n("Speed"), AlignVCenter);
 d->mSpeed->setRange(0.0, 100.0);
 layout->addMultiCellWidget(d->mSpeed, 0, 0, 0, 1);
 QWhatsThis::add(d->mSpeed, i18n("<p>The number of pixels that a unit moves forward in every advance call. Don't use too high values and remember that the actual speed depends on the game speed!</p>"));

 QLabel* canGoLandLabel = new QLabel(i18n("Can Go on Land"), d->mUnitMobileProperties);
 layout->addWidget(canGoLandLabel, 1, 0);
 d->mCanGoOnLand = new QCheckBox(d->mUnitMobileProperties);
 layout->addWidget(d->mCanGoOnLand, 1, 1);
 QWhatsThis::add(d->mCanGoOnLand, i18n("<p>Whether this unit can cross land tiles (grass, desert, ...</p>"));

 QLabel* canGoWaterLabel = new QLabel(i18n("Can Go on Water"), d->mUnitMobileProperties);
 layout->addWidget(canGoWaterLabel, 2, 0);
 d->mCanGoOnWater = new QCheckBox(d->mUnitMobileProperties);
 layout->addWidget(d->mCanGoOnWater, 2, 1);
 QWhatsThis::add(d->mCanGoOnWater, i18n("<p>Whether this unit can cross water tiles.</p>"));

 // TODO: setEnabled(false) when d->mCanMineOil is checked
 QLabel* canMineMineralsLabel = new QLabel(i18n("Can Mine Minerals"), d->mUnitMobileProperties);
 layout->addWidget(canMineMineralsLabel, 3, 0);
 d->mCanMineMinerals= new QCheckBox(d->mUnitMobileProperties);
 layout->addWidget(d->mCanMineMinerals, 3, 1);
 QWhatsThis::add(d->mCanMineMinerals, i18n("<p>Whether this unit mine minerals. You usually need only one unit per species theme that can mine minerals (the mineral harvester).</p>"
			"<p>Please note that this cannot be checked if <i>Can Mine Oil</i> was checked!</p>"));

 // TODO: setEnabled(false) when d->mCanMineMinerals is checked
 QLabel* canMineOilLabel = new QLabel(i18n("Can Mine Oil"), d->mUnitMobileProperties);
 layout->addWidget(canMineOilLabel, 3, 0);
 d->mCanMineOil = new QCheckBox(d->mUnitMobileProperties);
 layout->addWidget(d->mCanMineOil, 3, 1);
 QWhatsThis::add(d->mCanMineOil, i18n("<p>Whether this unit mine oil. You usually need only one unit per species theme that can mine oil (the oil harvester).</p>"
			"<p>Please note that this cannot be checked if <i>Can Mine Minerals</i> was checked!</p>"));

 // TODO setEnabled(false) if d->mCanMineMinerals and d->mCanMineMinerals are
 // unchecked!
 d->mMaxResources = new KIntNumInput(d->mUnitMobileProperties);
 d->mMaxResources->setLabel(i18n("Max Resources"), AlignVCenter);
 d->mMaxResources->setRange(0, 10000);
 layout->addMultiCellWidget(d->mMaxResources, 0, 0, 0, 1);
 QWhatsThis::add(d->mMaxResources, i18n("<p>The number of resources your unit can mine before it has to return to a refinery.</p>"
			"<p>This can not be checked if neither <i>Can Mine Minerals</i> nor <i>Can Mine Oil</i> is checked</p>"));

}

void BosonUnitDialog::initFacilityProperties(QWidget* page)
{
// Facility Properties 
 d->mUnitFacilityProperties = new QWidget(page);
 d->mUnitFacilityProperties->setEnabled(false);
 QGridLayout* layout = new QGridLayout(d->mUnitFacilityProperties, -1, 2, marginHint(), spacingHint());

 QLabel* canProduceLabel = new QLabel(i18n("Can Produce (warning - implementation is obsolete)"), d->mUnitFacilityProperties);
 layout->addWidget(canProduceLabel, 0, 0);
 d->mCanProduce = new QCheckBox(d->mUnitFacilityProperties);
 layout->addWidget(d->mCanProduce, 0, 1);
 QWhatsThis::add(d->mCanProduce, i18n("<p>Whether this facility can produce other units/facilities (i.e. is a factory).</p>"));

 QLabel* producerListLabel = new QLabel(i18n("Producer List (comma separated)"), d->mUnitFacilityProperties);
 layout->addWidget(producerListLabel, 1, 0);
 d->mProducerList = new QLineEdit(d->mUnitFacilityProperties);
 layout->addWidget(d->mProducerList, 1, 1);
 QWhatsThis::add(d->mProducerList, i18n("<p>A comma separated list of <i>Producer</i> numbers that this factory can produce.</p>"
			"<p>Usually this is only a single number, e.g. simply &quot;10&quot; for the Command Bunker.</p>"));
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
 delete d->mUnit;
 d->mUnit = new UnitProperties(0);
 d->mUnit->loadUnitType(file);

 loadProperties();
 loadMobileProperties();
 loadFacilityProperties();
 loadPixmaps();
}

void BosonUnitDialog::loadProperties()
{
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
 d->mArmor->setValue(d->mUnit->armor());
 d->mShield->setValue(d->mUnit->shields());
 d->mMineralCosts->setValue(d->mUnit->mineralCost());
 d->mOilCosts->setValue(d->mUnit->oilCost());
 d->mWeaponRange->setValue(d->mUnit->weaponRange());
 d->mSightRange->setValue(d->mUnit->sightRange());
 d->mWeaponDamage->setValue(d->mUnit->weaponDamage());
 d->mWeaponReload->setValue(d->mUnit->reload());
 d->mTerrainType->setCurrentItem(d->mUnit->isLand() ? 0 : d->mUnit->isShip() ?
		1 : d->mUnit->isAircraft() ? 2 : 0);
 d->mProductionTime->setValue(d->mUnit->productionTime());
 d->mCanShootAtAir->setChecked(d->mUnit->canShootAtAirUnits());
 d->mCanShootAtLand->setChecked(d->mUnit->canShootAtLandUnits());
 switch (d->mUnit->producer()) {
	case 0:
		d->mProducer->setCurrentItem(0);
		break;
	case 1:
		d->mProducer->setCurrentItem(1);
		break;
	case 2:
		d->mProducer->setCurrentItem(2);
		break;
	case 10:
		d->mProducer->setCurrentItem(3);
		break;
	default:
		d->mProducer->setCurrentItem(-1);
		kdWarning() << "Value " << d->mUnit->producer() << " not yet supported" << endl;
		break;
 }
 d->mSupportMiniMap->setChecked(d->mUnit->supportMiniMap());
}
 
void BosonUnitDialog::loadMobileProperties()
{
 if (d->mUnit->isFacility()) {
	return;
 }
 d->mSpeed->setValue(d->mUnit->speed());
 d->mCanGoOnLand->setChecked(d->mUnit->canGoOnLand());
 d->mCanGoOnWater->setChecked(d->mUnit->canGoOnWater());
 kdWarning() << k_funcinfo << "pluginproperties are not yet supported!" << endl;
// d->mCanMineMinerals->setChecked(d->mUnit->canMineMinerals());
// d->mCanMineOil->setChecked(d->mUnit->canMineOil());
// d->mMaxResources->setValue(d->mUnit->maxResources());
}

void BosonUnitDialog::loadFacilityProperties()
{
 if (!d->mUnit->isFacility()) {
	return;
 }
 QString producerList;
 ProductionProperties* prod = (ProductionProperties*)d->mUnit->properties(PluginProperties::Production);
 QValueList<int> list;
 d->mCanProduce->setChecked(prod != 0);
 if (prod) {
	list = prod->producerList();
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	if (producerList.length() > 0) {
		producerList += QString::fromLatin1(",");
	}
	producerList += QString::number(list[i]);
 }
 d->mProducerList->setText(producerList);
}

void BosonUnitDialog::loadPixmaps()
{
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

 KSimpleConfig cfg(dir + QString::fromLatin1("index.desktop"));
 saveProperties(&cfg);
}

void BosonUnitDialog::saveProperties(KSimpleConfig* cfg)
{
 bool isFacility = d->mUnitType->find(0)->isDown();
 int terrainType = d->mTerrainType->currentItem();

 cfg->setGroup(QString::fromLatin1("Boson Unit"));
 cfg->writeEntry("IsFacility", isFacility);
 cfg->writeEntry("Name", d->mUnitName->text());
 cfg->writeEntry("Id", (int)d->mUnitId->value());
 cfg->writeEntry("Health", (unsigned long int)d->mHealth->value());
 cfg->writeEntry("Armor", (unsigned long int)d->mArmor->value());
 cfg->writeEntry("Shield", (unsigned long int)d->mShield->value());
 cfg->writeEntry("MineralCost", (unsigned long int)d->mMineralCosts->value());
 cfg->writeEntry("OilCost", (unsigned long int)d->mOilCosts->value());
 cfg->writeEntry("WeaponRange", (unsigned long int)d->mWeaponRange->value());
 cfg->writeEntry("SightRange", (unsigned long int)d->mSightRange->value());
 cfg->writeEntry("WeaponDamage", (long int)d->mWeaponDamage->value());
 cfg->writeEntry("Reload", (unsigned int)d->mWeaponReload->value());
 cfg->writeEntry("TerrainType", (int)d->mTerrainType->currentItem());
 cfg->writeEntry("ProductionTime", (unsigned int)d->mProductionTime->value());
 cfg->writeEntry("CanShootAtAirUnits", (bool)d->mCanShootAtAir->isChecked());
 cfg->writeEntry("CanShootAtAirLand", (bool)d->mCanShootAtLand->isChecked());
 switch (d->mProducer->currentItem()) {
	case 0:
		cfg->writeEntry("Producer", (unsigned int)0);
		break;
	case 1:
		cfg->writeEntry("Producer", (unsigned int)1);
		break;
	case 2:
		cfg->writeEntry("Producer", (unsigned int)2);
		break;
	case 3:
		cfg->writeEntry("Producer", (unsigned int)10);
		break;
	default:
		kdWarning() << "producer index " << d->mProducer->currentItem() << " not yet supported" << endl;
		break;
 }
 cfg->writeEntry("SupportMiniMap", (bool)d->mSupportMiniMap->isChecked());

 if (!isFacility) {
	saveMobileProperties(cfg);
 } else {
	saveFacilityProperties(cfg);
 }
}

void BosonUnitDialog::saveMobileProperties(KSimpleConfig* cfg)
{
 kdDebug() << "Save mobile unit" << endl;
 cfg->setGroup(QString::fromLatin1("Boson Mobile Unit"));
 cfg->writeEntry("Speed", (double)d->mSpeed->value());
 cfg->writeEntry("CanGoOnLand", (bool)d->mCanGoOnLand->isChecked());
 cfg->writeEntry("CanGoOnWater", (bool)d->mCanGoOnWater->isChecked());
 kdWarning() << k_funcinfo << "pluginproperties are not yet supported!" << endl;
// cfg->writeEntry("CanMineMinerals", (bool)d->mCanMineMinerals->isChecked());
// cfg->writeEntry("CanMineOil", (bool)d->mCanMineOil->isChecked());
// cfg->writeEntry("MaxResources", (unsigned int)d->mMaxResources->value());
}

void BosonUnitDialog::saveFacilityProperties(KSimpleConfig* cfg)
{
 kdDebug() << "Save facility" << endl;
 cfg->setGroup(QString::fromLatin1("Boson Facility"));
 cfg->writeEntry("CanProduce", (bool)d->mCanProduce->isChecked());
 
 QStringList list = QStringList::split(QString::fromLatin1(","), d->mProducerList->text());
 QString producerList;
 for (unsigned int i = 0; i < list.count(); i++) {
	bool ok = true;
	int v = list[i].toUInt(&ok);
	if (!ok) {
		KMessageBox::information(this, i18n("Please don't use spaces in the producer list! Skipping entry..."));
	} else {
		if (producerList.length() > 0) {
			producerList += QString::fromLatin1(",");
		}
		producerList += list[i];
	}
	
 }
 cfg->writeEntry("ProducerList", producerList);
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
			switch ((Direction)i) {
				case North:
					text = i18n("North");
					break;
				case NorthEast:
					text = i18n("NorthEast");
					break;
				case East:
					text = i18n("East");
					break;
				case SouthEast:
					text = i18n("SouthEast");
					break;
				case South:
					text = i18n("South");
					break;
				case SouthWest:
					text = i18n("SouthWest");
					break;
				case West:
					text = i18n("West");
					break;
				case NorthWest:
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

