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
#include "unitpropertiesprivate.h"
#include "bosonweapon.h"
#include "pluginproperties.h"
#include "bo3dtools.h"
#include "bofiledialog.h"
#include "bosonsearchpathswidget.h"
#include "bodebug.h"
#include "bosonconfig.h"

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


EditorUnitProperties::EditorUnitProperties(SpeciesTheme* theme, bool fullMode)
	: UnitProperties(theme, fullMode)
{
}

void EditorUnitProperties::setName(const QString& n)
{
 d->mName = n;
}

void EditorUnitProperties::setRequirements(QValueList<unsigned long int> requirements)
{
 d->mRequirements = requirements;
}

void EditorUnitProperties::clearPlugins(bool deleteweapons)
{
 // FIXME: deleteweapons is very ugly hack here. In unit editor, we store
 //  pointers to units, so we must not delete weapons here
 if (!deleteweapons) {
	d->mPlugins.setAutoDelete(false);
	PluginProperties* p = d->mPlugins.first();
	while (p) {
		if (p->pluginType() != PluginProperties::Weapon) {
			delete p;
		}
		d->mPlugins.remove();
		p = d->mPlugins.current();
	}
	d->mPlugins.setAutoDelete(true);
 } else {
	d->mPlugins.clear();
 }
}

void EditorUnitProperties::addPlugin(PluginProperties* prop)
{
 d->mPlugins.append(prop);
}

void EditorUnitProperties::setConstructionSteps(unsigned int steps)
{
 if (isFacility()) {
	mConstructionFrames = steps;
 }
}

void EditorUnitProperties::setRotationSpeed(int speed)
{
 if (isMobile()) {
	mRotationSpeed = speed;
 }
}

void EditorUnitProperties::setCanGoOnLand(bool c)
{
 if (isMobile()) {
	mCanGoOnLand = c;
 }
}

void EditorUnitProperties::setCanGoOnWater(bool c)
{
 if (isMobile()) {
	mCanGoOnWater = c;
 }
}

void EditorUnitProperties::addTextureMapping(QString shortname, QString longname)
{
 d->mTextureNames.insert(shortname, longname);
}

void EditorUnitProperties::addSound(int event, QString filename)
{
 d->mSounds.insert(event, filename);
}

void EditorUnitProperties::setDestroyedEffectIds(QValueList<unsigned long int> ids)
{
 d->mDestroyedEffectIds = ids;
}

void EditorUnitProperties::setConstructedEffectIds(QValueList<unsigned long int> ids)
{
 d->mConstructedEffectIds = ids;
}

void EditorUnitProperties::reset()
{
 if (mFullMode) {
	// UnitProperties should be never reset in full mode (aka game mode)
	boWarning() << k_funcinfo << "Resetting UnitProperties in full mode!!!" << endl;
 }
 clearPlugins(false); // reset() is only used by unit editor (this far), so don't delete weapons
 // Set variables to default values
 d->mUnitPath = "";
 mTypeId = 0;
 mTerrain = (TerrainType)0;
 mUnitWidth = 1.0f;
 mUnitHeight = 1.0f;
 mUnitDepth = 1.0;
 d->mName = i18n("Unknown");
 insertULongBaseValue(100, "Health", "MaxValue");
 insertULongBaseValue(5, "SightRange", "MaxValue");
 insertULongBaseValue(100, "ProductionTime", "MaxValue");
 insertULongBaseValue(100, "MineralCost", "MaxValue");
 insertULongBaseValue(0, "OilCost", "MaxValue");
 insertULongBaseValue(0, "Armor", "MaxValue");
 insertULongBaseValue(0, "Shields", "MaxValue");
 insertBoFixedBaseValue(0, "Speed", "MaxValue");
 insertBoFixedBaseValue(2.0f / 20.0f / 20.0f, "AccelerationSpeed", "MaxValue");
 insertBoFixedBaseValue(4.0f / 20.0f / 20.0f, "DecelerationSpeed", "MaxValue");
 mSupportMiniMap = false;
 d->mRequirements.clear();
 d->mDestroyedEffectIds.clear();
 d->mConstructedEffectIds.clear();
 d->mHitPoint.reset();
 mProducer = 0;
 mExplodingDamage = 0;
 mExplodingDamageRange = 0;
 // Mobile stuff (because unit is mobile by default)
 mIsFacility = false;
 mRotationSpeed = (int)(45.0f * bofixedBaseValue("Speed"));
 mCanGoOnLand = true;
 mCanGoOnWater = false;
 // Sounds
 d->mSounds.clear();
 d->mSounds.insert(SoundOrderMove, "order_move");
 d->mSounds.insert(SoundOrderAttack, "order_attack");
 d->mSounds.insert(SoundOrderSelect, "order_select");
 d->mSounds.insert(SoundReportProduced, "report_produced");
 d->mSounds.insert(SoundReportDestroyed, "report_destroyed");
 d->mSounds.insert(SoundReportUnderAttack, "report_underattack");
 // Clear other lists
 d->mTextureNames.clear();
}

