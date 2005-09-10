/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 Rivo Laks (rivolaks@hot.ee)

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

#include "bouniteditor.h"
#include "bouniteditor.moc"

#include "unitproperties.h"
#include "bosonweapon.h"
#include "pluginproperties.h"
#include "bo3dtools.h"
#include "bofiledialog.h"
#include "bosonsearchpathswidget.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>
#include <knuminput.h>
#include <klineedit.h>

#include <qstringlist.h>
#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>
#include <qvalidator.h>
#include <qfileinfo.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qcombobox.h>

static QString listToString(const QValueList<unsigned long int>& list)
{
 QString str;
 QValueList<unsigned long int>::const_iterator it;
 for(it = list.begin(); it != list.end(); ++it) {
	str = str + "," + QString::number(*it);
 }
 if(!str.isEmpty()) {
	str.remove(0, 1);
 }
 return str;
}

static QValueList<unsigned long int> stringToList(const QString& str)
{
 QValueList<unsigned long int> list;
 QStringList strlist = QStringList::split(",", str);
 QStringList::iterator it;
 for(it = strlist.begin(); it != strlist.end(); ++it) {
	list.append((*it).toULong());
 }
 return list;
}


BoUnitEditor::BoUnitEditor(QWidget* parent)
	: BoUnitEditorBase(parent)
{
 init();
}

BoUnitEditor::~BoUnitEditor()
{
 // Save config
 KConfig* cfg = kapp->config();
 cfg->setGroup("Boson Unit Editor");
 cfg->writeEntry("SearchPaths", mSearchPaths->currentPaths());
}

void BoUnitEditor::init()
{
 mUnitLoaded = false; // Bad hack
 mUnit = new UnitProperties(false);
 mSearchPaths = new BosonSearchPathsWidget;
 mCurrentWeapon = -1;
 connect(mSearchPaths->mOkButton, SIGNAL(clicked()), this, SLOT(slotHideSearchPaths()));
 mSearchPaths->hide();
 // Load search paths
 KConfig* cfg = kapp->config();
 cfg->setGroup("Boson Unit Editor");
 QStringList paths = cfg->readListEntry("SearchPaths");
 mSearchPaths->slotSetPaths(paths);
 // Load known units
 loadUnitsList();

 // Validator for lineedits with integer lists
 QRegExpValidator* v = new QRegExpValidator(QRegExp("[0-9,]*"), this);
 mUnitProducerList->setValidator(v);
 mUnitRequirements->setValidator(v);
 mUnitDestroyedEffects->setValidator(v);
 mUnitConstructedEffects->setValidator(v);
 mWeaponShootEffects->setValidator(v);
 mWeaponFlyEffects->setValidator(v);
 mWeaponHitEffects->setValidator(v);

 // Clear everything to default values
 if(mUnits.isEmpty()) {
	mUnit->reset();
 } else {
	mUnitsList->setSelected(0, true);
 }
 mConfigChanged = false;
 updateConfigWidgets();
}



void BoUnitEditor::slotTypeChanged()
{
 if(mUnitTypeFacility->isChecked()) {
	// Type is facility
	mUnitCanHarvest->setEnabled(false);
	mMobileOptions->setEnabled(false);
	mFacilityOptions->setEnabled(true);
 } else {
	// Type is mobile
	mUnitCanHarvest->setEnabled(true);
	mMobileOptions->setEnabled(true);
	mFacilityOptions->setEnabled(false);
 }
}

void BoUnitEditor::slotUnitSelected( int index)
{
 // Ask is user wants to save current properties
 if(mUnitLoaded && mConfigChanged) {
	int answer = KMessageBox::questionYesNoCancel(this,
			i18n("Do you want to save current unit properties?"),
			i18n("Save current unit?"));
	if(answer == KMessageBox::Cancel) {
		return;
	} else if(answer == KMessageBox::Yes) {
		slotSaveUnit();
	}
 }
 slotLoadUnit(mUnits[index]);
}

