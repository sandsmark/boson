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
#include "bosoncommandframe.moc"

#include "bosonorderwidget.h"
#include "boactionswidget.h"
#include "../unit.h"
#include "../unitplugins.h"
#include "../player.h"
#include "../speciestheme.h"
#include "../pluginproperties.h"
#include "../boselection.h"
#include "../defines.h"
#include "../upgradeproperties.h"

#include <klocale.h>
#include <kgameprogress.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qvbox.h>

class BoHarvesterWidget : public BoUnitDisplayBase
{
public:
	BoHarvesterWidget(BosonCommandFrame* cmdFrame, QWidget* parent) : BoUnitDisplayBase(cmdFrame, parent)
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

protected:
	virtual bool display(Unit* unit)
	{
		
		HarvesterPlugin* miner = (HarvesterPlugin*)unit->plugin(UnitPlugin::Harvester);
		if (miner) {
			setMiner(miner);
			return true;
		}
		return false;
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

	virtual bool useUpdateTimer()
	{
		Unit* u = 0;
		if (!u) {
			return false;
		}
		if (u->currentPluginType() == UnitPlugin::Harvester) {
			return true;
		}
		return false;
	}

private:
	QLabel* mMinerType;
	KGameProgress* mProgress;
};

class BoConstructionProgress : public BoUnitDisplayBase
{
public:
	BoConstructionProgress(BosonCommandFrame* cmdFrame, QWidget* parent) : BoUnitDisplayBase(cmdFrame, parent)
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
protected:
	virtual bool display(Unit* unit)
	{
		if (!unit->isFacility()) {
			return false;
		}
		Facility* fac = (Facility*)unit;
		if (fac->isConstructionComplete()) {
			return false;
		}
		setValue((int)fac->constructionProgress());
		return true;
	}

private:
	KGameProgress* mProgress;
};


class BosonCommandFrame::BosonCommandFramePrivate
{
public:
	BosonCommandFramePrivate()
	{
		mConstructionProgress = 0;
		mMinerWidget = 0;
		mUnitActions = 0;
	}