void EditorUnitProperties::setHitPoint(const BoVector3Fixed& hitpoint)
{
 d->mHitPoint = hitpoint;
}

bool EditorUnitProperties::saveUnitType(const QString& fileName)
{
 d->mUnitPath = fileName.left(fileName.length() - QString("index.unit").length());
 KSimpleConfig conf(fileName);
 conf.setGroup(QString::fromLatin1("Boson Unit"));

 conf.writeEntry("Id", typeId());
 conf.writeEntry("TerrainType", (int)mTerrain);
 conf.writeEntry("UnitWidth", (double)mUnitWidth);
 conf.writeEntry("UnitHeight", (double)mUnitHeight);
 conf.writeEntry("UnitDepth", (double)mUnitDepth);
 conf.writeEntry("Name", d->mName);
 conf.writeEntry("Health", ulongBaseValue("Health", "MaxValue", 100));
 conf.writeEntry("MineralCost", ulongBaseValue("MineralCost"));
 conf.writeEntry("OilCost", ulongBaseValue("OilCost"));
 conf.writeEntry("SightRange", ulongBaseValue("SightRange"));
 // This is converted from advance calls to seconds
 conf.writeEntry("ProductionTime", ulongBaseValue("ProductionTime") / 20.0f);
 conf.writeEntry("Shield", ulongBaseValue("Shields"));
 conf.writeEntry("Armor", ulongBaseValue("Armor"));
 conf.writeEntry("SupportMiniMap", mSupportMiniMap);
 conf.writeEntry("IsFacility", isFacility());
 BosonConfig::writeUnsignedLongNumList(&conf, "Requirements", d->mRequirements);
 conf.writeEntry("ExplodingDamage", mExplodingDamage);
 conf.writeEntry("ExplodingDamageRange", (double)mExplodingDamageRange);
 BoVector3Fixed tmpHitPoint(d->mHitPoint);
 BosonConfig::writeEntry(&conf, "HitPoint", d->mHitPoint);
 conf.writeEntry("Producer", mProducer);

 BosonConfig::writeUnsignedLongNumList(&conf, "DestroyedEffects", d->mDestroyedEffectIds);
 BosonConfig::writeUnsignedLongNumList(&conf, "ConstructedEffects", d->mConstructedEffectIds);

 if (isFacility()) {
	saveFacilityProperties(&conf);
 } else {
	saveMobileProperties(&conf);
 }

 saveAllPluginProperties(&conf);  // This saves weapons too
 saveTextureNames(&conf);
 saveSoundNames(&conf);

 return true;
}