void BoUnitEditor::slotAddTexture()
{
 if(mUnitTextureFrom->text().isEmpty() || mUnitTextureTo->text().isEmpty()) {
	return;
 }
 (void)new QListViewItem(mUnitTexturesList, mUnitTextureFrom->text(),
		mUnitTextureTo->text());
 mUnitTextureFrom->clear();
 mUnitTextureTo->clear();
 mConfigChanged = true;
 updateConfigWidgets();
}

void BoUnitEditor::slotRemoveTexture()
{
 delete mUnitTexturesList->currentItem();
 mConfigChanged = true;
 updateConfigWidgets();
}

void BoUnitEditor::slotCurrentTextureChanged()
{
 mUnitTextureRemove->setEnabled(mUnitTexturesList->currentItem() != 0);
}

void BoUnitEditor::slotAutoPickId()
{
 // Check if unit is facility or mobile; if facility, iterate through all
 //  facilities and pick next free id > 0; if it's mobile, iterate through all
 //  mobiles and pick next free id >= 10000
 int id;
 if(mUnitTypeFacility->isChecked()) {
	id = 1;
 } else {
	id = 10000;
 }
 while(mUsedIds.contains(id) > 0) {
	id++;
 }
 mUnitId->setValue(id);
 mConfigChanged = true;
 updateConfigWidgets();
}

void BoUnitEditor::slotSaveUnit()
{
 // Save unit
 // Check for errors
 QStringList errors = verifyProperties();
 if(!errors.isEmpty()) {
	QString allErrors = errors.join("\n");
	KMessageBox::sorry(this, i18n("Sorry, but current properties cannot be saved "
			"until following errors are fixed:\n\n%1").arg(allErrors),
			i18n("Current properties contain errors!"));
	return;
 }
 // Save properties
 QString dir = mUnitPath->text();
 QDir d;
 if(dir.isEmpty()) {
	dir = BoFileDialog::getExistingDirectory(QString::null, 0,
			i18n("Please select directory for your new unit"));
	if(dir.isEmpty()) { // No directory was chosen
		return;
	}
	if(mSearchPaths->currentPaths().contains(dir) == 0) {
		mSearchPaths->slotAppendPath(dir);
		loadUnitsList();
	}
 }
 d.setPath(dir);
 if(!d.exists()) {
	KMessageBox::error(this, i18n("Directory %1 does not exist!").arg(dir),
			i18n("Invalid directory"));
	return;
 }
 QFileInfo fi(d.absPath());
 if(!fi.isWritable()) {
	KMessageBox::error(this, i18n("Directory %1 is not writable!").arg(dir),
			i18n("Directory isn't writable"));
	return;
 }
 updateUnitProperties();
 mUnit->saveUnitType(d.absPath() + "/index.unit");
 mConfigChanged = false;
 updateConfigWidgets();
}

void BoUnitEditor::slotNewUnit()
{
 // Ask is user wants to save current properties
 if(mConfigChanged) {
	int answer = KMessageBox::questionYesNoCancel(this,
			i18n("Do you want to save current unit properties?"),
			i18n("Save current unit?"));
	if(answer == KMessageBox::Cancel) {
		return;
	} else if(answer == KMessageBox::Yes) {
		slotSaveUnit();
	}
 }
 // Clear everything to default values
 mUnit->reset();
 updateWidgets();
 mConfigChanged = false;
 updateConfigWidgets();
}

