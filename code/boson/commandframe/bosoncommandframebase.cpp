/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosoncommandframebase.h"
#include "bosoncommandframebase.moc"

#include "bosoncommandframe.h"
#include "editorcommandframe.h"

#include "bosonorderwidget.h"
#include "bosonunitview.h"
#include "../unit.h"
#include "../player.h"
#include "../playerio.h"
#include "../speciestheme.h"
#include "../boselection.h"
#include "../unitplugins.h"
#include "../defines.h"
#include "bodebug.h"

#include <klocale.h>

#include <qlayout.h>
#include <qscrollview.h>
#include <qtimer.h>
#include <qvbox.h>

#define UPDATE_TIMEOUT 200

BosonCommandFrameInterface* BosonCommandFrameFactory::createCommandFrame2(QWidget* parent, bool game)
{
 if (game) {
	return new BosonCommandFrame(parent);
 } else {
	return new EditorCommandFrame(parent);
 }
 return 0;
}

class BoScrollView : public QScrollView
{
public:
	BoScrollView(QWidget* parent) : QScrollView(parent)
	{
		setResizePolicy(QScrollView::AutoOneFit);
		setBackgroundOrigin(WindowOrigin);
		viewport()->setBackgroundOrigin(WindowOrigin);
		if (parent) {
			setBackgroundMode(parent->backgroundMode());
		}
		viewport()->setBackgroundMode(backgroundMode());
		setHScrollBarMode(QScrollView::AlwaysOff);
	}

	~BoScrollView()
	{
	}
};

BoUnitDisplayBase::BoUnitDisplayBase(BosonCommandFrameBase* frame, QWidget* parent) : QWidget(parent)
{
 setBackgroundOrigin(WindowOrigin);
 hide();
 mCommandFrame = frame;
 mUpdateTimer = false;
 if (!frame) {
	boError(220) << k_funcinfo << "NULL cmdFrame" << endl;
	return;
 }
 cmdFrame()->addUnitDisplayWidget(this);
}

BoUnitDisplayBase::~BoUnitDisplayBase()
{
}

class BosonCommandFrameBase::BosonCommandFrameBasePrivate
{
public:
	BosonCommandFrameBasePrivate()
	{
		mTopLayout = 0;
		mPlacementLayout = 0;

		mUnitView = 0;
		mUnitViewLayout = 0;
		mSelectionWidget = 0;

		mUnitActionsBox = 0;
		mUnitDisplayBox = 0;

		mOwner = 0;

		mSelection = 0;
	}

	QVBoxLayout* mTopLayout;
	QVBoxLayout* mPlacementLayout; // for editor
	QVBoxLayout* mUnitViewLayout; // for game mode

	BosonUnitView* mUnitView;
	BosonOrderWidget* mSelectionWidget;

	QVBox* mUnitActionsBox; // unit actions - move, attack, stop, ...
	QPtrList<BoUnitDisplayBase> mUnitDisplayWidgets;
	QVBox* mUnitDisplayBox; // plugin widgets

	Player* mOwner;

	QTimer mUpdateTimer;

	BoSelection* mSelection;
};

BosonCommandFrameBase::BosonCommandFrameBase(QWidget* parent) : BosonCommandFrameInterface(parent, "cmd frame")
{
 d = new BosonCommandFrameBasePrivate;
 mSelectedUnit = 0;

 setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
 setMinimumSize(230, 200); // FIXME hardcoded value

 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdate()));

 d->mTopLayout = new QVBoxLayout(this, 5, 5); // FIXME: hardcoded - maybe use KDialog::marginHint(), KDialog::spacingHint()

 // this is unused in game mode. we add it here to avoid trouble with
 // insertWidget() in editor.
 d->mPlacementLayout = new QVBoxLayout;
 d->mTopLayout->addLayout(d->mPlacementLayout, 2);

 // use addUnitView() to actually add the unit view. we reserve a place for it
 // here.
 d->mUnitViewLayout = new QVBoxLayout(d->mTopLayout);

 // the action buttons (move, attack, stop, ...)
 d->mUnitActionsBox = new QVBox(this);
 d->mUnitActionsBox->setBackgroundOrigin(WindowOrigin);
 d->mTopLayout->addWidget(d->mUnitActionsBox);
 d->mUnitActionsBox->hide(); // hidden by default as it's unused for editor

 // plugins. in this box everything displaying a unit will be inserted, e.g.
 // construction progress.
 d->mUnitDisplayBox = new QVBox(this);
 d->mUnitDisplayBox->setBackgroundOrigin(WindowOrigin);
 d->mTopLayout->addWidget(d->mUnitDisplayBox);


 // the selected units. we use a scrollview here, since a player can e.g. select
 // 20 units and we need to display them all.
 // this widget is used for order buttons (when you select a factory), too
 // TODO: limit max number of selected units. maybve 50 or so - otherwise we
 // have too much memory allocated for widget!
 BoScrollView* selectScrollView = new BoScrollView(this);
 d->mTopLayout->addWidget(selectScrollView, 1);
 d->mSelectionWidget = new BosonOrderWidget(selectScrollView->viewport());
 selectScrollView->addChild(d->mSelectionWidget);
 d->mSelectionWidget->setBackgroundOrigin(WindowOrigin);

 // common to editor and game mode:
 connect(d->mSelectionWidget, SIGNAL(signalSelectUnit(Unit*)),
		this, SIGNAL(signalSelectUnit(Unit*)));

 setBackgroundOrigin(WindowOrigin);
 show();
}

BosonCommandFrameBase::~BosonCommandFrameBase()
{
 d->mUnitDisplayWidgets.clear();
 delete d;
}

BosonOrderWidget* BosonCommandFrameBase::selectionWidget() const
{
 return d->mSelectionWidget;
}

