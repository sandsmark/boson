/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosoncommandframe.h"

#include "unit.h"
#include "unitplugins.h"
#include "player.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "bosonunitview.h"
#include "cell.h"
#include "bosontiles.h"
#include "bosoncommandwidget.h"
#include "bosonconfig.h"

#include <kstandarddirs.h>
#include <klocale.h>
#include <kgameprogress.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qintdict.h>
#include <qscrollview.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qlabel.h>

#include "defines.h"

#include "bosoncommandframe.moc"

#define UPDATE_TIMEOUT 200

class BoHarvesterWidget : public QWidget
{
public:
	BoHarvesterWidget(QWidget* parent) : QWidget(parent)
	{
		QHBoxLayout* layout = new QHBoxLayout(this);

		mMinerType = new QLabel(this);
		layout->addWidget(mMinerType);
		
		layout->addStretch(1);

		mProgress = new KGameProgress(this);
		layout->addWidget(mProgress);
	}

	~BoHarvesterWidget()
	{
	}

	void setMiner(HarvesterPlugin* h)
	{
		if (!h || (!h->canMineMinerals() && !h->canMineOil())) {
			return;
		}
		if (h->canMineMinerals()) {
			mMinerType->setText(i18n("Mineral Filling:"));
		} else {
			mMinerType->setText(i18n("Oil Filling:"));
		}

		unsigned int max = h->maxResources();
		unsigned int r = h->resourcesMined();
		double p = (double)(r * 100) / (double)max;
		mProgress->setValue((int)p);
	}
	
private:
	QLabel* mMinerType;
	KGameProgress* mProgress;
};

class BoConstructionProgress : public QWidget
{
public:
	BoConstructionProgress(QWidget* parent) : QWidget(parent)
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		QLabel* label = new QLabel(i18n("Construction:"), this);
		layout->addWidget(label);

		layout->addStretch(1);

		mProgress = new KGameProgress(this);
		layout->addWidget(mProgress);
	}

	~BoConstructionProgress()
	{
	}
	void setValue(int v)
	{
		mProgress->setValue(v);
	}
private:
	KGameProgress* mProgress;
};


class BoOrderWidget::BoOrderWidgetPrivate
{
public:
	BoOrderWidgetPrivate()
	{
		mTopLayout = 0;
		mOrderLayout = 0;

		mTransRef = 0;
		mInverted = 0;
		mTiles = 0;

		mButtonOffset = 0;
	}

	QIntDict<BosonCommandWidget> mOrderButton;
	QVBoxLayout* mTopLayout;
	QGridLayout* mOrderLayout; 
	
	QComboBox* mTransRef;
	QCheckBox* mInverted;
	BosonTiles* mTiles;
	QString mTilesDir;

	OrderType mOrderType; // plain tiles, facilities, mob units, ...

	int mButtonOffset;
};

BoOrderWidget::BoOrderWidget(bool editor, QWidget* parent) : QWidget(parent)
{
 d = new BoOrderWidgetPrivate;

 if (editor) {
	initEditor();
 }
}

BoOrderWidget::~BoOrderWidget()
{
 delete d->mTiles;
 delete d;
}

void BoOrderWidget::ensureButtons(unsigned int number)
{
 if (d->mOrderButton.count() >= number) {
	return;
 }
 for (unsigned int i = 0; i < number; i++) {
	if (!d->mOrderButton[i]) {
		BosonCommandWidget* b = new BosonCommandWidget(this);
		b->hide();
		b->setBackgroundOrigin(WindowOrigin);
		d->mOrderButton.insert(i, b);
		connect(b, SIGNAL(signalPlaceCell(int)), 
				this, SIGNAL(signalPlaceCell(int)));
		connect(b, SIGNAL(signalProduceUnit(unsigned long int)),
				this, SIGNAL(signalProduceUnit(unsigned long int)));
		connect(b, SIGNAL(signalStopProduction(unsigned long int)),
				this, SIGNAL(signalStopProduction(unsigned long int)));
		connect(b, SIGNAL(signalAction(int)),
				this, SIGNAL(signalAction(int)));
	}
 }
 resetLayout();
}