bool EditorUnitProperties::saveMobileProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Mobile Unit");
 // We multiply speeds with 20 because speeds in config files are cells/second,
 //  but here we have cells/advance call
 conf->writeEntry("Speed", bofixedBaseValue("Speed") * 20.0f);
 conf->writeEntry("AccelerationSpeed", (double)bofixedBaseValue("AccelerationSpeed") * 20.0f * 20.0f);
 conf->writeEntry("DecelerationSpeed", (double)bofixedBaseValue("DecelerationSpeed") * 20.0f * 20.0f);
 conf->writeEntry("RotationSpeed", mRotationSpeed * 20.0f);
 conf->writeEntry("CanGoOnLand", mCanGoOnLand);
 conf->writeEntry("CanGoOnWater", mCanGoOnWater);
 return true;
}

bool EditorUnitProperties::saveFacilityProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Facility");
 conf->writeEntry("ConstructionSteps", mConstructionFrames);
 return true;
}

bool EditorUnitProperties::saveAllPluginProperties(KSimpleConfig* conf)
{
 int weaponcounter = 0;
 QPtrListIterator<PluginProperties> it(d->mPlugins);
 while (it.current()) {
	if (it.current()->pluginType() == PluginProperties::Weapon)
	{
		conf->setGroup(QString("Weapon_%1").arg(weaponcounter++));
	}
	it.current()->savePlugin(conf);
	++it;
 }
 conf->setGroup("Boson Unit");
 conf->writeEntry("Weapons", weaponcounter);
 return true;
}

bool EditorUnitProperties::saveTextureNames(KSimpleConfig* conf)
{
 if (d->mTextureNames.count() == 0) {
	return true;
 }
 conf->setGroup("Textures");
 QMap<QString, QString>::Iterator it;
 QStringList textures;
 for (it = d->mTextureNames.begin(); it != d->mTextureNames.end(); ++it) {
	textures.append(it.key());
	conf->writeEntry(it.key(), it.data());
 }
 conf->writeEntry("Textures", textures);
 return true;
}

bool EditorUnitProperties::saveSoundNames(KSimpleConfig* conf)
{
 conf->setGroup("Sounds");
 conf->writeEntry("OrderMove", d->mSounds[SoundOrderMove]);
 conf->writeEntry("OrderAttack", d->mSounds[SoundOrderAttack]);
 conf->writeEntry("OrderSelect", d->mSounds[SoundOrderSelect]);
 conf->writeEntry("ReportProduced", d->mSounds[SoundReportProduced]);
 conf->writeEntry("ReportDestroyed", d->mSounds[SoundReportDestroyed]);
 conf->writeEntry("ReportUnderAttack", d->mSounds[SoundReportUnderAttack]);
 return true;
}




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
 mProducerPageHandler = new BoProducerPageHandler(this);
 mMappingPageHandler = new BoMappingPageHandler(this);
 mWeaponPageHandler = new BoWeaponPageHandler(this);

 mUnitLoaded = false; // Bad hack
 mUnit = new EditorUnitProperties(0, false);
 mSearchPaths = new BosonSearchPathsWidget;
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
 mMappingPageHandler->slotAddTexture();
}

void BoUnitEditor::slotRemoveTexture()
{
 mMappingPageHandler->slotRemoveTexture();
}

