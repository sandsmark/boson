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

void BoUnitEditor::slotShootAtChanged()
{
    if(mUnitCanShootLand->isChecked() || mUnitCanShootAir->isChecked()) {
	mUnitWeaponGroup->setEnabled(true);     
    } else {
	mUnitWeaponGroup->setEnabled(false);
    }
}

void BoUnitEditor::slotTypeChanged()
{
    if(mUnitTypeFacility->isChecked()) {
	// Type is facility
	//mUnitProducingGroup->setEnabled(true);
	mMobileOptions->setEnabled(false);
	mFacilityOptions->setEnabled(true);
    } else {
	// Type is mobile
	//mUnitProducingGroup->setEnabled(false);
	mMobileOptions->setEnabled(true);
	mFacilityOptions->setEnabled(false);
    }
}

void BoUnitEditor::slotUnitSelected( int )
{
    mEditUnitButton->setEnabled(true);
    // Store selected unit somewhere
}

void BoUnitEditor::slotEditUnit()
{
    // Ask is user wants to save current properties
    int answer = KMessageBox::questionYesNoCancel(this,
		i18n("Do you want to save current unit properties?"),
		i18n("Save current unit?"));
    if(answer == KMessageBox::Cancel) {
	return;
    } else if(answer == KMessageBox::Yes) {
	slotSaveUnit();
    }
    // Open new unit
    slotLoadUnit(mUnits[mUnitsList->currentItem()]);
}

void BoUnitEditor::slotAddTexture()
{
    (void)new QListViewItem(mUnitTexturesList, mUnitTextureFrom->text(),
		mUnitTextureTo->text());
    mUnitTextureFrom->clear();
    mUnitTextureTo->clear();
}

void BoUnitEditor::slotRemoveTexture()
{
    delete mUnitTexturesList->currentItem();
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
    KSimpleConfig cfg(d.absPath() + "/index.unit");
    cfg.setGroup("Boson Unit");
    // General page
    cfg.writeEntry("Name", mUnitName->text());
    cfg.writeEntry("Id", mUnitId->value());
    cfg.writeEntry("IsFacility", mUnitTypeFacility->isChecked());
    // Properties page
    cfg.writeEntry("Health", mUnitHealth->value());
    cfg.writeEntry("Armor", mUnitArmor->value());
    cfg.writeEntry("Shield", mUnitShields->value());
    cfg.writeEntry("MineralCost", mUnitMineralCost->value());
    cfg.writeEntry("OilCost", mUnitOilCost->value());
    cfg.writeEntry("SightRange", mUnitSight->value());
    cfg.writeEntry("TerrainType", mUnitTerrain->currentItem());
    cfg.writeEntry("SupportMiniMap", mUnitSupportMiniMap->isChecked());
    // Weapon page
    cfg.writeEntry("CanShootAtLandUnits", mUnitCanShootLand->isChecked());
    cfg.writeEntry("CanShootAtAirUnits", mUnitCanShootAir->isChecked());
    if(mUnitWeaponGroup->isEnabled()) {
	cfg.writeEntry("WeaponRange", mUnitWeaponRange->value());
	cfg.writeEntry("WeaponDamage", mUnitWeaponDamage->value());
	cfg.writeEntry("Reload", mUnitWeaponReload->value());
    }
    // Plugins page
    if(mUnitCanProduce->isChecked()) {
	cfg.setGroup("ProductionPlugin");
	// We save producer list as string but it *should* be in form of a list
	cfg.writeEntry("ProducerList", mUnitProducerList->text());
    } else {
	// Current plugins detection code sucks. It checks if certain groups exist, so
	//  we must delete those groups if unit doesn't have plugin
	cfg.deleteGroup("ProductionPlugin");
    }
    if(mUnitCanHarvest->isChecked()) {
	cfg.setGroup("HarvesterPlugin");
	cfg.writeEntry("CanMineMinerals", mUnitHarvestMinerals->isChecked());
	cfg.writeEntry("CanMineOil", mUnitHarvestOil->isChecked());
	cfg.writeEntry("MaxResources", mUnitMaxResource->value());
    } else {
	cfg.deleteGroup("HarvesterPlugin");
    }
    if(mUnitCanRepair->isChecked()) {
	cfg.setGroup("RepairPlugin");
	// Dummy entry because KConfig doesn't create group without any keys in it
	cfg.writeEntry("Dummy", "Dummy entry");
    } else {
	cfg.deleteGroup("RepairPlugin");
    }
    // Producing page
    cfg.setGroup("Boson Unit");
    cfg.writeEntry("ProductionTime", mUnitProductionTime->value());
    cfg.writeEntry("Producer", mUnitProducer->value());
    cfg.writeEntry("Requirements", mUnitRequirements->text());
    // Mapping page
    if(mUnitTexturesList->childCount() > 0) {
	cfg.setGroup("Textures");
	QListViewItemIterator it(mUnitTexturesList);
	QStringList textures;
	for (; it.current(); ++it) {
	    cfg.writeEntry(it.current()->text(0), it.current()->text(1));
	    textures.append(it.current()->text(0));
	}
	cfg.writeEntry("Textures", textures);
    }
    cfg.setGroup("Sounds");
    cfg.writeEntry("Shoot", mUnitSoundShoot->text());
    cfg.writeEntry("OrderMove", mUnitSoundOrderMove->text());
    cfg.writeEntry("OrderAttack", mUnitSoundOrderAttack->text());
    cfg.writeEntry("OrderSelect", mUnitSoundOrderSelect->text());
    cfg.writeEntry("ReportProduced", mUnitSoundReportProduced->text());
    cfg.writeEntry("ReportDestroyed", mUnitSoundReportDestroyed->text());
    cfg.writeEntry("ReportUnderAttack", mUnitSoundReportUnderAttack->text());
    
    // Save mobile/facility properties
    if(mUnitTypeMobile->isChecked()) {
	// Unit is mobile
	cfg.setGroup("Boson Mobile Unit");
	cfg.writeEntry("Speed", mUnitSpeed->value());
	cfg.writeEntry("CanGoOnLand", mUnitCanGoOnLand->isChecked());
	cfg.writeEntry("CanGoOnWater", mUnitCanGoOnWater->isChecked());
    } else {
	// If it's not mobile, it's facility
	cfg.setGroup("Boson Facility");
	cfg.writeEntry("CanRefineMinerals", mUnitCanRefineMinerals->isChecked());
	cfg.writeEntry("CanRefineOil", mUnitCanRefineOil->isChecked());
	cfg.writeEntry("ConstructionSteps", mUnitConstructionSteps->value());
    }
}