void BoOrderWidget::resetLayout()
{
 delete d->mOrderLayout;
 delete d->mTopLayout;
 d->mTopLayout = new QVBoxLayout(this);
 if (d->mTransRef) {
	d->mTopLayout->addWidget(d->mTransRef);
 }
 if (d->mInverted) {
	d->mTopLayout->addWidget(d->mInverted);
 }
 int buttons = boConfig->commandButtonsPerRow();
 d->mOrderLayout = new QGridLayout(d->mTopLayout, -1, -1);
 d->mTopLayout->addStretch(1);
 for (unsigned int i = 0; i < d->mOrderButton.count(); i++) {
	BosonCommandWidget* b = d->mOrderButton[i];
	d->mOrderLayout->addWidget(b, i / buttons, i % buttons, AlignHCenter);
 }
 int row = ((d->mOrderButton.count() - 1) / buttons) + 1;
 d->mOrderLayout->setRowStretch(row, 1);
 d->mOrderLayout->activate();
}

void BoOrderWidget::setButtonsPerRow(int b)
{
 boConfig->setCommandButtonsPerRow(b);
 resetLayout();
}

void BoOrderWidget::setOrderButtons(QValueList<unsigned long int> produceList, Player* owner, Facility* factory)
{
 ensureButtons(produceList.count() + d->mButtonOffset);
 hideOrderButtons();
 unsigned long int unitType = 0;
 ProductionPlugin* production = 0;
 if (factory) {
	production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
	if (!production) {
		kdDebug() << k_funcinfo << "factory cannot produce" << endl;
	} else if (production->hasProduction()) {
		unitType = production->currentProduction();
	}
 }
 for (unsigned int i = d->mButtonOffset; i < produceList.count() + d->mButtonOffset; i++) {
	d->mOrderButton[i]->setUnit(produceList[i], owner);
	d->mTopLayout->activate();
	if (unitType > 0 && production) {
		int count = production->productionList().contains(produceList[i]);
		if (produceList[i] != unitType) {
			d->mOrderButton[i]->setProductionCount(count);
			d->mOrderButton[i]->setGrayOut(true);
		} else {
			d->mOrderButton[i]->advanceProduction(production->productionProgress());
			if (factory->currentPluginType() != UnitPlugin::Production) {
				d->mOrderButton[i]->setProductionCount(-1);
			} else {
				d->mOrderButton[i]->setProductionCount(count);
			}
			d->mOrderButton[i]->setGrayOut(false);
		}
	} else {
		resetButton(d->mOrderButton[i]);
	}
 }
}

void BoOrderWidget::hideOrderButtons()
{
 QIntDictIterator<BosonCommandWidget> it(d->mOrderButton);
 while (it.current()) {
	it.current()->setUnit(0);
	++it;
 }
 d->mButtonOffset = 0;
}

void BoOrderWidget::showUnitActions(Unit* unit)
{
 kdDebug() << k_funcinfo << endl;
 d->mButtonOffset = 0;  // Should be 0 anyway...
 ensureButtons(3); // 3 is maximum number of actions for now

 // Order of action buttons: move, attack, stop. Nothing else yet.
 if(unit->isMobile()) {
	// If it's mobile, it can move
	resetButton(d->mOrderButton[d->mButtonOffset]);
	d->mOrderButton[d->mButtonOffset]->setAction(ActionMove, unit->owner());
	d->mButtonOffset++;
 }

 if(unit->unitProperties()->canShoot()) {
	// It can shoot
	resetButton(d->mOrderButton[d->mButtonOffset]);
	d->mOrderButton[d->mButtonOffset]->setAction(ActionAttack, unit->owner());
	d->mButtonOffset++;
 }

 // If it can't move or attack, then there's no sense in having stop
 if(d->mButtonOffset != 0) {
	resetButton(d->mOrderButton[d->mButtonOffset]);
	d->mOrderButton[d->mButtonOffset]->setAction(ActionStop, unit->owner());
	d->mButtonOffset++;
 }
 d->mTopLayout->activate();
}

