/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "../../bomemory/bodummymemory.h"
#include "bosonorderwidget.h"
#include "bosonunitview.h"
#include "boactionswidget.h"
#include "editorunitconfigwidget.h"
#include "bocommandframeeventlistener.h"
#include "../../gameengine/unit.h"
#include "../../gameengine/unitplugins/unitplugins.h"
#include "../../gameengine/unitproperties.h"
#include "../../gameengine/upgradeproperties.h"
#include "../../gameengine/pluginproperties.h"
#include "../../gameengine/player.h"
#include "../../gameengine/playerio.h"
#include "../../gameengine/speciestheme.h"
#include "../../gameengine/boson.h"
#include "../../boufo/boufolabel.h"
#include "../../boufo/boufoprogress.h"
#include "../../boufo/boufocustomwidget.h"
#include "../../speciesdata.h"
#include "../../bosonviewdata.h"
#include "../boselection.h"
#include "bodebug.h"

#include <klocale.h>

#include <qtimer.h>
#include <qptrdict.h>
#include <qcolor.h>

#define UPDATE_TIMEOUT 200

// TODO scroll widget for selection widget


class ProduceActionCollection
{
public:
	ProduceActionCollection()
	{
	}
	~ProduceActionCollection()
	{
		mNewProduceActions.setAutoDelete(true);
		mNewProduceActions.clear();
	}

	const BoAction* produceActionFor(const UnitProperties* prop)
	{
		if (!prop) {
			BO_NULL_ERROR(prop);
			return 0;
		}
		if (mProduceActions.contains(prop)) {
			return mProduceActions[prop];
		}
		SpeciesTheme* theme = prop->theme();
		BoAction* a = new BoAction(QString("ProduceAction-%1").arg(prop->typeId()), boViewData->speciesData(theme)->smallOverview(prop->typeId(), theme->teamColor()), "Produce");
		mNewProduceActions.append(a);
		mProduceActions.insert(prop, a);
		return a;
	}
	const BoAction* produceActionFor(const UpgradeProperties* technology, const SpeciesTheme* theme)
	{
		if (!technology) {
			BO_NULL_ERROR(technology);
			return 0;
		}
		if (!theme) {
			BO_NULL_ERROR(theme);
			return 0;
		}
		if (mProduceActions.contains(technology)) {
			return mProduceActions[technology];
		}
		const BoAction* a = boViewData->speciesData(theme)->action(technology->produceActionString());
		mProduceActions.insert(technology, a);
		return a;
	}

private:
	QMap<const void*, const BoAction*> mProduceActions;
	QPtrList<BoAction> mNewProduceActions;
};


class BoHarvesterWidget : public BoUnitDisplayBase
{
public:
	BoHarvesterWidget(BosonCommandFrame* cmdFrame)
		: BoUnitDisplayBase(cmdFrame)
	{
		setLayoutClass(UHBoxLayout);

		mMinerType = new BoUfoLabel();
		addWidget(mMinerType);

#if 0
		layout->addStretch(1);
#endif

		mProgress = new BoUfoProgress();
		mProgress->setRange(0.0, 100.0);
		mProgress->setOrientation(Horizontal);
		mProgress->setFrameColor(QColor(128, 128, 128));
		addWidget(mProgress);
	}

	~BoHarvesterWidget()
	{
	}

protected:
	virtual bool display(Unit* unit)
	{
		if (unit->isFacility()) {
			UnitConstruction* c = unit->construction();
			if (!c->isConstructionComplete()) {
				return false;
			}
		}
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
		mProgress->setValue(p);
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
	BoUfoLabel* mMinerType;
	BoUfoProgress* mProgress;
};

class BoConstructionProgress : public BoUnitDisplayBase
{
public:
	BoConstructionProgress(BosonCommandFrame* cmdFrame)
		: BoUnitDisplayBase(cmdFrame)
	{
		setLayoutClass(UHBoxLayout);
		BoUfoLabel* label = new BoUfoLabel(i18n("Construction:"));
		addWidget(label);

#if 0
		layout->addStretch(1);
#endif
		mProgress = new BoUfoProgress();
		mProgress->setRange(0.0, 100.0);
		mProgress->setFrameColor(QColor(128, 128, 128));
		addWidget(mProgress);
	}