QStringList BoUnitEditor::verifyProperties()
{
 QStringList errors;
 if(mUnitName->text().isEmpty()) {
	errors += i18n("* Unit's name cannot be empty");
 }
 if(mUnitId->value() == 0) {
	errors += i18n("* Unit's id must not be 0");
 }
 if(mUnitHarvestMinerals->isChecked() && mUnitHarvestOil->isChecked()) {
	errors += i18n("* Unit cannot mine *both* oil and minerals");
 }
 if(mUnitRefineMinerals->isChecked() && mUnitRefineOil->isChecked()) {
	errors += i18n("* Unit cannot refine *both* oil and minerals");
 }
 if(mUnitCanHarvest->isChecked() && mUnitCanRefine->isChecked()) {
	errors += i18n("* Unit cannot mine *and* harvest");
 }
 return errors;
}

void BoUnitEditor::slotLoadUnit( QString dir )
{
 mUnit->reset();
 mUnit->loadUnitType(dir + "/index.unit", false);
 updateWidgets();
 mUnitLoaded = true;
 // If unit has any weapons, make the first one selected
 if(mUnit->canShoot()) {
	mWeaponsList->setSelected(0, true);
 }
 mConfigChanged = false;
 updateConfigWidgets();
}

void BoUnitEditor::slotEditSearchPaths()
{
 // Can't we do this more simply?
 mSearchPaths->show();
 mSearchPaths->setActiveWindow();
 mSearchPaths->raise();
}

void BoUnitEditor::slotHideSearchPaths()
{
 mSearchPaths->hide();
 loadUnitsList();
}

void BoUnitEditor::loadUnitsList()
{
 mUsedIds.clear();
 mUnits.clear();
 mUnitsList->clear();

 QStringList paths = mSearchPaths->currentPaths();
 QStringList units;  // This contains all directories where all known units are stored
 int pos = 0;

 for (QStringList::Iterator it = paths.begin(); it != paths.end(); ++it ) {
	QDir dir(*it);
	QStringList dirList = dir.entryList(QDir::Dirs);
	for (QStringList::Iterator it2 = dirList.begin(); it2 != dirList.end(); ++it2 ) {
		if((*it2 == "..") || (*it2 == ".")) {
			continue;
		}
		QString file = dir.path() + "/" + *it2;
		if(QFile::exists(file + "/index.unit")) {
			units.append(file);
		}
	}
	if(QFile::exists(dir.path() + "/index.unit")) {
		units.append(dir.path());
	}
 }

 for (QStringList::Iterator it = units.begin(); it != units.end(); ++it) {
	KSimpleConfig cfg(*it + "/index.unit");
	cfg.setGroup("Boson Unit");
	QString name = cfg.readEntry("Name", i18n("Unknown"));
	int id = cfg.readLongNumEntry("Id", 0);
	mUsedIds.append(id);

	mUnits.insert(pos, *it);
	mUnitsList->insertItem(name + QString(" (%1)").arg(id), pos);
	pos++;
 }
}

void BoUnitEditor::slotOpenUnit()
{
 // Ask is user wants to save current properties
 if(mConfigChanged) {
	int answer = KMessageBox::questionYesNoCancel(this,
			i18n("Do you want to save current unit properties?"),
			i18n("Save current unit?"));
	if(answer == KMessageBox::Cancel) {
		return;
	} else if(answer == KMessageBox::Yes) {
		slotSaveUnit();
	}
 }

 // Open new unit
 QString dir = BoFileDialog::getExistingDirectory();
 if((dir == QString::null) || (!QFile::exists(dir + "/index.unit"))) {
	KMessageBox::error(this, i18n("No unit configuration file was found in this directory!"), i18n("Invalid directory!"));
	return;
 }
 if(mSearchPaths->currentPaths().contains(dir) == 0) {
	mSearchPaths->slotAppendPath(dir);
	loadUnitsList();
 }
 slotLoadUnit(dir);
}