void BoOrderWidget::slotRedrawTiles()
{
 bool inverted = d->mInverted->isChecked();
 Cell::TransType trans = (Cell::TransType)d->mTransRef->currentItem();
 // trans is one of TRANS_GW, TRANS_GD, TRANS_DW, TRANS_DWD ans specifies the
 // tile type (desert/water and so on)
 switch (d->mOrderType) {
	case OrderPlainTiles:
		hideOrderButtons();
		ensureButtons(Cell::GroundLast - 1);
		for (int i = 0; i < 5; i++) {
			int groundType = i + 1;
			d->mOrderButton[i]->setCell(groundType, d->mTiles);
		}
		break;
	case OrderSmall:
		hideOrderButtons();
		ensureButtons(9);
		for (int i = 0; i < 9; i++) {
			int tile = Cell::smallTileNumber(i, trans, inverted);
			d->mOrderButton[i]->setCell(tile, d->mTiles);
		}
		break;
	case OrderBig1:
		hideOrderButtons();
		ensureButtons(4);
		for (int i = 0; i < 4; i++) {
			d->mOrderButton[i]->setCell(Cell::getBigTransNumber(
					trans, (inverted ? 4 : 0) + i), 
					d->mTiles);
		}
		break;
	case OrderBig2:
		hideOrderButtons();
		ensureButtons(4);
		for (int i = 0; i < 4; i++) {
			d->mOrderButton[i]->setCell(Cell::getBigTransNumber(
					trans, (inverted ? 12 : 8) + i), 
					d->mTiles);
		}
		break;
	case OrderFacilities:
	case OrderMobiles:
		break;
	default:
		kdError() << "unexpected production index " << d->mOrderType << endl;
		break;
 }
}

void BoOrderWidget::setOrderType(int index)
{
 d->mOrderType = (OrderType)index;
}

void BoOrderWidget::initEditor()
{
 d->mTransRef = new QComboBox(this);
 connect(d->mTransRef, SIGNAL(activated(int)), this, SLOT(slotRedrawTiles()));
 d->mTransRef->insertItem(i18n("Grass/Water"), (int)Cell::TransGrassWater);
 d->mTransRef->insertItem(i18n("Grass/Desert"), (int)Cell::TransGrassDesert);
 d->mTransRef->insertItem(i18n("Desert/Water"), (int)Cell::TransDesertWater);
 d->mTransRef->insertItem(i18n("Deep Water"), (int)Cell::TransDeepWater);

 d->mInverted = new QCheckBox(this);
 d->mInverted->setText(i18n("Invert"));
 connect(d->mInverted, SIGNAL(toggled(bool)), this, SLOT(slotRedrawTiles()));
}

void BoOrderWidget::showUnit(Unit* unit)
{
 for (unsigned int i = 0; i < d->mOrderButton.count(); i++) {
	if (d->mOrderButton[i]->commandType() == BosonCommandWidget::CommandUnitSelected) {
		if (d->mOrderButton[i]->unit() == unit) {
			kdDebug() << "unit already displayed - update..." << endl;
			d->mOrderButton[i]->slotUnitChanged(unit);
			return;
		}
	} else if (d->mOrderButton[i]->commandType() == BosonCommandWidget::CommandNothing) {
//		kdDebug() << "show unit at " << i << endl;
		d->mOrderButton[i]->setUnit(unit);
		return;
	}
 }
 ensureButtons(d->mOrderButton.count() + 1);
// kdDebug() << "display unit" << endl;
 d->mOrderButton[d->mOrderButton.count() - 1]->setUnit(unit);
}

void BoOrderWidget::productionAdvanced(Unit* factory, double percentage)
{
 if (!factory->isFacility()) {
	kdError() << k_lineinfo << "NOT factory" << endl;
	return;
 }
 ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
 if (!production) {
	kdError() << k_funcinfo << factory->id() << " cannot produce" << endl;
	return;
 }
 if (!production->hasProduction()) {
	kdDebug() << k_funcinfo << "no production" << endl;
	return;
 }
 for (unsigned int i = 0; i < d->mOrderButton.count(); i++) {
	BosonCommandWidget* c = d->mOrderButton[i];
	if (c->commandType() == BosonCommandWidget::CommandUnit) {
		if (c->unitType() == production->currentProduction()) {
			c->advanceProduction(percentage);
		}
	}
 }
}