void BoUnitEditor::slotNewUnit()
{
    // Ask is user wants to save current properties
    int answer = KMessageBox::questionYesNoCancel(this,
		i18n("Do you want to save current unit properties?"),
		i18n("Save current unit?"));
    if(answer == KMessageBox::Cancel) {
	return;
    } else if(answer == KMessageBox::Yes) {
	slotSaveUnit();
    }
    // Clear everything to default values
    slotResetProperties();
}

void BoUnitEditor::slotResetProperties()
{
    // Reset all properties to default values
    // NOTE: keep this in sync with unitproperties.cpp/whereever defaults are asigned
    // General page
    mUnitPath->clear();
    mUnitName->clear();
    mUnitId->setValue(0);
    mUnitTypeMobile->setChecked(true);
    // Properties page
    mUnitHealth->setValue(100);
    mUnitArmor->setValue(0);
    mUnitShields->setValue(0);
    mUnitMineralCost->setValue(100);
    mUnitOilCost->setValue(0);
    mUnitSight->setValue(5);
    mUnitTerrain->setCurrentItem(0);
    mUnitSupportMiniMap->setChecked(false);
    mUnitSpeed->setValue(0);
    mUnitCanGoOnLand->setChecked(false);
    mUnitCanGoOnWater->setChecked(false);
    mUnitCanRefineMinerals->setChecked(false);
    mUnitCanRefineOil->setChecked(false);
    mUnitConstructionSteps->setValue(4);
    // Weapon page
    mUnitCanShootLand->setChecked(false);
    mUnitCanShootAir->setChecked(false);
    mUnitWeaponRange->setValue(0);
    mUnitWeaponDamage->setValue(0);
    mUnitWeaponReload->setValue(0);
    // Plugins page
    mUnitCanProduce->setChecked(false);
    mUnitProducerList->clear();
    mUnitCanHarvest->setChecked(false);
    mUnitHarvestMinerals->setChecked(false);
    mUnitHarvestOil->setChecked(false);
    mUnitMaxResource->setValue(100);
    mUnitCanRepair->setChecked(false);
    // Producer page
    mUnitProductionTime->setValue(0);
    mUnitProducer->setValue(0);
    mUnitRequirements->clear();
    // Mapping page
    mUnitTextureFrom->clear();
    mUnitTextureTo->clear();
    mUnitTexturesList->clear();
    mUnitSoundShoot->setText("shoot");
    mUnitSoundOrderMove->setText("order_move");
    mUnitSoundOrderAttack->setText("order_attack");
    mUnitSoundOrderSelect->setText("order_select");
    mUnitSoundReportProduced->setText("report_produced");
    mUnitSoundReportDestroyed->setText("report_destoyed");
    mUnitSoundReportUnderAttack->setText("report_underattack");
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
    return errors;
}