void BoUnitEditor::updateUnitProperties()
{
 mUnit->clearPlugins(true);
 // General page
 mUnit->setName(mUnitName->text());
 mUnit->setTypeId(mUnitId->value());
 mUnit->setUnitWidth((mUnitWidth->value()));
 mUnit->setUnitHeight((mUnitHeight->value()));
 mUnit->setUnitDepth(mUnitDepth->value());

 // Mobile/facility properties
 if(mUnitTypeFacility->isChecked()) {
	mUnit->setIsFacility(true);
	mUnit->setConstructionSteps(mUnitConstructionSteps->value());
 } else {
	mUnit->setIsFacility(false);
	mUnit->insertBoFixedBaseValue(bofixed(mUnitSpeed->value()), "Speed");
	mUnit->setCanGoOnLand(mUnitCanGoOnLand->isChecked());
	mUnit->setCanGoOnWater(mUnitCanGoOnWater->isChecked());
 }
 // Properties page
 mUnit->insertULongBaseValue(mUnitHealth->value(), "Health");
 mUnit->insertULongBaseValue(mUnitArmor->value(), "Armor");
 mUnit->insertULongBaseValue(mUnitShields->value(), "Shields");
 mUnit->insertULongBaseValue(mUnitMineralCost->value(), "MineralCost");
 mUnit->insertULongBaseValue(mUnitOilCost->value(), "OilCost");
 mUnit->insertULongBaseValue(mUnitSight->value(), "SightRange");
 mUnit->setTerrainType((UnitProperties::TerrainType)(mUnitTerrain->currentItem()));
 mUnit->setSupportMiniMap(mUnitSupportMiniMap->isChecked());
 // Weapons page
 // Sync current weapon first
 updateWeaponProperties();
 QPtrListIterator<BosonWeaponProperties> it(mWeapons);
 while(it.current() != 0) {
	mUnit->addPlugin(it.current());
	++it;
 }
 // Plugins page
 if(mUnitCanProduce->isChecked()) {
	ProductionProperties* p = new ProductionProperties(mUnit);
	p->setProducerList(stringToList(mUnitProducerList->text()));
	mUnit->addPlugin(p);
 }
 if(mUnitCanHarvest->isChecked()) {
	HarvesterProperties* p = new HarvesterProperties(mUnit);
	p->setCanMineMinerals(mUnitHarvestMinerals->isChecked());
	p->setCanMineOil(mUnitHarvestOil->isChecked());
	p->setMaxResources(mUnitMaxResource->value());
	p->setMiningSpeed(mUnitMiningSpeed->value());
	p->setUnloadingSpeed(mUnitUnloadingSpeed->value());
	mUnit->addPlugin(p);
 }
 if(mUnitCanRefine->isChecked()) {
	RefineryProperties* p = new RefineryProperties(mUnit);
	p->setCanRefineMinerals(mUnitRefineMinerals->isChecked());
	p->setCanRefineOil(mUnitRefineOil->isChecked());
	mUnit->addPlugin(p);
 }
 if(mUnitCanRepair->isChecked()) {
	RepairProperties* p = new RepairProperties(mUnit);
	mUnit->addPlugin(p);
 }
 // Producing page
 mUnit->insertULongBaseValue(mUnitProductionTime->value(), "ProductionTime");
 mUnit->setProducer(mUnitProducer->value());
 mUnit->setRequirements(stringToList(mUnitRequirements->text()));
 // Mapping page
 if(mUnitTexturesList->childCount() > 0) {
	QListViewItemIterator it(mUnitTexturesList);
	QStringList textures;
	for (; it.current(); ++it) {
		mUnit->addTextureMapping(it.current()->text(0), it.current()->text(1));
	}
 }
 mUnit->addSound(SoundOrderMove, mUnitSoundOrderMove->text());
 mUnit->addSound(SoundOrderAttack, mUnitSoundOrderAttack->text());
 mUnit->addSound(SoundOrderSelect, mUnitSoundOrderSelect->text());
 mUnit->addSound(SoundReportProduced, mUnitSoundReportProduced->text());
 mUnit->addSound(SoundReportDestroyed, mUnitSoundReportDestroyed->text());
 mUnit->addSound(SoundReportUnderAttack, mUnitSoundReportUnderAttack->text());
 // Other page
 mUnit->setDestroyedEffectIds(stringToList(mUnitDestroyedEffects->text()));
 mUnit->setConstructedEffectIds(stringToList(mUnitConstructedEffects->text()));
 mUnit->setExplodingDamageRange(mUnitExplodingDamageRange->value());
 mUnit->setExplodingDamage(mUnitExplodingDamage->value());
 BoVector3Fixed hitpoint(mUnitHitPointX->value(), mUnitHitPointY->value(), mUnitHitPointZ->value());
 mUnit->setHitPoint(hitpoint);
}