void BoOrderWidget::editorLoadTiles(const QString& tiles)
{
 QString themePath = KGlobal::dirs()->findResourceDir("data", QString("boson/themes/grounds/%1/index.desktop").arg(tiles)) + QString("boson/themes/grounds/%1").arg(tiles);
 d->mTilesDir = themePath;
 if (d->mTilesDir == QString::null) {
	kdError() << k_funcinfo << "Cannot find " << tiles << endl;
 } else {
	QTimer::singleShot(0, this, SLOT(slotEditorLoadTiles()));
 }
}

void BoOrderWidget::slotEditorLoadTiles()
{
 d->mTiles = new BosonTiles();
 if (!d->mTiles->loadTiles(d->mTilesDir)) {
	kdError() << k_funcinfo << "Could not load " << d->mTilesDir << endl;
	return;
 }
}

void BoOrderWidget::resetButton(BosonCommandWidget* button)
{
 button->setProductionCount(0);
 button->setGrayOut(false);
}


class BosonCommandFrame::BosonCommandFramePrivate
{
public:
	BosonCommandFramePrivate()
	{
		mOrderWidget = 0;
		mTopLayout = 0;

		mUnitView = 0;

		mOwner = 0;
		mSelectedUnit = 0;

		mConstructionProgress = 0;
		mMinerWidget = 0;
	}

	QVBoxLayout* mTopLayout;


	BosonUnitView* mUnitView;

	QPtrList<QWidget> mActionWidgets;
	BoOrderWidget* mOrderWidget;
	BoConstructionProgress* mConstructionProgress;
	BoHarvesterWidget* mMinerWidget;

	Player* mOwner;

	// for the action widgets
	Unit* mSelectedUnit;


	QTimer mUpdateTimer;
};

BosonCommandFrame::BosonCommandFrame(QWidget* parent, bool editor) : QFrame(parent, "cmd frame")
{
 init();

 d->mTopLayout = new QVBoxLayout(this, 5, 5); // FIXME: hardcoded - maybe use KDialog::marginHint(), KDialog::spacingHint()  -> but might be a problem cause we use setLineWidth(5)

 d->mUnitView = new BosonUnitView(this);
 d->mTopLayout->addWidget(d->mUnitView, 0, AlignHCenter);
 d->mUnitView->setBackgroundOrigin(WindowOrigin);

// we can have several "action" widgets in the frame. some of them just display
// a status (e.g. the construction progress), some of them actually provide
// actions (e.g. order buttons)
 QScrollView* scrollView = new QScrollView(this);
 scrollView->setResizePolicy(QScrollView::AutoOneFit);
 d->mTopLayout->addWidget(scrollView, 1);
 scrollView->setBackgroundOrigin(WindowOrigin);
 scrollView->viewport()->setBackgroundOrigin(WindowOrigin);
 scrollView->viewport()->setBackgroundMode(backgroundMode());

 QWidget* actionWidget = new QWidget(scrollView->viewport());
 scrollView->addChild(actionWidget);
 QVBoxLayout* actionLayout = new QVBoxLayout(actionWidget);

// the order buttons
 d->mOrderWidget = new BoOrderWidget(editor, actionWidget);
 connect(d->mOrderWidget, SIGNAL(signalProduceUnit(unsigned long int)),
		this, SLOT(slotProduceUnit(unsigned long int)));
 connect(d->mOrderWidget, SIGNAL(signalStopProduction(unsigned long int)),
		this, SLOT(slotStopProduction(unsigned long int)));
 connect(d->mOrderWidget, SIGNAL(signalPlaceCell(int)),
		this, SIGNAL(signalCellSelected(int)));
 connect(d->mOrderWidget, SIGNAL(signalAction(int)),
		this, SIGNAL(signalAction(int)));
 actionLayout->addWidget(d->mOrderWidget);
 d->mOrderWidget->setBackgroundOrigin(WindowOrigin);
 d->mActionWidgets.append(d->mOrderWidget);

// the construction progress
 d->mConstructionProgress = new BoConstructionProgress(actionWidget);
 d->mConstructionProgress->setBackgroundOrigin(WindowOrigin);
 actionLayout->addWidget(d->mConstructionProgress);
 d->mConstructionProgress->hide();
 d->mActionWidgets.append(d->mConstructionProgress);

// the miner display (minerals/oil)
 d->mMinerWidget = new BoHarvesterWidget(actionWidget);
 d->mMinerWidget->setBackgroundOrigin(WindowOrigin);
 actionLayout->addWidget(d->mMinerWidget);
 d->mMinerWidget->hide();
 d->mActionWidgets.append(d->mMinerWidget);

 actionLayout->addStretch(1);
 setBackgroundOrigin(WindowOrigin);
 show();
}