	~BoConstructionProgress()
	{
	}
	void setValue(double v)
	{
		mProgress->setValue(v);
	}
protected:
	virtual bool display(Unit* unit)
	{
		if (!unit->isFacility()) {
			return false;
		}
		UnitConstruction* c = unit->construction();
		if (c->isConstructionComplete()) {
			return false;
		}
		setValue(c->constructionProgress());
		return true;
	}

private:
	BoUfoProgress* mProgress;
};

class BoResourceMineWidget: public BoUnitDisplayBase
{
public:
	BoResourceMineWidget(BosonCommandFrame* cmdFrame)
		: BoUnitDisplayBase(cmdFrame)
	{
		setLayoutClass(UVBoxLayout);

		mMinerals = new BoUfoLabel();
		mOil = new BoUfoLabel();

		addWidget(mMinerals);
		addWidget(mOil);
	}

	~BoResourceMineWidget()
	{
	}

protected:
	virtual bool display(Unit* unit)
	{
		if (unit->isFacility()) {
			UnitConstruction* c = unit->construction();
			if (!c->isConstructionComplete()) {
				return false;
			}
		}
		ResourceMinePlugin* mine = (ResourceMinePlugin*)unit->plugin(UnitPlugin::ResourceMine);
		if (mine) {
			setMine(mine);
			return true;
		}
		return false;
	}

	void setMine(ResourceMinePlugin* m)
	{
		if (!m || (!m->canProvideMinerals() && !m->canProvideOil())) {
			return;
		}
		if (m->canProvideMinerals()) {
			mMinerals->show();
		} else {
			mMinerals->hide();
		}
		if (m->canProvideOil()) {
			mOil->show();
		} else {
			mOil->hide();
		}
		if (m->minerals() >= 0) {
			mMinerals->setText(i18n("Minerals: %1").arg(m->minerals()));
		} else {
			mMinerals->setText(i18n("Minerals: unlimited"));
		}
		if (m->oil() >= 0) {
			mOil->setText(i18n("Oil: %1").arg(m->oil()));
		} else {
			mOil->setText(i18n("Oil: unlimited"));
		}
	}

	virtual bool useUpdateTimer()
	{
		Unit* u = 0;
		if (!u) {
			return false;
		}
		return true;
	}

private:
	BoUfoLabel* mMinerals;
	BoUfoLabel* mOil;
};



static void connectDisconnect(bool connect, const QObject* sender, const char* signal, const QObject* receiver, const char* slot)
{
 if (connect) {
	QObject::connect(sender, signal, receiver, slot);
 } else {
	QObject::disconnect(sender, signal, receiver, slot);
 }
}

BoUnitDisplayBase::BoUnitDisplayBase(BosonCommandFrame* frame)
	: BoUfoWidget()
{
 hide();
 mCommandFrame = frame;
 mUpdateTimer = false;
 mAvailableInGame = false;
 mAvailableInEditor = false;
 mGameMode = true;
 if (!frame) {
	boError(220) << k_funcinfo << "NULL cmdFrame" << endl;
	return;
 }
 cmdFrame()->addUnitDisplayWidget(this);
}

BoUnitDisplayBase::~BoUnitDisplayBase()
{
}

bool BoUnitDisplayBase::showUnit(Unit* unit)
{
 if (isAvailable() && unit && display(unit)) {
	show();
	mUpdateTimer = true;
	return true;
 }
 hide();
 mUpdateTimer = false;
 return false;
}


class BosonInfoWidget : public BoUfoCustomWidget
{
public:
	BosonInfoWidget();

	void showUnit(const PlayerIO* player, const UnitProperties* prop, bool gameMode = true);
	void showTechnology(const PlayerIO* player, const UpgradeProperties* prop, bool gameMode = true);

