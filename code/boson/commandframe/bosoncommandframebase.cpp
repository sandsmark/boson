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

		mUnitView = 0;

		mOrderWidget = 0;

		mOwner = 0;
	}

	QVBoxLayout* mTopLayout;


	BosonUnitView* mUnitView;

	BosonOrderWidget* mOrderWidget;

	QPtrList<BoUnitDisplayBase> mUnitDisplayWidgets;
	QVBox* mUnitDisplayBox;

	Player* mOwner;

	QTimer mUpdateTimer;
};

BosonCommandFrameBase::BosonCommandFrameBase(QWidget* parent) : QFrame(parent, "cmd frame")
{
 init();

 d->mTopLayout = new QVBoxLayout(this, 5, 5); // FIXME: hardcoded - maybe use KDialog::marginHint(), KDialog::spacingHint()

 d->mUnitView = new BosonUnitView(this);
 d->mTopLayout->addWidget(d->mUnitView, 0, AlignHCenter);
 d->mUnitView->setBackgroundOrigin(WindowOrigin);

 d->mUnitDisplayBox = new QVBox(this);
 d->mTopLayout->addWidget(d->mUnitDisplayBox);

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
 d->mOrderWidget = new BosonOrderWidget(actionWidget);
 actionLayout->addWidget(d->mOrderWidget);
 d->mOrderWidget->setBackgroundOrigin(WindowOrigin);

 // common to editor and game mode:
 connect(d->mOrderWidget, SIGNAL(signalSelectUnit(Unit*)),
		this, SIGNAL(signalSelectUnit(Unit*)));

 actionLayout->addStretch(1);
 setBackgroundOrigin(WindowOrigin);
 show();
}

void BosonCommandFrameBase::init()
{
 d = new BosonCommandFrameBasePrivate;
 mSelectedUnit = 0;

 setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
 setMinimumSize(230, 200); // FIXME hardcoded value

 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdate()));
}

BosonCommandFrameBase::~BosonCommandFrameBase()
{
 d->mUnitDisplayWidgets.clear();
 delete d;
}

BosonOrderWidget* BosonCommandFrameBase::orderWidget() const
{
 return d->mOrderWidget;
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
 if (!selection || selection->count() == 0) {
	clearSelection();
	return;
 }
 Unit* leader = selection->leader();
 if (!leader) {
	boError() << k_funcinfo << "non-empty selection, but NULL leader" << endl;
	clearSelection();
	return;
 } else {
	// display group leader in d->mUnitView
	if (!leader->owner()) {
		boError() << k_funcinfo << "group leader has NULL owner" << endl;
		clearSelection();
		return;
	}
	if (leader->isDestroyed()) {
		boWarning() << k_funcinfo << "group leader is destroyed" << endl;
		d->mUnitView->setUnit(0);
		setAction(0);
	} else {
		d->mUnitView->setUnit(leader);
		setAction(leader);
	}
 }

 if (selection->count() > 1) {
	QPtrList<Unit> list = selection->allUnits();
	QPtrListIterator<Unit> it(list);
	for (; it.current(); ++it) {
		d->mOrderWidget->showUnit(it.current());
	}
	d->mOrderWidget->show();
 } else {
	d->mOrderWidget->hide();
 }
}

void BosonCommandFrameBase::clearSelection()
{
 // display nothing
 setAction(0);
 d->mUnitView->setUnit(0);
 d->mOrderWidget->hide();
}

void BosonCommandFrameBase::setAction(Unit* unit)
{
 // this makes sure that they are even hidden if the unit cannot produce or
 // something:
 hideActions();

 mSelectedUnit = 0; // in case hideActions() might change one day...
 if (!unit) {
	// display nothing
	return;
 }
 Player* owner = unit->owner();
 if (!owner) {
	boError() << k_funcinfo << "no owner" << endl;
	return;
 }
 if (!unit->speciesTheme()) {
	boError() << k_funcinfo << "NULL speciestheme" << endl;
	return;
 }

 // don't display production items of units of other players.
 if (owner != d->mOwner) {
	return;
 }
 mSelectedUnit = unit;
}

void BosonCommandFrameBase::hideActions()
{
 d->mUpdateTimer.stop();
 mSelectedUnit = 0;
 d->mOrderWidget->hideOrderButtons();
 QPtrListIterator<BoUnitDisplayBase> it(d->mUnitDisplayWidgets);
 while (it.current()) {
	it.current()->hide();
	++it;
 }

 // not a unit display widget, so we need to hide manually
 d->mOrderWidget->hide();
 showUnitActions(0);
}

void BosonCommandFrameBase::setLocalPlayer(Player* p)
{
 d->mOwner = p;
}

void BosonCommandFrameBase::slotPlaceUnit(ProductionType t, unsigned long int unitType)
{
 if (!d->mOwner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (t != ProduceUnit) {
	boError() << k_funcinfo << "Only ProduceUnit supported" << endl;
	return;
 }
 emit signalPlaceUnit(unitType, d->mOwner);
}

void BosonCommandFrameBase::slotProduce(ProductionType type, unsigned long int id)
{
 if (selectedUnit()) {
	if (d->mOwner != selectedUnit()->owner()) {
		boError() << k_funcinfo << "local owner != selected unit owner" << endl;
		return;
	}
 }
 emit signalProduce(type, id, (UnitBase*)selectedUnit(), d->mOwner);
}

void BosonCommandFrameBase::slotStopProduction(ProductionType type, unsigned long int id)
{
 if (selectedUnit()) {
	if (d->mOwner != selectedUnit()->owner()) {
		boError() << k_funcinfo << "local owner != selected unit owner" << endl;
		return;
	}
 }
 emit signalStopProduction(type, id, (UnitBase*)selectedUnit(), d->mOwner);
}

void BosonCommandFrameBase::slotUpdateProduction(Unit* f)
{
 if (!f) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (selectedUnit() == f) {
	setAction(f);
 }
}

void BosonCommandFrameBase::slotUpdateProductionOptions()
{
 if(selectedUnit()) {
	setAction(selectedUnit());
 }
}

void BosonCommandFrameBase::slotUpdate()
{
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
 d->mOrderWidget->setButtonsPerRow(b);
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
	setMinimumWidth(sizeHint().width());
 }
 QFrame::resizeEvent(e);
}