void BosonCommandFrame::init()
{
 d = new BosonCommandFramePrivate;

 setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
 setMinimumSize(230, 200); // FIXME hardcoded value

 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdate()));
}

BosonCommandFrame::~BosonCommandFrame()
{
 delete d;
}

void BosonCommandFrame::slotShowSingleUnit(Unit* unit)
{
 if (!unit) {
	// display nothing
	d->mUnitView->setUnit(0);
	return;
 }
 if (unit->isDestroyed() || !unit->owner()) {
	d->mUnitView->setUnit(0);
	return;
 }
 d->mUnitView->setUnit(unit);
}

void BosonCommandFrame::slotSetAction(Unit* unit)
{
 // this makes sure that they are even hidden if the unit cannot produce or
 // something:
 hideActions();

 if (!unit) {
	// display nothing
	return;
 }
 Player* owner = unit->owner();
 if (!owner) {
	kdError() << k_funcinfo << "no owner" << endl;
	return;
 }
 if (!unit->speciesTheme()) {
	kdError() << k_funcinfo << "NULL speciestheme" << endl;
	return;
 }

 // don't display production items of units of other players.
 if (owner != d->mOwner) {
	return;
 }

 d->mSelectedUnit = unit;

 // Show unit's actions (move, attack, stop...
 d->mOrderWidget->showUnitActions(unit);
 d->mOrderWidget->show();

 if (d->mSelectedUnit->isFacility()) {
	Facility* fac = (Facility*)d->mSelectedUnit;
	if (!fac->isConstructionComplete()) {
		slotShowConstructionProgress(fac);
		kdWarning() << "construction not completed" << endl;
		// we must not yet display anything else.
		return;
	}
 }
 kdDebug() << k_funcinfo << endl;

 if (d->mSelectedUnit->plugin(UnitPlugin::Production)) {
	ProductionPlugin* production = (ProductionPlugin*)d->mSelectedUnit->plugin(UnitPlugin::Production);
	if (!d->mSelectedUnit->properties(PluginProperties::Production)) {
		// must not happen if the units has the production
		// plugin
		kdError() << k_funcinfo << "no production properties!" << endl;
		return;
	}
	ProductionProperties* pp = (ProductionProperties*)d->mSelectedUnit->properties(PluginProperties::Production);
	QValueList<unsigned long int> produceList = d->mSelectedUnit->speciesTheme()->productions(pp->producerList());
	// Filter out things that player can't actually build (requirements aren't
	//  met yet)
	QValueList<unsigned long int>::Iterator it;
	it = produceList.begin();
	while(it != produceList.end()) {
		if(!owner->canBuild(*it)) {
			it = produceList.remove(it);
		} else {
			it++;
		}
	}
	d->mOrderWidget->setOrderButtons(produceList, owner, (Facility*)unit);
	d->mOrderWidget->show();
	if (production->hasProduction()) {
		if (!d->mUpdateTimer.isActive()) {
			d->mUpdateTimer.start(UPDATE_TIMEOUT);
		}
	}
 }
 if (d->mSelectedUnit->plugin(UnitPlugin::Harvester)) {
	d->mMinerWidget->setMiner((HarvesterPlugin*)d->mSelectedUnit->plugin(UnitPlugin::Harvester));
	d->mMinerWidget->show();
	if (!d->mUpdateTimer.isActive()) {
		d->mUpdateTimer.start(UPDATE_TIMEOUT);
	}
 }
}

void BosonCommandFrame::hideActions()
{
 d->mUpdateTimer.stop();
 d->mSelectedUnit = 0;
 d->mOrderWidget->hideOrderButtons();
 QPtrListIterator<QWidget> it(d->mActionWidgets);
 while (it.current()) {
	it.current()->hide();
	++it;
 }
}