void BoUnitEditor::updateWidgets()
{
 // General page
 mUnitPath->setText(mUnit->unitPath());
 mUnitName->setText(mUnit->name());
 mUnitId->setValue(mUnit->typeId());
 bool isFac = mUnit->isFacility();    
 mUnitTypeFacility->setChecked(isFac);
 mUnitTypeMobile->setChecked(!isFac);
 slotTypeChanged();
 mUnitWidth->setValue(mUnit->unitWidth());
 mUnitHeight->setValue(mUnit->unitHeight());
 mUnitDepth->setValue(mUnit->unitDepth());
 // Properties page
 mUnitHealth->setValue(mUnit->ulongBaseValue("Health"));
 mUnitArmor->setValue(mUnit->ulongBaseValue("Armor"));
 mUnitShields->setValue(mUnit->ulongBaseValue("Shields"));
 mUnitMineralCost->setValue(mUnit->ulongBaseValue("MineralCost"));
 mUnitOilCost->setValue(mUnit->ulongBaseValue("OilCost"));
 mUnitSight->setValue(mUnit->ulongBaseValue("SightRange"));
 int terrain = (int)(mUnit->terrainType());
 mUnitTerrain->setCurrentItem(terrain);
 mUnitSupportMiniMap->setChecked(mUnit->supportMiniMap());
 // FIXME: UnitProperties only saves mobile *or* facility properties, but
 //  I'd like to have them both saved
 // TODO: This MUST be double, but Designer knows nothing about KDoubleNumInput
 mUnitSpeed->setValue(mUnit->bofixedBaseValue("Speed"));
 mUnitCanGoOnLand->setChecked(mUnit->canGoOnLand());
 mUnitCanGoOnWater->setChecked(mUnit->canGoOnWater());
 mUnitConstructionSteps->setValue(mUnit->constructionSteps());
 // Weapons
 mWeapons.clear();
 mWeaponsList->clear();
 int weaponcounter = 0;
 mWeaponGroup->setEnabled(false);
 mCurrentWeapon = -1;
 QPtrListIterator<PluginProperties> it(*(mUnit->plugins()));
 while(it.current()) {
	if(it.current()->pluginType() == PluginProperties::Weapon) {
		BosonWeaponProperties* w = (BosonWeaponProperties*)(it.current());
		mWeapons.insert(weaponcounter, w);
		mWeaponsList->insertItem(w->weaponName(), weaponcounter);
		weaponcounter++;
	}
	++it;
 }
 // Plugins page
 const ProductionProperties* productionProp = (ProductionProperties*)(mUnit->properties(PluginProperties::Production));
 bool canProduce = (productionProp != 0l);
 mUnitCanProduce->setChecked(canProduce);
 mUnitProducingGroup->setEnabled(canProduce);
 if(canProduce) {
	mUnitProducerList->setText(listToString(productionProp->producerList()));
 }
 const HarvesterProperties* harvesterProp = (HarvesterProperties*)(mUnit->properties(PluginProperties::Harvester));
 bool canHarvest = (harvesterProp != 0l);
 mUnitCanHarvest->setChecked(canHarvest);
 mUnitHarvestGroup->setEnabled(canHarvest);
 if(canHarvest) {
	mUnitHarvestMinerals->setChecked(harvesterProp->canMineMinerals());
	mUnitHarvestOil->setChecked(harvesterProp->canMineOil());
	mUnitMaxResource->setValue(harvesterProp->maxResources());
	mUnitMiningSpeed->setValue(harvesterProp->miningSpeed());
	mUnitUnloadingSpeed->setValue(harvesterProp->unloadingSpeed());
 }
 const RefineryProperties* refineryProp = (RefineryProperties*)(mUnit->properties(PluginProperties::Refinery));
 bool canRefine = (refineryProp != 0l);
 mUnitCanRefine->setChecked(canRefine);
 mUnitRefineGroup->setEnabled(canRefine);
 if(canRefine) {
	mUnitRefineMinerals->setChecked(refineryProp->canRefineMinerals());
	mUnitRefineOil->setChecked(refineryProp->canRefineOil());
 }
 bool canRepair = (mUnit->properties(PluginProperties::Repair) != 0l);
 mUnitCanRepair->setChecked(canRepair);
 // Producing page
 mUnitProductionTime->setValue(mUnit->productionTime());
 mUnitProducer->setValue(mUnit->producer());
 mUnitRequirements->setText(listToString(mUnit->requirements()));
 // Mapping page
 mUnitTexturesList->clear();
 QMap<QString, QString> textures = mUnit->longTextureNames();
 QMap<QString, QString>::Iterator tit;
 for(tit = textures.begin(); tit != textures.end(); ++tit) {
	(void)new QListViewItem(mUnitTexturesList, tit.key(), tit.data());
 }
 mUnitSoundOrderMove->setText(mUnit->sound(SoundOrderMove));
 mUnitSoundOrderAttack->setText(mUnit->sound(SoundOrderAttack));
 mUnitSoundOrderSelect->setText(mUnit->sound(SoundOrderSelect));
 mUnitSoundReportProduced->setText(mUnit->sound(SoundReportProduced));
 mUnitSoundReportDestroyed->setText(mUnit->sound(SoundReportDestroyed));
 mUnitSoundReportUnderAttack->setText(mUnit->sound(SoundReportUnderAttack));
 // Other page
 mUnitDestroyedEffects->setText(listToString(mUnit->destroyedEffectIds()));
 mUnitConstructedEffects->setText(listToString(mUnit->constructedEffectIds()));
 mUnitExplodingDamageRange->setValue(mUnit->explodingDamageRange());
 mUnitExplodingDamage->setValue(mUnit->explodingDamage());
 BoVector3Fixed hitpoint = mUnit->hitPoint();
 mUnitHitPointX->setValue(hitpoint.x());
 mUnitHitPointY->setValue(hitpoint.y());
 mUnitHitPointZ->setValue(hitpoint.z());
}