QVBox* BosonCommandFrameBase::unitActionsBox() const
{
 return d->mUnitActionsBox;
}

QVBox* BosonCommandFrameBase::unitDisplayBox() const
{
 return d->mUnitDisplayBox;
}


void BosonCommandFrameBase::addUnitDisplayWidget(BoUnitDisplayBase* w)
{
 if (!w || d->mUnitDisplayWidgets.contains(w)) {
	return;
 }
 d->mUnitDisplayWidgets.append(w);
}

void BosonCommandFrameBase::slotSelectionChanged(BoSelection* selection)
{
 boDebug(220) << k_funcinfo << endl;
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
	selectionWidget()->showUnits(selection->allUnits());
 } else {
	setProduction(leader);
 }
}

void BosonCommandFrameBase::clearSelection()
{
 // display nothing
 setSelectedUnit(0);
 setProduction(0); // do NOT call when multiple-unit selection should get displayed

 hidePluginWidgets();
 d->mUpdateTimer.stop();
}

void BosonCommandFrameBase::setSelectedUnit(Unit* unit)
{
 mSelectedUnit = 0;
 if (d->mUnitView) {
	d->mUnitView->setUnit(unit); // also for NULL unit
 }
 showUnitActions(unit); // also for NULL unit
 if (!unit) {
	hidePluginWidgets();
	return;
 }
 if (!unit->speciesTheme()) {
	boError(220) << k_funcinfo << "NULL speciestheme" << endl;
	return;
 }

 // For enemy units, we show nothing.
 // For friendly units, we show plugins.
 // For our own units, we show everything
 if (!localPlayerIO() || localPlayerIO()->isEnemy(unit)) {
	hidePluginWidgets();
	selectionWidget()->hideOrderButtons();
	return;
 }
 if (!localPlayerIO() || !localPlayerIO()->ownsUnit(unit)) {
	selectionWidget()->hideOrderButtons();
 }
 mSelectedUnit = unit;
}

void BosonCommandFrameBase::setProduction(Unit* unit)
{
 if (!unit) {
	selectionWidget()->hideOrderButtons();
	return;
 }
}

// hide the plugin widgets (unit display widgets)
void BosonCommandFrameBase::hidePluginWidgets()
{
 QPtrListIterator<BoUnitDisplayBase> it(d->mUnitDisplayWidgets);
 while (it.current()) {
	it.current()->hide();
	++it;
 }
}

void BosonCommandFrameBase::setLocalPlayer(Player* p)
{
 d->mOwner = p;
}

Player* BosonCommandFrameBase::localPlayer() const
{
 return d->mOwner;
}

PlayerIO* BosonCommandFrameBase::localPlayerIO() const
{
 if (!localPlayer()) {
	return 0;
 }
 return localPlayer()->playerIO();
}

void BosonCommandFrameBase::slotPlaceUnit(const BoSpecificAction& action)
{
 if (!localPlayerIO()) {
	boError(220) << k_funcinfo << "NULL local player IO" << endl;
	return;
 }
 emit signalPlaceUnit(action.productionId(), localPlayerIO()->player());
 BoSpecificAction a;
 a.setType(ActionPlacementPreview);
 emit signalAction(a);
}

void BosonCommandFrameBase::slotPlaceGround(unsigned int textureCount, unsigned char* alpha)
{
 emit signalPlaceGround(textureCount, alpha);
 BoSpecificAction a;
 a.setType(ActionPlacementPreview);
 emit signalAction(a);
}

void BosonCommandFrameBase::slotProduce(const BoSpecificAction& _action)
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
}

void BosonCommandFrameBase::slotUpdateProduction(Unit* f)
{
 if (!f) {
	boError(220) << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (selectedUnit() == f && selectionWidget()->isProduceAction()) {
	boDebug(220) << k_funcinfo << endl;
	setProduction(f);
 }
}

void BosonCommandFrameBase::slotUpdateProductionOptions()
{
 if (selectedUnit() && selectionWidget()->isProduceAction()) {
	boDebug(220) << k_funcinfo << endl;
	setProduction(selectedUnit());
 }
}

void BosonCommandFrameBase::slotUpdate()
{
 boDebug(220) << k_funcinfo << endl;
 if (!selectedUnit()) {
	d->mUpdateTimer.stop();
	return;
 }

 // check if we still need the update timer and stop it if it is unused
 startStopUpdateTimer();
}

void BosonCommandFrameBase::startStopUpdateTimer()
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

bool BosonCommandFrameBase::checkUpdateTimer() const
{
 if (!selectedUnit()) {
	return false;
 }
 QPtrListIterator<BoUnitDisplayBase> it(d->mUnitDisplayWidgets);
 bool use = false;
 for (; it.current() && !use; ++it) {
	use = it.current()->updateTimer();
 }
 return use;
}

void BosonCommandFrameBase::resizeEvent(QResizeEvent* e)
{
 if (minimumSize().width() < sizeHint().width()) {
//	setMinimumWidth(sizeHint().width());
 }
 BosonCommandFrameInterface::resizeEvent(e);
}

QScrollView* BosonCommandFrameBase::addPlacementView()
{
 BoScrollView* scrollView = new BoScrollView(this);
 d->mPlacementLayout->addWidget(scrollView, 1);
 scrollView->show();
 return scrollView;
}

void BosonCommandFrameBase::addUnitView()
{
 // the unit view. this displays the leader of a selection, showing further
 // information about this unit.
 d->mUnitView = new BosonUnitView(this);
 d->mUnitViewLayout->addWidget(d->mUnitView, 0, AlignHCenter);
 d->mUnitView->setBackgroundOrigin(WindowOrigin);
}

BoSelection* BosonCommandFrameBase::selection() const
{
 return d->mSelection;
}