void BosonCommandFrame::slotEditorProduction(int index, Player* owner)
{
 d->mOrderWidget->hideOrderButtons();
 if (index == -1) {
	return;
 }
 if (!owner) {
	kdError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	kdError() << k_funcinfo << "NULL theme" << endl;
	return;
 }
 d->mOrderWidget->setOrderType(index);
 switch (index) {
	case OrderPlainTiles:
	case OrderSmall:
	case OrderBig1:
	case OrderBig2:
		d->mOrderWidget->slotRedrawTiles();
		d->mOrderWidget->show();
		break;
	case OrderMobiles:
		d->mOrderWidget->setOrderButtons(theme->allMobiles(), owner);
		d->mOrderWidget->show();
		break;
	case OrderFacilities:
		d->mOrderWidget->setOrderButtons(theme->allFacilities(), owner);
		d->mOrderWidget->show();
		break;
	default:
		kdError() << k_funcinfo << "Invalid index " << index << endl;
		return;
 }
}

void BosonCommandFrame::slotEditorLoadTiles(const QString& fileName)
{
 d->mOrderWidget->editorLoadTiles(fileName);
}

void BosonCommandFrame::setLocalPlayer(Player* p)
{
 d->mOwner = p;
}

void BosonCommandFrame::slotProduceUnit(unsigned long int unitType)
{
 emit signalProduceUnit(unitType, (UnitBase*)d->mSelectedUnit, d->mOwner);
}
void BosonCommandFrame::slotStopProduction(unsigned long int unitType)
{
 emit signalStopProduction(unitType, (UnitBase*)d->mSelectedUnit, d->mOwner);
}

void BosonCommandFrame::slotShowUnit(Unit* unit)
{
 d->mOrderWidget->showUnit(unit);
 d->mOrderWidget->show();
}

void BosonCommandFrame::slotUpdateProduction(Unit* f)
{
 if (!f) {
	kdError() << k_funcinfo << "NULL production plugin" << endl;
	return;
 }
 if ((d->mSelectedUnit) == f) {
	slotSetAction(f);
 }
}

void BosonCommandFrame::slotShowConstructionProgress(Facility* fac)
{
 if (!fac) {
	kdError() << k_funcinfo << "NULL facility" << endl;
	return;
 }
 d->mConstructionProgress->show();
 d->mConstructionProgress->setValue((int)((Facility*)d->mSelectedUnit)->constructionProgress());
 d->mUpdateTimer.start(1000);
}

void BosonCommandFrame::slotSetButtonsPerRow(int b)
{
 d->mOrderWidget->setButtonsPerRow(b);
}

void BosonCommandFrame::slotUpdate()
{
 if (!d->mSelectedUnit) {
	d->mUpdateTimer.stop();
	return;
 }
 bool used = false;
 if (!d->mConstructionProgress->isHidden()) {
	slotShowConstructionProgress((Facility*)d->mSelectedUnit);
	if (((Facility*)d->mSelectedUnit)->isConstructionComplete()) {
		slotSetAction(d->mSelectedUnit);
	} else {
		used = true;
	}
 }
 if (!d->mOrderWidget->isHidden()) {
	ProductionPlugin* production = (ProductionPlugin*)d->mSelectedUnit->plugin(UnitPlugin::Production);
	if (!production || !production->hasProduction()) {
		slotUpdateProduction(d->mSelectedUnit);
	} else {
		d->mOrderWidget->productionAdvanced(d->mSelectedUnit, 
				production->productionProgress());
		used = true;
	}
 }
 if (!d->mMinerWidget->isHidden()) {
	if (d->mSelectedUnit->currentPluginType() == UnitPlugin::Harvester) {
		used = true;
	}
	d->mMinerWidget->setMiner((HarvesterPlugin*)d->mSelectedUnit->plugin(UnitPlugin::Harvester));
 }
 if (!used) {
	d->mUpdateTimer.stop();
	return;
 }
}

void BosonCommandFrame::reparentMiniMap(QWidget* map)
{
 if (!map) {
	kdError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 map->reparent(this, QPoint(0,0));
 map->hide();
 d->mTopLayout->insertWidget(0, map, 0, AlignHCenter);
}

void BosonCommandFrame::resizeEvent(QResizeEvent* e)
{
 if (minimumSize().width() < sizeHint().width()) {
	setMinimumWidth(sizeHint().width());
 }
 QFrame::resizeEvent(e);
}