	virtual void paintWidget();

private:
	BoUfoLabel* mInfo;
};

BosonInfoWidget::BosonInfoWidget()
	: BoUfoCustomWidget()
{
 hide();
 setLayoutClass(UHBoxLayout);

 BoUfoWidget* spacer = new BoUfoWidget;
 spacer->setPreferredWidth(5);

 mInfo = new BoUfoLabel();
 mInfo->setPreferredWidth(200);
 mInfo->setPreferredHeight(1000);

 addWidget(spacer);
 addWidget(mInfo);
}

void BosonInfoWidget::showUnit(const PlayerIO* player, const UnitProperties* prop, bool gameMode)
{
 if (!prop) {
	hide();
	return;
 }

 QString info;
 info = i18n("%1\n").arg(prop->name());
 if (gameMode) {
	info += QString("Minerals: %2\nOil: %3\n")
			.arg(prop->mineralCost()).arg(prop->oilCost());
	// Check if the unit can be built
	QString missingRequirements;
	QValueList<unsigned long int> requirements = prop->requirements();
	for (QValueList<unsigned long int>::Iterator it = requirements.begin(); it != requirements.end(); ++it) {
		if (!player->hasUnitWithType(*it)) {
			missingRequirements.append("  " + player->unitProperties(*it)->name() + "\n");
		}
	}
	if (!missingRequirements.isEmpty()) {
		info += "\nThis unit has missing requirements:\n";
		info += missingRequirements;
	}
 }
 if (!prop->description().isEmpty()) {
	if (!info.isEmpty()) {
		info.append("\n");
	}
	info.append(prop->description());
 }

 mInfo->setText(info);
 show();
}

void BosonInfoWidget::showTechnology(const PlayerIO* player, const UpgradeProperties* prop, bool gameMode)
{
 if (!prop) {
	hide();
	return;
 }

 QString info;
 info = i18n("%1\n").arg(prop->upgradeName());
 if (gameMode) {
	// TODO: if the requirements are not met yet: display missing
	// requirements
	info += i18n("Minerals: %2\nOil: %3\n")
			.arg(prop->mineralCost()).arg(prop->oilCost());
 }
 if (!prop->upgradeDescription().isEmpty()) {
	if (!info.isEmpty()) {
		info.append("\n");
	}
	info.append(prop->upgradeDescription());
 }

 mInfo->setText(info);
 show();
}

void BosonInfoWidget::paintWidget()
{
 /*if (!prop) {
	return;
 }

 glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_VIEWPORT_BIT);

 // Render a semi-transparent rectangle
 glEnable(GL_BLEND);
 glColor4f(0.0, 0.0, 0.0, 0.75);
 QPoint p = rootLocation();
 glRecti(x(), y(), x() + width(), y() + height());

 glPopAttrib();*/
}


class BosonCommandFramePrivate
{
public:
	BosonCommandFramePrivate()
	{
		mLocalPlayerIO = 0;
		mEventListener = 0;
		mSelection = 0;
		mCursorRootPos = 0;

		mUnitViewWidget = 0;
		mUnitView = 0;
		mUnitActionsBox = 0;
		mUnitActions = 0;
		mUnitDisplayBox = 0;
		mSelectionScrollWidget = 0;
		mSelectionWidget = 0;
		mPlacementScrollWidget = 0;
		mPlacementWidget = 0;
		mUnitConfigWidget = 0;

		mConstructionProgress = 0;
		mMinerWidget = 0;
		mResourceMineWidget = 0;
		mInfoWidget = 0;

		mProduceActions = 0;
	}

	bool mGameMode;
	PlayerIO* mLocalPlayerIO;
	BoCommandFrameEventListener* mEventListener;
	BoSelection* mSelection;
	QTimer mUpdateTimer;
	const QPoint* mCursorRootPos;

	QPtrList<BoUnitDisplayBase> mUnitDisplayWidgets;

	BoUfoWidget* mUnitViewWidget;
	BosonUnitView* mUnitView;
	BoUfoVBox* mUnitActionsBox;
	BoActionsWidget* mUnitActions;
	BoUfoVBox* mUnitDisplayBox;
	BoUfoVBox* mSelectionScrollWidget;
	BosonOrderWidget* mSelectionWidget;
	BoUfoVBox* mPlacementScrollWidget;
	BosonOrderWidget* mPlacementWidget; // editor
	EditorUnitConfigWidget* mUnitConfigWidget;

	BoConstructionProgress* mConstructionProgress;
	BoHarvesterWidget* mMinerWidget;
	BoResourceMineWidget* mResourceMineWidget;
	BosonInfoWidget* mInfoWidget;

