/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qstringlist.h>
#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>
#include <qvalidator.h>
#include <qfileinfo.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kfiledialog.h>

#include "unitproperties.h"
#include "bosonweapon.h"
#include "pluginproperties.h"
#include "bo3dtools.h"

QString listToString(QValueList<unsigned long int> list)
{
    QString str;
    QValueList<unsigned long int>::Iterator it;
    for(it = list.begin(); it != list.end(); ++it) {
	str = str + "," + QString::number(*it);
    }
    if(!str.isEmpty()) {
	str.remove(0, 1);
    }
    return str;
}

QValueList<unsigned long int> stringToList(QString str)
{
    QValueList<unsigned long int> list;
    QStringList strlist = QStringList::split(",", str);
    QStringList::Iterator it;
    for(it = strlist.begin(); it != strlist.end(); ++it) {
	list.append((*it).toULong());
    }
    return list;
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
	dir = KFileDialog::getExistingDirectory(QString::null, 0,
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
    slotUpdateUnitProperties();
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
    slotUpdateWidgets();
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
    slotUpdateWidgets();
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
    mUnitDestroyedParticles->setValidator(v);
    mUnitConstructedParticles->setValidator(v);
    mWeaponShootParticles->setValidator(v);
    mWeaponFlyParticles->setValidator(v);
    mWeaponHitParticles->setValidator(v);
    
    // Clear everything to default values
    if(mUnits.isEmpty()) {
	mUnit->reset();
    } else {
	mUnitsList->setSelected(0, true);
    }
    mConfigChanged = false;
    updateConfigWidgets();
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

void BoUnitEditor::destroy()
{
    // Save config
    KConfig* cfg = kapp->config();
    cfg->setGroup("Boson Unit Editor");
    cfg->writeEntry("SearchPaths", mSearchPaths->currentPaths());
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
    QString dir = KFileDialog::getExistingDirectory();
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

void BoUnitEditor::slotCanHarvestChanged()
{
    mUnitHarvestGroup->setEnabled(mUnitCanHarvest->isChecked());
}

void BoUnitEditor::slotCanProduceChanged()
{
    mUnitProducingGroup->setEnabled(mUnitCanProduce->isChecked());
}




void BoUnitEditor::slotUpdateUnitProperties()
{
    mUnit->clearPlugins(false);
    // General page
    mUnit->setName(mUnitName->text());
    mUnit->setTypeId(mUnitId->value());
    mUnit->setUnitWidth((unsigned int)(mUnitWidth->value() * BO_TILE_SIZE));
    mUnit->setUnitHeight((unsigned int)(mUnitHeight->value() * BO_TILE_SIZE));
    mUnit->setUnitDepth(mUnitDepth->value());
    
    // Mobile/facility properties
    if(mUnitTypeFacility->isChecked()) {
	mUnit->createFacilityProperties();
	mUnit->setConstructionSteps(mUnitConstructionSteps->value());
    } else {
	mUnit->createMobileProperties();
	mUnit->setSpeed(mUnitSpeed->value());
	mUnit->setCanGoOnLand(mUnitCanGoOnLand->isChecked());
	mUnit->setCanGoOnWater(mUnitCanGoOnWater->isChecked());
    }
    // Properties page
    mUnit->setHealth(mUnitHealth->value());
    mUnit->setArmor(mUnitArmor->value());
    mUnit->setShields(mUnitShields->value());
    mUnit->setMineralCost(mUnitMineralCost->value());
    mUnit->setOilCost(mUnitOilCost->value());
    mUnit->setSightRange(mUnitSight->value());
    mUnit->setTerrainType((UnitProperties::TerrainType)(mUnitTerrain->currentItem()));
    mUnit->setSupportMiniMap(mUnitSupportMiniMap->isChecked());
    // Weapons page
    // Sync current weapon first
    slotUpdateWeaponProps();
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
    mUnit->setProductionTime(mUnitProductionTime->value());
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
    mUnit->setDestroyedParticleSystemIds(stringToList(mUnitDestroyedParticles->text()));
    mUnit->setConstructedParticleSystemIds(stringToList(mUnitConstructedParticles->text()));
    mUnit->setExplodingDamageRange(mUnitExplodingDamageRange->value());
    mUnit->setExplodingDamage(mUnitExplodingDamage->value());
    BoVector3 hitpoint(mUnitHitPointX->value(), mUnitHitPointY->value(), mUnitHitPointZ->value());
    hitpoint.cellToCanvas();
    mUnit->setHitPoint(hitpoint);
}

void BoUnitEditor::slotUpdateWidgets()
{
    // General page
    mUnitPath->setText(mUnit->unitPath());
    mUnitName->setText(mUnit->name());
    mUnitId->setValue(mUnit->typeId());
    bool isFac = mUnit->isFacility();    
    mUnitTypeFacility->setChecked(isFac);
    mUnitTypeMobile->setChecked(!isFac);
    slotTypeChanged();
    mUnitWidth->setValue(mUnit->unitWidth() / (double)BO_TILE_SIZE);
    mUnitHeight->setValue(mUnit->unitHeight() / (double)BO_TILE_SIZE);
    mUnitDepth->setValue(mUnit->unitDepth());
    // Properties page
    mUnitHealth->setValue(mUnit->health());
    mUnitArmor->setValue(mUnit->armor());
    mUnitShields->setValue(mUnit->shields());
    mUnitMineralCost->setValue(mUnit->mineralCost());
    mUnitOilCost->setValue(mUnit->oilCost());
    mUnitSight->setValue(mUnit->sightRange());
    int terrain = (int)(mUnit->terrainType());
    mUnitTerrain->setCurrentItem(terrain);
    mUnitSupportMiniMap->setChecked(mUnit->supportMiniMap());
    // FIXME: UnitProperties only saves mobile *or* facility properties, but
    //  I'd like to have them both saved
    // TODO: This MUST be double, but Designer knows nothing about KDoubleNumInput
    mUnitSpeed->setValue(mUnit->speed());
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
    mUnitDestroyedParticles->setText(listToString(mUnit->destroyedParticleSystemIds()));
    mUnitConstructedParticles->setText(listToString(mUnit->constructedParticleSystemIds()));
    mUnitExplodingDamageRange->setValue(mUnit->explodingDamageRange());
    mUnitExplodingDamage->setValue(mUnit->explodingDamage());
    BoVector3 hitpoint = mUnit->hitPoint();
    hitpoint.canvasToCell();
    mUnitHitPointX->setValue(hitpoint.x());
    mUnitHitPointY->setValue(hitpoint.y());
    mUnitHitPointZ->setValue(hitpoint.z());
}

void BoUnitEditor::slotUpdateWeaponProps()
{
    if(mCurrentWeapon == -1) {
	return;
    }
    BosonWeaponProperties* w = mWeapons.at(mCurrentWeapon);
    w->setWeaponName(mWeaponName->text());
    w->setDamage(mWeaponDamage->value());
    w->setDamageRange(mWeaponDamageRange->value());
    w->setFullDamageRange(mWeaponFullDamageRange->value());
    w->setReloadingTime(mWeaponReload->value());
    w->setRange(mWeaponRange->value());
    w->setCanShootAtAirUnits(mWeaponCanShootAtAirUnits->isChecked());
    w->setCanShootAtLandUnits(mWeaponCanShootAtLandUnits->isChecked());
    w->setSpeed((long unsigned int)mWeaponSpeed->value());
    w->setModelFileName(mWeaponModel->text());
    w->setHeight(mWeaponHeight->value());
    BoVector3 offset(mWeaponOffsetX->value(), mWeaponOffsetY->value(), mWeaponOffsetZ->value());
    offset.cellToCanvas();
    w->setOffset(offset);
    w->setShootParticleSystemIds(stringToList(mWeaponShootParticles->text()));
    w->setFlyParticleSystemIds(stringToList(mWeaponFlyParticles->text()));
    w->setHitParticleSystemIds(stringToList(mWeaponHitParticles->text()));
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
	slotUpdateWeaponProps();
    }
    mCurrentWeapon = index;
    slotUpdateWeaponWidgets();
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
    slotUpdateWeaponWidgets();
    mConfigChanged = true;
    updateConfigWidgets();
}


void BoUnitEditor::slotUpdateWeaponWidgets()
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
    BoVector3 o = w->offset();
    o.canvasToCell();
    mWeaponOffsetX->setValue(o.x());
    mWeaponOffsetY->setValue(o.y());
    mWeaponOffsetZ->setValue(o.z());
    mWeaponShootParticles->setText(listToString(w->shootParticleSystemIds()));
    mWeaponFlyParticles->setText(listToString(w->flyParticleSystemIds()));
    mWeaponHitParticles->setText(listToString(w->hitParticleSystemIds()));
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