void BoUnitEditor::updateWeaponProperties()
{
 if(mCurrentWeapon == -1) {
	return;
 }
 BosonWeaponProperties* w = mWeapons.at(mCurrentWeapon);
 w->setWeaponName(mWeaponName->text());
 w->insertLongWeaponBaseValue(mWeaponDamage->value(), "Damage");
 w->insertBoFixedWeaponBaseValue(mWeaponDamageRange->value(), "DamageRange");
 w->insertBoFixedWeaponBaseValue(mWeaponFullDamageRange->value(), "FullDamageRange");
 w->insertULongWeaponBaseValue(mWeaponReload->value(), "Reload");
 w->insertULongWeaponBaseValue(mWeaponRange->value(), "Range");
 w->insertBoFixedWeaponBaseValue(mWeaponSpeed->value(), "Speed");
 w->setCanShootAtAirUnits(mWeaponCanShootAtAirUnits->isChecked());
 w->setCanShootAtLandUnits(mWeaponCanShootAtLandUnits->isChecked());
 w->setModelFileName(mWeaponModel->text());
 w->setHeight(mWeaponHeight->value());
 BoVector3Fixed offset(mWeaponOffsetX->value(), mWeaponOffsetY->value(), mWeaponOffsetZ->value());
 w->setShootEffectIds(stringToList(mWeaponShootEffects->text()));
 w->setFlyEffectIds(stringToList(mWeaponFlyEffects->text()));
 w->setHitEffectIds(stringToList(mWeaponHitEffects->text()));
 w->setSound(SoundWeaponShoot, mWeaponSoundShoot->text());
}


