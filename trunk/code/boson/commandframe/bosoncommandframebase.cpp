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

#include "bosoncommandframebase.h"
#include "bosoncommandframebase.moc"

#include "bosonorderwidget.h"
#include "bosonunitview.h"
#include "../unit.h"
#include "../player.h"
#include "../speciestheme.h"
#include "../boselection.h"
#include "../defines.h"
#include "bodebug.h"

#include <klocale.h>

#include <qlayout.h>
#include <qscrollview.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qvbox.h>

#define UPDATE_TIMEOUT 200

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
	boError() << k_funcinfo << "NULL cmdFrame" << endl;
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
		mSelectionWidget = 0;

		mUnitActionsBox = 0;
		mUnitDisplayBox = 0;

		mOwner = 0;
	}

	QVBoxLayout* mTopLayout;
	QVBoxLayout* mPlacementLayout; // for editor

	BosonUnitView* mUnitView;
	BosonOrderWidget* mSelectionWidget;

	QVBox* mUnitActionsBox; // unit actions - move, attack, stop, ...
	QPtrList<BoUnitDisplayBase> mUnitDisplayWidgets;
	QVBox* mUnitDisplayBox; // plugin widgets

	Player* mOwner;

	QTimer mUpdateTimer;
};

BosonCommandFrameBase::BosonCommandFrameBase(QWidget* parent) : QFrame(parent, "cmd frame")
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

 // the unit view. this displays the leader of a selection, showing further
 // information about this unit.
 d->mUnitView = new BosonUnitView(this);
 d->mTopLayout->addWidget(d->mUnitView, 0, AlignHCenter);
 d->mUnitView->setBackgroundOrigin(WindowOrigin);

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
 boDebug() << k_funcinfo << endl;
 Unit* leader = 0;
 if (!selection || selection->count() == 0) {
	clearSelection();
 } else {
	leader = selection->leader();
	if (!leader) {
		boError() << k_funcinfo << "non-empty selection, but NULL leader" << endl;
		leader = 0;
		clearSelection();
	} else if (!leader->owner()) {
		boError() << k_funcinfo << "group leader has NULL owner" << endl;
		leader = 0;
		clearSelection();
	} else {
		if (leader->isDestroyed()) {
			boWarning() << k_funcinfo << "group leader is destroyed" << endl;
			leader = 0;
		}
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
 d->mUnitView->setUnit(unit); // also for NULL unit
 showUnitActions(unit); // also for NULL unit
 if (!unit) {
	hidePluginWidgets();
	return;
 }
 Player* owner = unit->owner();
 if (!owner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (!unit->speciesTheme()) {
	boError() << k_funcinfo << "NULL speciestheme" << endl;
	return;
 }

 if (owner != localPlayer()) {
	// hmm.. is this correct? maybe display for plugins anyway? dont know...
	return;
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

void BosonCommandFrameBase::slotPlaceUnit(ProductionType t, unsigned long int unitType)
{
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }
 if (t != ProduceUnit) {
	boError() << k_funcinfo << "Only ProduceUnit supported" << endl;
	return;
 }
 emit signalPlaceUnit(unitType, localPlayer());
}

void BosonCommandFrameBase::slotProduce(ProductionType type, unsigned long int id)
{
 if (selectedUnit()) {
	if (localPlayer() != selectedUnit()->owner()) {
		boError() << k_funcinfo << "local owner != selected unit owner" << endl;
		return;
	}
 }
 emit signalProduce(type, id, (UnitBase*)selectedUnit(), localPlayer());
}

void BosonCommandFrameBase::slotStopProduction(ProductionType type, unsigned long int id)
{
 if (selectedUnit()) {
	if (localPlayer() != selectedUnit()->owner()) {
		boError() << k_funcinfo << "local player != owner of selected unit" << endl;
		return;
	}
 }
 emit signalStopProduction(type, id, (UnitBase*)selectedUnit(), localPlayer());
}

void BosonCommandFrameBase::slotUpdateProduction(Unit* f)
{
 if (!f) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (selectedUnit() == f) {
	boDebug() << k_funcinfo << endl;
	setProduction(f);
 }
}

void BosonCommandFrameBase::slotUpdateProductionOptions()
{
 if (selectedUnit()) {
	boDebug() << k_funcinfo << endl;
	setProduction(selectedUnit());
 }
}

void BosonCommandFrameBase::slotUpdate()
{
 boDebug() << k_funcinfo << endl;
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

void BosonCommandFrameBase::slotSetButtonsPerRow(int b)
{
 selectionWidget()->setButtonsPerRow(b);
}

void BosonCommandFrameBase::reparentMiniMap(QWidget* map)
{
 if (!map) {
	boError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 map->reparent(this, QPoint(0,0));
 map->hide();
 d->mTopLayout->insertWidget(0, map, 0, AlignHCenter);
}

void BosonCommandFrameBase::resizeEvent(QResizeEvent* e)
{
 if (minimumSize().width() < sizeHint().width()) {
//	setMinimumWidth(sizeHint().width());
 }
 QFrame::resizeEvent(e);
}

QScrollView* BosonCommandFrameBase::addPlacementView()
{
 BoScrollView* scrollView = new BoScrollView(this);
 d->mPlacementLayout->addWidget(scrollView, 1);
 scrollView->show();
 return scrollView;
}