void BoUnitEditor::slotLoadUnit( QString dir )
{
    slotResetProperties();
    
    mUnitPath->setText(dir);
    
    KSimpleConfig cfg(dir + "/index.unit");
    cfg.setGroup("Boson Unit");
    // General page
    mUnitName->setText(cfg.readEntry("Name", i18n("Unknown")));
    mUnitId->setValue(cfg.readUnsignedLongNumEntry("Id", 0));
    bool isFac = cfg.readBoolEntry("IsFacility", false);
    mUnitTypeFacility->setChecked(isFac);
    // Properties page
    mUnitHealth->setValue(cfg.readUnsignedLongNumEntry("Health", 100));
    mUnitArmor->setValue(cfg.readUnsignedLongNumEntry("Armor", 0));
    mUnitShields->setValue(cfg.readUnsignedLongNumEntry("Shield", 0));
    mUnitMineralCost->setValue(cfg.readUnsignedLongNumEntry("MineralCost", 100));
    mUnitOilCost->setValue(cfg.readUnsignedLongNumEntry("OilCost", 100));
    mUnitSight->setValue(cfg.readUnsignedLongNumEntry("SightRange", 5));
    int terrain = cfg.readNumEntry("TerrainType", 0);
    mUnitTerrain->setCurrentItem(terrain);
    mUnitSupportMiniMap->setChecked(cfg.readBoolEntry("SupportMiniMap", false));
    // Yes, we load both mobile *and* facility properties for all units
    cfg.setGroup("Boson Mobile Unit");
    // TODO: This MUST be double, but Designer knows nothing about KDoubleNumInput
    mUnitSpeed->setValue(cfg.readNumEntry("Speed", 0));
    mUnitCanGoOnLand->setChecked(cfg.readBoolEntry("CanGoOnLand",
		(terrain == 2) || (terrain == 0)));
    mUnitCanGoOnWater->setChecked(cfg.readBoolEntry("CanGoOnWater", 
		(terrain == 2) || (terrain == 1)));
    cfg.setGroup("Boson Facility");
    mUnitCanRefineMinerals->setChecked(cfg.readBoolEntry("CanRefineMinerals", false));
    mUnitCanRefineOil->setChecked(cfg.readBoolEntry("CanRefineOil", false));
    // Weapon page
    cfg.setGroup("Boson Unit");
    mUnitWeaponRange->setValue(cfg.readUnsignedLongNumEntry("WeaponRange", 0));
    int wd = cfg.readLongNumEntry("WeaponDamage", 0);
    mUnitWeaponDamage->setValue(wd);
    mUnitWeaponReload->setValue(cfg.readUnsignedLongNumEntry("Reload", 0));
    mUnitCanShootLand->setChecked(cfg.readBoolEntry("CanShootAtLandUnits",
		(terrain != 2) && wd));
    mUnitCanShootAir->setChecked(cfg.readBoolEntry("CanShootAtAirUnits",
		(terrain == 2) && wd));
    // Plugins page
    if(cfg.hasGroup("ProductionPlugin")) {
	cfg.setGroup("ProductionPlugin");
	mUnitCanProduce->setChecked(true);
	mUnitProducerList->setText(cfg.readEntry("ProducerList", ""));
    }
    if(cfg.hasGroup("HarvesterPlugin")) {
	cfg.setGroup("HarvesterPlugin");
	mUnitCanHarvest->setChecked(true);
	mUnitHarvestMinerals->setChecked(cfg.readBoolEntry("CanMineMinerals", false));
	mUnitHarvestOil->setChecked(cfg.readBoolEntry("CanMineOil", false));
	mUnitMaxResource->setValue(cfg.readUnsignedNumEntry("MaxResources", 100));
    }
    if(cfg.hasGroup("RepairPlugin")) {
	mUnitCanRepair->setChecked(true);
    }
    // Producing page
    cfg.setGroup("Boson Unit");
    mUnitProductionTime->setValue(cfg.readUnsignedLongNumEntry("ProductionTime", 100));
    mUnitProducer->setValue(cfg.readUnsignedNumEntry("Producer", isFac ? 10 : terrain));
    mUnitRequirements->setText(cfg.readEntry("Requirements", ""));
    // Mapping page
    mUnitTexturesList->clear();
    if(cfg.hasGroup("Textures")) {
	QStringList textures = cfg.readListEntry("Textures");
	for(unsigned int i = 0; i < textures.count(); i++) {
	    (void)new QListViewItem(mUnitTexturesList, textures[i],
			cfg.readEntry(textures[i], "none"));
	}
    }
    cfg.setGroup("Sounds");
    mUnitSoundShoot->setText(cfg.readEntry("Shoot", "shoot"));
    mUnitSoundOrderMove->setText(cfg.readEntry("OrderMove", "order_move"));
    mUnitSoundOrderAttack->setText(cfg.readEntry("OrderAttack","order_attack" ));
    mUnitSoundOrderSelect->setText(cfg.readEntry("OrderSelect", "order_select"));
    mUnitSoundReportProduced->setText(cfg.readEntry("ReportProduced", "report_produced"));
    mUnitSoundReportDestroyed->setText(cfg.readEntry("ReporDestroyedt", "report_destroyed"));
    mUnitSoundReportUnderAttack->setText(cfg.readEntry("ReportUnderAttack", "report_underattack"));
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
    int answer = KMessageBox::questionYesNoCancel(this,
		i18n("Do you want to save current unit properties?"),
		i18n("Save current unit?"));
    if(answer == KMessageBox::Cancel) {
	return;
    } else if(answer == KMessageBox::Yes) {
	slotSaveUnit();
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