void BoUnitEditor::slotAddWeapon()
{
 // 0 is actually invalid id... but we don't care as it's only used for loading/saving games
 BosonWeaponProperties* w = new BosonWeaponProperties(mUnit, 0);
 w->reset();
 mWeapons.append(w);
 mWeaponsList->insertItem("New weapon", mWeapons.count() - 1);
 mWeaponsList->setCurrentItem(mWeapons.count() - 1);
 slotWeaponSelected(mWeapons.count() - 1);
 mConfigChanged = true;
 updateConfigWidgets();
}

void BoUnitEditor::slotWeaponSelected( int index )
{
 // Selecting another weapon modifies widget's contents, but we want to maintain old configChanged status
 bool configWasChanged = mConfigChanged;
 if(mCurrentWeapon != -1) {
	updateWeaponProperties();
 }
 mCurrentWeapon = index;
 updateWeaponWidgets();
 mConfigChanged = configWasChanged;
 updateConfigWidgets();
}


void BoUnitEditor::slotRemoveWeapon()
{
 if(mCurrentWeapon == -1) {
	mWeaponsRemove->setEnabled(false);
	return;
 }
 int item = mCurrentWeapon;
 mWeaponsList->removeItem(item);
 mWeapons.remove(item);
 if(mCurrentWeapon >= (signed int)(mWeaponsList->count())) {
	mCurrentWeapon--;
 }
 updateWeaponWidgets();
 mConfigChanged = true;
 updateConfigWidgets();
}


void BoUnitEditor::updateWeaponWidgets()
{
 if(mCurrentWeapon == -1) {
	mWeaponGroup->setEnabled(false);
	mWeaponsRemove->setEnabled(false);
	return;
 }
 mWeaponGroup->setEnabled(true);
 mWeaponsRemove->setEnabled(true);
 BosonWeaponProperties* w = mWeapons.at(mCurrentWeapon);
 mWeaponName->setText(w->weaponName());
 mWeaponDamage->setValue(w->damage());
 mWeaponDamageRange->setValue(w->damageRange());
 mWeaponFullDamageRange->setValue(w->fullDamageRange());
 mWeaponReload->setValue(w->reloadingTime());
 mWeaponRange->setValue(w->range());
 mWeaponCanShootAtAirUnits->setChecked(w->canShootAtAirUnits());
 mWeaponCanShootAtLandUnits->setChecked(w->canShootAtLandUnits());
 mWeaponSpeed->setValue(w->speed());
 mWeaponModel->setText(w->modelFileName());
 mWeaponHeight->setValue(w->height());
 BoVector3Fixed o = w->offset();
 mWeaponOffsetX->setValue(o.x());
 mWeaponOffsetY->setValue(o.y());
 mWeaponOffsetZ->setValue(o.z());
 mWeaponShootEffects->setText(listToString(w->shootEffectIds()));
 mWeaponFlyEffects->setText(listToString(w->flyEffectIds()));
 mWeaponHitEffects->setText(listToString(w->hitEffectIds()));
 mWeaponSoundShoot->setText(w->sound(SoundWeaponShoot));
}


void BoUnitEditor::slotConfigChanged()
{
 // Unit config has changed
 mConfigChanged = true;
 updateConfigWidgets();
}


void BoUnitEditor::updateConfigWidgets()
{
 mUnitSaveButton->setEnabled(mConfigChanged);
}