	ProduceActionCollection* mProduceActions;
};

BosonCommandFrame::BosonCommandFrame()
	: BoUfoWidget()
{
 d = new BosonCommandFramePrivate;
 d->mGameMode = true;;
 mSelectedUnit = 0;
 d->mProduceActions = new ProduceActionCollection();

 setLayoutClass(UVBoxLayout);

 connect(&d->mUpdateTimer, SIGNAL(timeout()),
		this, SLOT(slotUpdate()));

 initUnitView();
 initUnitActions();
 initUnitDisplayBox();
 initSelectionWidget();
 initPlacementWidget();
 initGamePlugins();
 initEditorPlugins();
 initUnitInfo();

 BoUfoWidget* stretch = new BoUfoWidget();
 stretch->setStretch(1);
 addWidget(stretch);

 // we force an update of the game mode
 d->mGameMode = false;
 setGameMode(true);
}

BosonCommandFrame::~BosonCommandFrame()
{
 d->mUnitDisplayWidgets.clear();
 delete d->mEventListener;
 delete d->mProduceActions;
 delete d;
}

void BosonCommandFrame::initUnitView()
{
 BoUfoHBox* hbox = new BoUfoHBox();
 addWidget(hbox);
 d->mUnitViewWidget = new BoUfoWidget();
 d->mUnitViewWidget->setName("UnitViewWidget");
 d->mUnitView = new BosonUnitView();
 d->mUnitViewWidget->addWidget(d->mUnitView);

 BoUfoWidget* stretch = new BoUfoWidget();
 stretch->setStretch(1);
 hbox->addWidget(d->mUnitViewWidget);
 hbox->addWidget(stretch);
}

void BosonCommandFrame::initUnitActions()
{
 // the action buttons (move, attack, stop, ...)
 d->mUnitActionsBox = new BoUfoVBox();
 d->mUnitActionsBox->setOpaque(false);
 addWidget(d->mUnitActionsBox);

 d->mUnitActions = new BoActionsWidget();
 d->mUnitActionsBox->addWidget(d->mUnitActions);
}

void BosonCommandFrame::initUnitDisplayBox()
{
  // plugins. in this box everything displaying a unit will be inserted, e.g.
 // construction progress.
 d->mUnitDisplayBox = new BoUfoVBox();
 addWidget(d->mUnitDisplayBox);
}

void BosonCommandFrame::initSelectionWidget()
{
 // TODO: scroll widget!
 d->mSelectionScrollWidget = new BoUfoVBox();
 addWidget(d->mSelectionScrollWidget);
 d->mSelectionWidget = new BosonOrderWidget(this);
 d->mSelectionScrollWidget->addWidget(d->mSelectionWidget);

 connect(d->mSelectionWidget, SIGNAL(signalSelectUnit(Unit*)),
		this, SIGNAL(signalSelectUnit(Unit*)));
 connect(d->mSelectionWidget, SIGNAL(signalUnitTypeHighlighted(const PlayerIO*, const UnitProperties*)),
		this, SLOT(slotUnitTypeHighlighted(const PlayerIO*, const UnitProperties*)));
 connect(d->mSelectionWidget, SIGNAL(signalTechnologyHighlighted(const PlayerIO*, const UpgradeProperties*)),
		this, SLOT(slotTechnologyHighlighted(const PlayerIO*, const UpgradeProperties*)));
}

void BosonCommandFrame::initPlacementWidget()
{
 // TODO: scroll widget!
 d->mPlacementScrollWidget = new BoUfoVBox();
 addWidget(d->mPlacementScrollWidget);
 d->mPlacementWidget = new BosonOrderWidget(this);
 d->mPlacementScrollWidget->addWidget(d->mPlacementWidget);
 d->mPlacementWidget->hide();
 d->mPlacementWidget->setGridLayoutColumns(5);
 connect(d->mPlacementWidget, SIGNAL(signalUnitTypeHighlighted(const PlayerIO*, const UnitProperties*)),
		this, SLOT(slotUnitTypeHighlighted(const PlayerIO*, const UnitProperties*)));
 connect(d->mPlacementWidget, SIGNAL(signalTechnologyHighlighted(const PlayerIO*, const UpgradeProperties*)),
		this, SLOT(slotTechnologyHighlighted(const PlayerIO*, const UpgradeProperties*)));

}

void BosonCommandFrame::initUnitInfo()
{
 d->mInfoWidget = new BosonInfoWidget();
 addWidget(d->mInfoWidget);
}

void BosonCommandFrame::setLocalPlayerIO(PlayerIO* io)
{
 delete d->mEventListener;
 d->mEventListener = 0;
 d->mLocalPlayerIO = io;

 if (!d->mLocalPlayerIO) {
	return;
 }
 Boson* game = boGame;
 BO_CHECK_NULL_RET(game);
 if (d->mLocalPlayerIO && game->gameMode()) {
	// AB: WARNING: the event listener will not get loaded correctly when
	// loading a game!
	// this is harmless currently, as we do not use scripts and therefore
	// there is nothing to be loaded.
	BoEventManager* manager = game->eventManager();
	d->mEventListener = new BoCommandFrameEventListener(io, manager, this);
	connect(d->mEventListener, SIGNAL(signalUpdateSelection()),
			this, SLOT(slotUpdateSelection()));
	connect(d->mEventListener, SIGNAL(signalUpdateProductionOptions()),
			this, SLOT(slotUpdateProductionOptions()));
	connect(d->mEventListener, SIGNAL(signalUpdateProduction(unsigned long int)),
			this, SLOT(slotUpdateProduction(unsigned long int)));
	connect(d->mEventListener, SIGNAL(signalFacilityConstructed(unsigned long int)),
			this, SLOT(slotConstructionCompleted(unsigned long int)));
	connect(d->mEventListener, SIGNAL(signalUnitDestroyed(unsigned long int)),
			this, SLOT(slotUnitDestroyed(unsigned long int)));
 }
}

PlayerIO* BosonCommandFrame::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

BoSelection* BosonCommandFrame::selection() const
{
 return d->mSelection;
}

void BosonCommandFrame::slotUpdateSelection()
{
 slotSelectionChanged(selection());
}

void BosonCommandFrame::slotSelectionChanged(BoSelection* selection)
{
 Unit* leader = 0;
 d->mSelection = selection;
 if (!selection || selection->count() == 0) {
	clearSelection();
 } else {
	leader = selection->leader();
	if (!leader) {
		boError(220) << k_funcinfo << "non-empty selection, but NULL leader" << endl;
		leader = 0;
		clearSelection();
	} else if (!leader->owner()) {
		boError(220) << k_funcinfo << "group leader has NULL owner" << endl;
		leader = 0;
		clearSelection();
	} else if (leader->isDestroyed()) {
		boWarning(220) << k_funcinfo << "group leader is destroyed" << endl;
		leader = 0;
	}
 }

 // note that these functions must allow leader == 0!
 // then the widget should display no content or so.
 setSelectedUnit(leader);
 if (!selection || selection->count() == 0) {
	// display nothing at all!
	setProduction(0);
	d->mUpdateTimer.stop();
 } else if (selection->count() > 1) {
	d->mSelectionWidget->showUnits(selection->allUnits());
 } else {
	setProduction(leader);
 }
}

void BosonCommandFrame::slotConstructionCompleted(unsigned long int facilityId)
{
 if (!selection() || selection()->count() == 0) {
	return;
 }
 if (!selection()->leader()) {
	return;
 }
 if (selection()->leader()->id() == facilityId) {
	slotSelectionChanged(selection());
 }
}

void BosonCommandFrame::slotUnitDestroyed(unsigned long int id)
{
 if (!selection() || selection()->count() == 0) {
	return;
 }
 if (!selection()->leader()) {
	return;
 }
 if (selection()->leader()->id() == id) {
	slotSelectionChanged(selection());
 }
}

void BosonCommandFrame::slotProduce(const BoSpecificAction& _action)
{
 boDebug(220) << k_funcinfo << endl;
 BoSpecificAction action = _action;
 if (selectedUnit()) {
	if (!localPlayerIO() || !localPlayerIO()->ownsUnit(selectedUnit())) {
		boError(220) << k_funcinfo << "local player doesn't own selected unit" << endl;
		return;
	}
	// FIXME: is there any way not to hardcode this
	if (action.type() != ActionStopProduceUnit && action.type() != ActionStopProduceTech) {
		ProductionPlugin* pp = (ProductionPlugin*)selectedUnit()->plugin(UnitPlugin::Production);
		if (!pp) {
			boWarning(220) << k_funcinfo << "NULL production plugin?!" << endl;
			return;
		}
		if (pp->completedProductionType() == action.productionType() &&
				pp->completedProductionId() == action.productionId()) {
			// the player did not start to place the completed production.
			// enable the placement preview (aka lock the action)
			action.setType(ActionPlacementPreview);
			emit signalAction(action);
			return; // do NOT start to produce more here.
		}
	}
 }
 boDebug(220) << k_funcinfo << "Emitting signalAction(action)  (for producing; type: " << action.type() << ")" << endl;
 action.setUnit(selectedUnit());
 emit signalAction(action);

 // This has to be done with QTimer since the action will be sent over the net
 //  and thus won't take effect immediately
 // TODO: any timeout value, even 0, works for singleplayer games; however for
 //  multiplayer games, 50 msecs might not be enough. Some clever system should
 //  be implemented which would update production when the message actually
 //  arrives
 QTimer::singleShot(50, this, SLOT(slotUpdateProductionOptions()));
}

void BosonCommandFrame::slotPlaceUnit(const BoSpecificAction& action)
{
 if (!localPlayerIO()) {
	boError(220) << k_funcinfo << "NULL local player IO" << endl;
	return;
 }
#warning FIXME: dont use Player pointer, use PlayerIO instead
 emit signalPlaceUnit(action.productionId(), localPlayerIO()->player());
 BoSpecificAction a;
 a.setType(ActionPlacementPreview);
 emit signalAction(a);
}

void BosonCommandFrame::slotPlaceGround(unsigned int textureCount, unsigned char* alpha)
{
 emit signalPlaceGround(textureCount, alpha);
 BoSpecificAction a;
 a.setType(ActionPlacementPreview);
 emit signalAction(a);
}

void BosonCommandFrame::setGameMode(bool game)
{
 if (d->mGameMode == game) {
	return;
 }
 d->mGameMode = game;

 d->mUnitActionsBox->setVisible(game);
 d->mPlacementWidget->setVisible(!game);
 connectDisconnect(game,
		d->mUnitActions, SIGNAL(signalAction(const BoSpecificAction&)),
		this, SIGNAL(signalAction(const BoSpecificAction&)));
 connectDisconnect(game,
		d->mSelectionWidget, SIGNAL(signalAction(const BoSpecificAction&)),
		this, SLOT(slotProduce(const BoSpecificAction&)));
 connectDisconnect(!game,
		d->mPlacementWidget, SIGNAL(signalAction(const BoSpecificAction&)),
		this, SLOT(slotPlaceUnit(const BoSpecificAction&)));
 connectDisconnect(!game,
		d->mPlacementWidget, SIGNAL(signalPlaceGround(unsigned int, unsigned char*)),
		this, SLOT(slotPlaceGround(unsigned int, unsigned char*)));

 for (QPtrListIterator<BoUnitDisplayBase> it(d->mUnitDisplayWidgets); it.current(); ++it) {
	it.current()->setGameMode(game);
 }
}

void BosonCommandFrame::clearSelection()
{
 setSelectedUnit(0);
 setProduction(0); // do NOT call when multiple-unit selection should get displayed

 d->mSelectionWidget->hideOrderButtons();
 hidePluginWidgets();
 d->mUpdateTimer.stop();
}

void BosonCommandFrame::setSelectedUnit(Unit* unit)
{
 mSelectedUnit = 0;
 d->mUnitView->setUnit(unit);
 showUnitActions(unit);
 showPluginWidgetsForUnit(unit);
 setProduction(0); // reset the order buttons. this widget will be filled with content later
 mSelectedUnit = unit;

 startStopUpdateTimer();
}

void BosonCommandFrame::setProduction(Unit* unit)
{
 boDebug(220) << k_funcinfo << endl;
 if (!unit) {
	d->mSelectionWidget->hideOrderButtons();
	return;
 }
 if (!d->mGameMode && unit) {
	setProduction(0);
	return;
 }
 BO_CHECK_NULL_RET(localPlayerIO());
 SpeciesTheme* speciesTheme = unit->speciesTheme();
 BO_CHECK_NULL_RET(speciesTheme);

 // don't display production items of other players
 if (!localPlayerIO()->ownsUnit(unit)) {
	return;
 }

 ProductionPlugin* production = (ProductionPlugin*)unit->plugin(UnitPlugin::Production);
 if (!production) {
	// this unit cannot produce.
	d->mSelectionWidget->hideOrderButtons();
	return;
 }
 QValueList<BoSpecificAction> actions;

 QValueList<int> grayOutActions;

 QValueList<unsigned long int> grayedOutUnits;
 QValueList<unsigned long int> unitsList = production->allUnitProductions(0, &grayedOutUnits);
 for (QValueList<unsigned long int>::iterator it = unitsList.begin(); it != unitsList.end(); ++it) {
	BoSpecificAction a(d->mProduceActions->produceActionFor(speciesTheme->unitProperties(*it)));
	a.setType(ActionProduceUnit);
	a.setProductionId(*it);
	a.setUnit(unit);
	actions.append(a);
	if (grayedOutUnits.contains(*it)) {
		grayOutActions.append(actions.count() - 1);
	}
 }

 QValueList<unsigned long int> grayedOutTechs;
 QValueList<unsigned long int> techList = production->allTechnologyProductions(0, &grayedOutTechs);
 for (QValueList<unsigned long int>::iterator it = techList.begin(); it != techList.end(); it++) {
	BoSpecificAction a(d->mProduceActions->produceActionFor(speciesTheme->technology(*it), production->speciesTheme()));
	a.setType(ActionProduceTech);
	a.setProductionId(*it);
	a.setUnit(unit);
	actions.append(a);
	if (grayedOutTechs.contains(*it)) {
		grayOutActions.append(actions.count() - 1);
	}
 }

 // Set buttons
 d->mSelectionWidget->setOrderButtons(actions, grayOutActions);
 d->mSelectionWidget->show();

 startStopUpdateTimer();
}

void BosonCommandFrame::hidePluginWidgets()
{
 QPtrListIterator<BoUnitDisplayBase> it(d->mUnitDisplayWidgets);
 while (it.current()) {
	it.current()->hide();
	++it;
 }
}

void BosonCommandFrame::showPluginWidgetsForUnit(Unit* unit)
{
 hidePluginWidgets();
 if (!unit) {
	return;
 }
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(unit->owner());
 QPtrListIterator<BoUnitDisplayBase> it(d->mUnitDisplayWidgets);
 for (; it.current(); ++it) {
	// In editor mode, we ignore friendly/unfriendly settings
	// In game mode:
	// For enemy units, we show nothing
	// For friendly or even our own units, we show plugins
	if (d->mGameMode) {
		if (localPlayerIO()->isEnemy(unit)) {
			continue;
		}
	}
	it.current()->showUnit(unit);
 }
 startStopUpdateTimer();
}

void BosonCommandFrame::addUnitDisplayWidget(BoUnitDisplayBase* w)
{
 if (!w || d->mUnitDisplayWidgets.contains(w)) {
	return;
 }
 d->mUnitDisplayWidgets.append(w);
}

bool BosonCommandFrame::checkUpdateTimer() const
{
 if (!selectedUnit()) {
	return false;
 }
 QPtrListIterator<BoUnitDisplayBase> it(d->mUnitDisplayWidgets);
 bool use = false;
 for (; it.current() && !use; ++it) {
	use = it.current()->updateTimer();
 }
 ProductionPlugin* production = (ProductionPlugin*)selectedUnit()->plugin(UnitPlugin::Production);
 if (production && production->hasProduction()) {
	return use = true;
 }
 return use;
}

void BosonCommandFrame::startStopUpdateTimer()
{
 if (!selectedUnit() || !checkUpdateTimer()) {
	d->mUpdateTimer.stop();
	return;
 } else {
	if (!d->mUpdateTimer.isActive()) {
		d->mUpdateTimer.start(UPDATE_TIMEOUT);
	}
 }
}

void BosonCommandFrame::showUnitActions(Unit* unit)
{
 if (!unit || unit->isDestroyed()) {
	d->mUnitActions->hide();
	return;
 }
 if (unit->isFacility()) {
	UnitConstruction* c = unit->construction();
	if (!c->isConstructionComplete()) {
		d->mUnitActions->hide();
		return;
	}
 }
 if (!localPlayerIO() || !localPlayerIO()->ownsUnit(unit)) {
	d->mUnitActions->hide();
	return;
 }
 if (selection()) {
	d->mUnitActions->showUnitActions(unit, selection()->allUnits());
 } else {
	d->mUnitActions->showUnitActions(unit, QPtrList<Unit>());
 }
 d->mUnitActions->show();
}

void BosonCommandFrame::slotUpdate()
{
 if (!selectedUnit()) {
	d->mUpdateTimer.stop();
	return;
 }

 // check if we still need the update timer and stop it if it is unused
 startStopUpdateTimer();

 // update the plugin widgets
 showPluginWidgetsForUnit(selectedUnit());

 ProductionPlugin* production = (ProductionPlugin*)selectedUnit()->plugin(UnitPlugin::Production);
 if (production && production->hasProduction()) {
	d->mSelectionWidget->productionAdvanced(selectedUnit(), production->productionProgress());
 }
}

void BosonCommandFrame::slotUpdateProduction(unsigned long int id)
{
 if (!selectedUnit()) {
	return;
 }
 if (selectedUnit()->id() != id) {
	return;
 }
 slotUpdateProduction(selectedUnit());
}

void BosonCommandFrame::slotUpdateProduction(Unit* f)
{
 if (!f) {
	boError(220) << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (selectedUnit() == f && d->mSelectionWidget->isProduceAction()) {
	boDebug(220) << k_funcinfo << endl;
	setProduction(f);
 }
}

void BosonCommandFrame::slotUpdateProductionOptions()
{
 if (selectedUnit() && d->mSelectionWidget->isProduceAction()) {
	boDebug(220) << k_funcinfo << endl;
	setProduction(selectedUnit());
 }
}

void BosonCommandFrame::initGamePlugins()
{
// the construction progress
 d->mConstructionProgress = new BoConstructionProgress(this);
 d->mConstructionProgress->setAvailableInGame(true);
 d->mUnitDisplayBox->addWidget(d->mConstructionProgress);

// the miner display (minerals/oil)
 d->mMinerWidget = new BoHarvesterWidget(this);
 d->mMinerWidget->setAvailableInGame(true);
 d->mUnitDisplayBox->addWidget(d->mMinerWidget);

 // Resource mine display (how much minerals/oil is left in a mine)
 d->mResourceMineWidget = new BoResourceMineWidget(this);
 d->mResourceMineWidget->setAvailableInGame(true);
 d->mUnitDisplayBox->addWidget(d->mResourceMineWidget);
}

void BosonCommandFrame::initEditorPlugins()
{
 d->mUnitConfigWidget = new EditorUnitConfigWidget(this);
 d->mUnitConfigWidget->setAvailableInEditor(true);
 d->mUnitDisplayBox->addWidget(d->mUnitConfigWidget);
 connect(d->mUnitConfigWidget, SIGNAL(signalUpdateUnit()),
		this, SLOT(slotUpdateUnitConfig()));
}

void BosonCommandFrame::slotSetGroundTheme(BosonGroundTheme* theme)
{
 d->mPlacementWidget->setGroundTheme(theme);
}

void BosonCommandFrame::placeGround()
{
 d->mPlacementWidget->setOrderButtonsGround();
}

void BosonCommandFrame::placeMobiles(PlayerIO* io)
{
 boDebug(220) << k_funcinfo << endl;
 if (!io) {
	boError(220) << k_funcinfo << "NULL io" << endl;
	return;
 }
 SpeciesTheme* theme = io->speciesTheme();
 if (!theme) {
	boError(220) << k_funcinfo << "NULL speciestheme" << endl;
	return;
 }
 QValueList<long unsigned int> units = theme->allMobiles();
 QValueList<long unsigned int>::iterator it;
 QValueList<BoSpecificAction> actions;
 for (it = units.begin(); it != units.end(); ++it) {
	BoSpecificAction a(d->mProduceActions->produceActionFor(theme->unitProperties(*it)));
	a.setType(ActionPlacementPreview);
	a.setProductionId(*it);
	a.setProductionOwner(io);
	actions.append(a);
 }
 d->mPlacementWidget->setOrderButtons(actions);
}

void BosonCommandFrame::placeFacilities(PlayerIO* io)
{
 boDebug(220) << k_funcinfo << endl;
 if (!io) {
	boError(220) << k_funcinfo << "NULL io" << endl;
	return;
 }
 SpeciesTheme* theme = io->speciesTheme();
 if (!theme) {
	boError(220) << k_funcinfo << "NULL speciestheme" << endl;
	return;
 }
 QValueList<long unsigned int> units = theme->allFacilities();
 QValueList<long unsigned int>::iterator it;
 QValueList<BoSpecificAction> actions;
 for (it = units.begin(); it != units.end(); ++it) {
	BoSpecificAction a(d->mProduceActions->produceActionFor(theme->unitProperties(*it)));
	a.setType(ActionPlacementPreview);
	a.setProductionId(*it);
	a.setProductionOwner(io);
	actions.append(a);
 }
 d->mPlacementWidget->setOrderButtons(actions);
}

void BosonCommandFrame::slotUpdateUnitConfig()
{
 BO_CHECK_NULL_RET(selectedUnit());
 BO_CHECK_NULL_RET(d->mUnitConfigWidget);

 // TODO: support undo
 // apply changed in the config widget to the unit
 d->mUnitConfigWidget->updateUnit(selectedUnit());
}

void BosonCommandFrame::slotUnitTypeHighlighted(const PlayerIO* player, const UnitProperties* prop)
{
 d->mInfoWidget->showUnit(player, prop, d->mGameMode);
}

void BosonCommandFrame::slotTechnologyHighlighted(const PlayerIO* player, const UpgradeProperties* prop)
{
 d->mInfoWidget->showTechnology(player, prop, d->mGameMode);
}

const QPoint* BosonCommandFrame::cursorRootPos() const
{
 return d->mCursorRootPos;
}

void BosonCommandFrame::setCursorRootPos(const QPoint* pos)
{
 d->mCursorRootPos = pos;
}