void BoUnitEditor::slotCurrentTextureChanged()
{
 mMappingPageHandler->slotCurrentTextureChanged();
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
 mWeaponPageHandler->updateUnitProperties();

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

 mProducerPageHandler->updateUnitProperties();
 mMappingPageHandler->updateUnitProperties();

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
 mWeaponPageHandler->updateWidget();

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

 mProducerPageHandler->updateWidget();
 mMappingPageHandler->updateWidget();

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

void BoUnitEditor::slotAddWeapon()
{
 mWeaponPageHandler->slotAddWeapon();
}

void BoUnitEditor::slotWeaponSelected( int index )
{
 mWeaponPageHandler->slotWeaponSelected(index);
}

void BoUnitEditor::slotRemoveWeapon()
{
 mWeaponPageHandler->slotRemoveWeapon();
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


BoProducerPageHandler::BoProducerPageHandler(BoUnitEditor* parent)
	: QObject(parent)
{
 mEditor = parent;
}

void BoProducerPageHandler::updateUnitProperties()
{
 BO_CHECK_NULL_RET(mEditor);
 BO_CHECK_NULL_RET(mEditor->mUnit);
 EditorUnitProperties* unit = mEditor->mUnit;
 unit->insertULongBaseValue(mEditor->mUnitProductionTime->value(), "ProductionTime");
 unit->setProducer(mEditor->mUnitProducer->value());
 unit->setRequirements(stringToList(mEditor->mUnitRequirements->text()));
}

void BoProducerPageHandler::updateWidget()
{
 BO_CHECK_NULL_RET(mEditor);
 BO_CHECK_NULL_RET(mEditor->mUnit);
 EditorUnitProperties* unit = mEditor->mUnit;
 mEditor->mUnitProductionTime->setValue(unit->productionTime());
 mEditor->mUnitProducer->setValue(unit->producer());
 mEditor->mUnitRequirements->setText(listToString(unit->requirements()));
}


BoMappingPageHandler::BoMappingPageHandler(BoUnitEditor* parent)
	: QObject(parent)
{
 mEditor = parent;
}

void BoMappingPageHandler::updateUnitProperties()
{
 BO_CHECK_NULL_RET(mEditor);
 BO_CHECK_NULL_RET(mEditor->mUnit);
 EditorUnitProperties* unit = mEditor->mUnit;
 if(mEditor->mUnitTexturesList->childCount() > 0) {
	QListViewItemIterator it(mEditor->mUnitTexturesList);
	QStringList textures;
	for (; it.current(); ++it) {
		unit->addTextureMapping(it.current()->text(0), it.current()->text(1));
	}
 }
 unit->addSound(SoundOrderMove, mEditor->mUnitSoundOrderMove->text());
 unit->addSound(SoundOrderAttack, mEditor->mUnitSoundOrderAttack->text());
 unit->addSound(SoundOrderSelect, mEditor->mUnitSoundOrderSelect->text());
 unit->addSound(SoundReportProduced, mEditor->mUnitSoundReportProduced->text());
 unit->addSound(SoundReportDestroyed, mEditor->mUnitSoundReportDestroyed->text());
 unit->addSound(SoundReportUnderAttack, mEditor->mUnitSoundReportUnderAttack->text());
}

void BoMappingPageHandler::updateWidget()
{
 BO_CHECK_NULL_RET(mEditor);
 BO_CHECK_NULL_RET(mEditor->mUnit);
 EditorUnitProperties* unit = mEditor->mUnit;

 mEditor->mUnitTexturesList->clear();
 QMap<QString, QString> textures = unit->longTextureNames();
 for(QMap<QString, QString>::iterator it = textures.begin(); it != textures.end(); ++it) {
	(void)new QListViewItem(mEditor->mUnitTexturesList, it.key(), it.data());
 }
 mEditor->mUnitSoundOrderMove->setText(unit->sound(SoundOrderMove));
 mEditor->mUnitSoundOrderAttack->setText(unit->sound(SoundOrderAttack));
 mEditor->mUnitSoundOrderSelect->setText(unit->sound(SoundOrderSelect));
 mEditor->mUnitSoundReportProduced->setText(unit->sound(SoundReportProduced));
 mEditor->mUnitSoundReportDestroyed->setText(unit->sound(SoundReportDestroyed));
 mEditor->mUnitSoundReportUnderAttack->setText(unit->sound(SoundReportUnderAttack));
}

void BoMappingPageHandler::slotAddTexture()
{
 if(mEditor->mUnitTextureFrom->text().isEmpty() || mEditor->mUnitTextureTo->text().isEmpty()) {
	return;
 }
 (void)new QListViewItem(mEditor->mUnitTexturesList, mEditor->mUnitTextureFrom->text(),
		mEditor->mUnitTextureTo->text());
 mEditor->mUnitTextureFrom->clear();
 mEditor->mUnitTextureTo->clear();
 mEditor->mConfigChanged = true;
 mEditor->updateConfigWidgets();
}

void BoMappingPageHandler::slotRemoveTexture()
{
 QListViewItem* current = mEditor->mUnitTexturesList->currentItem();
 delete current;
 mEditor->mConfigChanged = true;
 mEditor->updateConfigWidgets();
}

void BoMappingPageHandler::slotCurrentTextureChanged()
{
 mEditor->mUnitTextureRemove->setEnabled(mEditor->mUnitTexturesList->currentItem() != 0);
}



BoWeaponPageHandler::BoWeaponPageHandler(BoUnitEditor* parent)
	: QObject(parent)
{
 mEditor = parent;

 mCurrentWeapon = -1;
}

void BoWeaponPageHandler::updateUnitProperties()
{
 BO_CHECK_NULL_RET(mEditor);
 BO_CHECK_NULL_RET(mEditor->mUnit);
 EditorUnitProperties* unit = mEditor->mUnit;

 // Sync current weapon first
 updateWeaponProperties();
 QPtrListIterator<BosonWeaponPropertiesEditor> it(mWeapons);
 while(it.current() != 0) {
	unit->addPlugin(it.current()->properties());
	++it;
 }
}

void BoWeaponPageHandler::updateWidget()
{
 BO_CHECK_NULL_RET(mEditor);
 BO_CHECK_NULL_RET(mEditor->mUnit);
 EditorUnitProperties* unit = mEditor->mUnit;

 mWeapons.clear();
 mEditor->mWeaponsList->clear();
 int weaponCounter = 0;
 mEditor->mWeaponGroup->setEnabled(false);
 mCurrentWeapon = -1;
 QPtrListIterator<PluginProperties> it(*(unit->plugins()));
 while(it.current()) {
	if(it.current()->pluginType() == PluginProperties::Weapon) {
		BosonWeaponProperties* w = (BosonWeaponProperties*)(it.current());
		w->setEditorObject(new BosonWeaponPropertiesEditor(w));
		mWeapons.insert(weaponCounter, w->editorObject());
		mEditor->mWeaponsList->insertItem(w->weaponName(), weaponCounter);
		weaponCounter++;
	}
	++it;
 }
}

void BoWeaponPageHandler::updateWeaponProperties()
{
 if(mCurrentWeapon == -1) {
	return;
 }
 BosonWeaponPropertiesEditor* w = mWeapons.at(mCurrentWeapon);
 w->setWeaponName(mEditor->mWeaponName->text());
 w->insertLongWeaponBaseValue(mEditor->mWeaponDamage->value(), "Damage");
 w->insertBoFixedWeaponBaseValue(mEditor->mWeaponDamageRange->value(), "DamageRange");
 w->insertBoFixedWeaponBaseValue(mEditor->mWeaponFullDamageRange->value(), "FullDamageRange");
 w->insertULongWeaponBaseValue(mEditor->mWeaponReload->value(), "Reload");
 w->insertULongWeaponBaseValue(mEditor->mWeaponRange->value(), "Range");
 w->insertBoFixedWeaponBaseValue(mEditor->mWeaponSpeed->value(), "Speed");
 w->setCanShootAtAirUnits(mEditor->mWeaponCanShootAtAirUnits->isChecked());
 w->setCanShootAtLandUnits(mEditor->mWeaponCanShootAtLandUnits->isChecked());
 w->setModelFileName(mEditor->mWeaponModel->text());
 w->setHeight(mEditor->mWeaponHeight->value());
 BoVector3Fixed offset(mEditor->mWeaponOffsetX->value(), mEditor->mWeaponOffsetY->value(), mEditor->mWeaponOffsetZ->value());
 w->setShootEffectIds(stringToList(mEditor->mWeaponShootEffects->text()));
 w->setFlyEffectIds(stringToList(mEditor->mWeaponFlyEffects->text()));
 w->setHitEffectIds(stringToList(mEditor->mWeaponHitEffects->text()));
 w->setSound(SoundWeaponShoot, mEditor->mWeaponSoundShoot->text());
}

void BoWeaponPageHandler::slotAddWeapon()
{
 // 0 is actually invalid id... but we don't care as it's only used for loading/saving games
 BosonWeaponProperties* w = new BosonWeaponProperties(mEditor->mUnit, 0);
 w->setEditorObject(new BosonWeaponPropertiesEditor(w));
 w->editorObject()->reset();
 mWeapons.append(w->editorObject());
 mEditor->mWeaponsList->insertItem("New weapon", mWeapons.count() - 1);
 mEditor->mWeaponsList->setCurrentItem(mWeapons.count() - 1);
 slotWeaponSelected(mWeapons.count() - 1);
 mEditor->mConfigChanged = true;
 mEditor->updateConfigWidgets();
}

void BoWeaponPageHandler::slotWeaponSelected( int index )
{
 // Selecting another weapon modifies widget's contents, but we want to maintain old configChanged status
 bool configWasChanged = mEditor->mConfigChanged;
 if(mCurrentWeapon != -1) {
	updateWeaponProperties();
 }
 mCurrentWeapon = index;
 updateWeaponWidgets();
 mEditor->mConfigChanged = configWasChanged;
 mEditor->updateConfigWidgets();
}

void BoWeaponPageHandler::slotRemoveWeapon()
{
 if(mCurrentWeapon == -1) {
	mEditor->mWeaponsRemove->setEnabled(false);
	return;
 }
 int item = mCurrentWeapon;
 mEditor->mWeaponsList->removeItem(item);
 mWeapons.remove(item);
 if(mCurrentWeapon >= (signed int)(mEditor->mWeaponsList->count())) {
	mCurrentWeapon--;
 }
 updateWeaponWidgets();
 mEditor->mConfigChanged = true;
 mEditor->updateConfigWidgets();
}

void BoWeaponPageHandler::updateWeaponWidgets()
{
 if(mCurrentWeapon == -1) {
	mEditor->mWeaponGroup->setEnabled(false);
	mEditor->mWeaponsRemove->setEnabled(false);
	return;
 }
 mEditor->mWeaponGroup->setEnabled(true);
 mEditor->mWeaponsRemove->setEnabled(true);
 const BosonWeaponProperties* w = mWeapons.at(mCurrentWeapon)->properties();
 mEditor->mWeaponName->setText(w->weaponName());
 mEditor->mWeaponDamage->setValue(w->damage());
 mEditor->mWeaponDamageRange->setValue(w->damageRange());
 mEditor->mWeaponFullDamageRange->setValue(w->fullDamageRange());
 mEditor->mWeaponReload->setValue(w->reloadingTime());
 mEditor->mWeaponRange->setValue(w->range());
 mEditor->mWeaponCanShootAtAirUnits->setChecked(w->canShootAtAirUnits());
 mEditor->mWeaponCanShootAtLandUnits->setChecked(w->canShootAtLandUnits());
 mEditor->mWeaponSpeed->setValue(w->speed());
 mEditor->mWeaponModel->setText(w->modelFileName());
 mEditor->mWeaponHeight->setValue(w->height());
 BoVector3Fixed o = w->offset();
 mEditor->mWeaponOffsetX->setValue(o.x());
 mEditor->mWeaponOffsetY->setValue(o.y());
 mEditor->mWeaponOffsetZ->setValue(o.z());
 mEditor->mWeaponShootEffects->setText(listToString(w->shootEffectIds()));
 mEditor->mWeaponFlyEffects->setText(listToString(w->flyEffectIds()));
 mEditor->mWeaponHitEffects->setText(listToString(w->hitEffectIds()));
 mEditor->mWeaponSoundShoot->setText(w->sound(SoundWeaponShoot));
}