	BoConstructionProgress* mConstructionProgress;
	BoHarvesterWidget* mMinerWidget;
	BoActionsWidget* mUnitActions;
};

BosonCommandFrame::BosonCommandFrame(QWidget* parent) : BosonCommandFrameBase(parent)
{
 init();

}

void BosonCommandFrame::init()
{
 d = new BosonCommandFramePrivate;

// the construction progress
 d->mConstructionProgress = new BoConstructionProgress(this, unitDisplayBox());

// the miner display (minerals/oil)
 d->mMinerWidget = new BoHarvesterWidget(this, unitDisplayBox());


// this should be at the end of unitDisplayBox():
 d->mUnitActions = new BoActionsWidget(unitDisplayBox());

// the order buttons
 connect(orderWidget(), SIGNAL(signalProduce(ProductionType, unsigned long int)),
		this, SLOT(slotProduce(ProductionType, unsigned long int)));
 connect(orderWidget(), SIGNAL(signalStopProduction(ProductionType, unsigned long int)),
		this, SLOT(slotStopProduction(ProductionType, unsigned long int)));
 connect(d->mUnitActions, SIGNAL(signalAction(int)),
		this, SIGNAL(signalAction(int)));
}

BosonCommandFrame::~BosonCommandFrame()
{
 delete d;
}

void BosonCommandFrame::setAction(Unit* unit)
{
 kdDebug() << k_funcinfo << endl;
 BosonCommandFrameBase::setAction(unit);
 if (!selectedUnit()) {
	return;
 }
 if (selectedUnit() != unit) {
	kdError() << k_funcinfo << "selectedUnit() != unit" << endl;
	return;
 }
 Player* owner = unit->owner();

 if (d->mConstructionProgress->showUnit(unit)) {
	startStopUpdateTimer();
	return;
 }

 // Show unit's actions (move, attack, stop...
 // TODO: these can be displayed (at least most of them) for groups, too!
 showUnitActions(unit);

 kdDebug() << k_funcinfo << endl;

 if (selectedUnit()->plugin(UnitPlugin::Production)) {
	if (!selectedUnit()->properties(PluginProperties::Production)) {
		// must not happen if the units has the production
		// plugin
		kdError() << k_funcinfo << "no production properties!" << endl;
		return;
	}
	ProductionProperties* pp = (ProductionProperties*)selectedUnit()->properties(PluginProperties::Production);
	QValueList<QPair<ProductionType, unsigned long int> > produceList;

	// Add units to production list
	QValueList<unsigned long int> unitsList = selectedUnit()->speciesTheme()->productions(pp->producerList());
	// Filter out things that player can't actually build (requirements aren't
	//  met yet)
	QValueList<unsigned long int>::Iterator it;
	it = unitsList.begin();
	while(it != unitsList.end()) {
		if(owner->canBuild(*it)) {
			QPair<ProductionType, unsigned long int> pair;
			pair.first = ProduceUnit;
			pair.second = *it;
			produceList.append(pair);
		}
		it++;
	}

	// Add technologies to production list
	QValueList<unsigned long int> techList = selectedUnit()->speciesTheme()->technologies(pp->producerList());
	// Filter out things that player can't actually build (requirements aren't
	//  met yet)
	QValueList<unsigned long int>::Iterator tit;  // tit = Technology ITerator ;-)
	for(tit = techList.begin(); tit != techList.end(); tit++) {
		if((!selectedUnit()->speciesTheme()->technology(*tit)->isResearched()) && (owner->canResearchTech(*tit))) {
			QPair<ProductionType, unsigned long int> pair;
			pair.first = ProduceTech;
			pair.second = *tit;
			produceList.append(pair);
		}
	}

	// Set buttons
	orderWidget()->setOrderButtons(produceList, owner, (Facility*)unit);
	orderWidget()->show();
 }
 d->mMinerWidget->showUnit(selectedUnit());
 startStopUpdateTimer();
}

void BosonCommandFrame::slotUpdate()
{
 BosonCommandFrameBase::slotUpdate();
 if (!selectedUnit()) {
	return;
 }
 if (!d->mConstructionProgress->isHidden()) {
	if (d->mConstructionProgress->showUnit(selectedUnit())) {
		if (!selectedUnit()->isFacility()) {
			// can't happen, since d->mConstructionProgress already
			// checks this
			kdError() << k_funcinfo << "No facility" << endl;
			return;
		}
	} else {
		// construction has been completed=!
		setAction(selectedUnit());
	}
 }
 if (!orderWidget()->isHidden()) {
	ProductionPlugin* production = (ProductionPlugin*)selectedUnit()->plugin(UnitPlugin::Production);
	if (production && production->hasProduction()) {
//		slotUpdateProduction(selectedUnit());
		orderWidget()->productionAdvanced(selectedUnit(), production->productionProgress());
	}
 }
}

bool BosonCommandFrame::checkUpdateTimer() const
{
 if (!selectedUnit()) {
	return false;
 }
 if (BosonCommandFrameBase::checkUpdateTimer()) {
	return true;
 }
 if (!orderWidget()->isHidden()) {
	ProductionPlugin* production = (ProductionPlugin*)selectedUnit()->plugin(UnitPlugin::Production);
	if (production && production->hasProduction()) {
		return true;
	}
 }
 return false;
}

void BosonCommandFrame::showUnitActions(Unit* unit)
{
 if (!unit) {
	d->mUnitActions->hide();
 } else {
	d->mUnitActions->showUnitActions(unit);
	d->mUnitActions->show();
 }
}

void BosonCommandFrame::slotSetButtonsPerRow(int b)
{
 BosonCommandFrameBase::slotSetButtonsPerRow(b);
 d->mUnitActions->setButtonsPerRow(b);
}
