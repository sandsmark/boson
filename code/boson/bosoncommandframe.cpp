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
#include "player.h"
#include "speciestheme.h"
#include "unitproperties.h"
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
#include <qpixmap.h>
#include <qscrollview.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qmap.h>
#include <qtimer.h>

#include "defines.h"

#include "bosoncommandframe.moc"

class OrderScrollView : public QScrollView
{
//	Q_OBJECT
public:
	OrderScrollView(QWidget* parent = 0)  : QScrollView(parent) 
	{
		setVScrollBarMode(QScrollView::AlwaysOn);
	}

	~OrderScrollView() 
	{
	}
	
protected:

};

class BosonCommandFrame::BosonCommandFramePrivate
{
public:
	BosonCommandFramePrivate()
	{
		mScrollView = 0;
		mOrderWidget = 0;
		mOrderLayout = 0;

		mUnitView = 0;

		mTiles = 0;

		mTransRef = 0;
		mInverted = 0;

		mOwner = 0;
		mFactory = 0;
		mFacility = 0; // TODO: merge with mFactory

		mConstructionProgress = 0;
	}

	QIntDict<BosonCommandWidget> mOrderButton;
	QWidget* mOrderWidget;
	QGridLayout* mOrderLayout; 
	OrderScrollView* mScrollView;


	BosonUnitView* mUnitView;

	BosonTiles* mTiles;

	QComboBox* mTransRef;
	QCheckBox* mInverted;

	OrderType mOrderType; // plain tiles, facilities, mob units, ...

	Player* mOwner;
	Unit* mFactory; // the unit that is producing
	Facility* mFacility;

	QMap<int, int> mOrder2Type; // map order button -> unitType

	KGameProgress* mConstructionProgress;

	QTimer mUpdateTimer;
};

BosonCommandFrame::BosonCommandFrame(QWidget* parent, bool editor) : QFrame(parent, "cmd frame")
{
 init();
 if (editor) {
	initEditor();
 }

 QVBoxLayout* layout = new QVBoxLayout(this, 5, 5); // FIXME: hardcoded - maybe use KDialog::marginHint(), KDialog::spacingHint()  -> but might be a problem cause we use setLineWidth(5)

 d->mUnitView = new BosonUnitView(this);
 layout->addWidget(d->mUnitView, 0, AlignHCenter);

 if (d->mTransRef) {
	layout->addWidget(d->mTransRef);
 }
 if (d->mInverted) {
	layout->addWidget(d->mInverted);
 }


// the order buttons
 d->mScrollView = new OrderScrollView(this);
 layout->addWidget(d->mScrollView, 1);
 d->mOrderWidget = new QWidget(d->mScrollView->viewport());
 d->mScrollView->addChild(d->mOrderWidget);
 d->mScrollView->viewport()->setBackgroundMode(backgroundMode());
 d->mConstructionProgress = new KGameProgress(d->mOrderWidget);
 d->mConstructionProgress->hide();
 
 show();
}

void BosonCommandFrame::init()
{
 d = new BosonCommandFramePrivate;

 setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
 setMinimumSize(230, 200); // FIXME hardcoded value

 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdate()));
}

void BosonCommandFrame::initEditor()
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

BosonCommandFrame::~BosonCommandFrame()
{
 if (d->mTiles) {
	delete d->mTiles;
 }
 delete d;
}

void BosonCommandFrame::initOrderButtons(unsigned int no)
{
 if (d->mOrderButton.count() >= no) {
	return;
 }
 for (unsigned int i = 0; i < no; i++) {
	if (!d->mOrderButton[i]) {
		BosonCommandWidget* b = new BosonCommandWidget(d->mOrderWidget);
		b->hide();
		d->mOrderButton.insert(i, b);
		connect(b, SIGNAL(signalPlaceCell(int)), 
				this, SIGNAL(signalCellSelected(int)));
		connect(b, SIGNAL(signalProduceUnit(int)),
				this, SLOT(slotProduceUnit(int)));
		connect(b, SIGNAL(signalStopProduction(int)),
				this, SLOT(slotStopProduction(int)));
	}
 }
 resetLayout();
}

void BosonCommandFrame::resetLayout()
{
 if (d->mOrderLayout) {
	delete d->mOrderLayout;
 }
 int buttons = boConfig->commandButtonsPerRow();
 d->mOrderLayout = new QGridLayout(d->mOrderWidget);
 for (unsigned int i = 0; i < d->mOrderButton.count(); i++) {
	BosonCommandWidget* b = d->mOrderButton[i];
	d->mOrderLayout->addWidget(b, i / buttons, i % buttons);
 }
 int column = (d->mOrderButton.count() - 1) / buttons + 1;
 d->mOrderLayout->addMultiCellWidget(d->mConstructionProgress, column, column, 0, buttons - 1);
 d->mOrderLayout->activate();
}

void BosonCommandFrame::slotShowSingleUnit(Unit* unit)
{
 if (!unit) {
	// display nothing
	d->mUnitView->setUnit(0);
	hideOrderButtons();
	return;
 }
 if (unit->isDestroyed()) {
	kdWarning() << k_funcinfo << ": unit is destroyed" << endl;
	return;
 }
 if (!unit->owner()) {
	kdError() << k_funcinfo << ": unit has no owner" << endl;
	return;
 }
 SpeciesTheme* theme = unit->owner()->speciesTheme();
 if (!theme) {
	kdError() << k_funcinfo << ": owner has no species theme" << endl;
	return;
 }
 QPixmap* p = theme->bigOverview(unit->type());
 if (!p) {
	kdError() << k_funcinfo << ": unit has no big overview in this theme" 
			<< endl;
	return;
 }

 d->mUnitView->setUnit(unit);
}

void BosonCommandFrame::slotSetProduction(Unit* unit)
{
 hideOrderButtons(); // this makes sure that they are even hidden if the unit 
                     // cannot produce
 if (!unit) {
	// display nothing
	return;
 }
 Player* owner = unit->owner();
 if (!owner) {
	kdError() << k_funcinfo << ": no owner" << endl;
	return;
 }

 // don't display production items of units of other players. 
 if (owner != d->mOwner) {
	return;
 }

 if (!unit->isFacility()) {
	return;
 }
 if (!((Facility*)unit)->completedConstruction()) {
	slotShowConstructionProgress((Facility*)unit);
	return;
 }
 const UnitProperties* prop = unit->unitProperties();
 if (!prop->canProduce()) {
	return;
 }
 QValueList<int> produceList = prop->produceList();
 setOrderButtons(produceList, owner, (Facility*)unit);
 d->mFactory = unit;
 
 d->mOrderType = OrderMobiles; // kind of FIXME: it doesn't matter whether this
                          // is OrderMobiles or OrderFacilities. The difference
			  // is only in editor.cpp for the KActions.
 if (((Facility*)unit)->hasProduction()) {
	d->mUpdateTimer.start(500);
 }
}

void BosonCommandFrame::hideOrderButtons()
{
 d->mUpdateTimer.stop();
 d->mConstructionProgress->hide();
 if (d->mFactory) {
	disconnect(d->mFactory->owner(), 0, this, 0);
 }
 d->mFactory = 0;
 d->mFacility = 0;
 QIntDictIterator<BosonCommandWidget> it(d->mOrderButton);
 while (it.current()) {
	it.current()->setUnit(0);
	++it;
 }
}

void BosonCommandFrame::setOrderButtons(QValueList<int> produceList, Player* owner, Facility* factory)
{
 initOrderButtons(produceList.count());
 int unitType = -1;
 if (factory) {
	if (factory->hasProduction()) {
		unitType = factory->currentProduction();
	}
 }
 for (unsigned int i = 0; i < produceList.count(); i++) {
	d->mOrderButton[i]->setUnit(produceList[i], owner);
	if (unitType > 0) {
		int count = factory->productionList().contains(produceList[i]);
		if (produceList[i] != unitType) {
			d->mOrderButton[i]->setProductionCount(count);
			d->mOrderButton[i]->setGrayOut(true);
		} else {
			d->mOrderButton[i]->advanceProduction(factory->productionProgress());
			if (factory->work() != Unit::WorkProduce) {
				d->mOrderButton[i]->setProductionCount(-1);
			} else {
				d->mOrderButton[i]->setProductionCount(count);
			}
			d->mOrderButton[i]->setGrayOut(false);
		}
	} else {
		d->mOrderButton[i]->setProductionCount(0);
		d->mOrderButton[i]->setGrayOut(false);
	}
 }
}

void BosonCommandFrame::slotEditorProduction(int index, Player* owner)
{
 hideOrderButtons();
 if (index == -1) {
	return;
 }
 if (!owner) {
	kdError() << k_funcinfo << ": NULL owner" << endl;
	return;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	kdError() << k_funcinfo << ": NULL theme" << endl;
	return;
 }
 d->mOrderType = (OrderType)index;
 switch (d->mOrderType) {
	case OrderPlainTiles:
	case OrderSmall:
	case OrderBig1:
	case OrderBig2:
		slotRedrawTiles(); // rename: redrawtiles
		break;
	case OrderMobiles:
		setOrderButtons(theme->allMobiles(), owner);
		break;
	case OrderFacilities:
		setOrderButtons(theme->allFacilities(), owner);
		break;
	default:
		kdError() << k_funcinfo << "Invalid index " << index << endl;
		return;
 }
}

void BosonCommandFrame::slotEditorLoadTiles(const QString& fileName)
{
 QString themePath = locate("data", QString("boson/themes/grounds/%1").arg(fileName));
 d->mTiles = new BosonTiles(themePath);
 if (d->mTiles->isNull()) {
	kdError() << k_funcinfo << "Could not load " << fileName << endl;
	return;
 }
}

void BosonCommandFrame::slotRedrawTiles()
{
 bool inverted = d->mInverted->isChecked();
 Cell::TransType trans = (Cell::TransType)d->mTransRef->currentItem();
 // trans is one of TRANS_GW, TRANS_GD, TRANS_DW, TRANS_DWD ans specifies the
 // tile type (desert/water and so on)
 switch (d->mOrderType) {
	case OrderPlainTiles:
		hideOrderButtons();
		initOrderButtons(Cell::GroundLast - 1);
		for (int i = 0; i < 5; i++) {
			int groundType = i + 1;
			d->mOrderButton[i]->setCell(groundType, d->mTiles);
		}
		break;
	case OrderSmall:
		hideOrderButtons();
		initOrderButtons(9);
		for (int i = 0; i < 9; i++) {
			int tile = Cell::smallTileNumber(i, trans, inverted);
			d->mOrderButton[i]->setCell(tile, d->mTiles);
		}
		break;
	case OrderBig1:
		hideOrderButtons();
		initOrderButtons(4);
		for (int i = 0; i < 4; i++) {
			d->mOrderButton[i]->setCell(Cell::getBigTransNumber(
					trans, (inverted ? 4 : 0) + i), 
					d->mTiles);
		}
		break;
	case OrderBig2:
		hideOrderButtons();
		initOrderButtons(4);
		for (int i = 0; i < 4; i++) {
			d->mOrderButton[i]->setCell(Cell::getBigTransNumber(
					trans, (inverted ? 12 : 8) + i), 
					d->mTiles);
		}
		break;
	case OrderFacilities:
		break;
	case OrderMobiles:
		break;
	default:
		kdError() << "unexpected production index " << index << endl;
		break;
 }
}

void BosonCommandFrame::setLocalPlayer(Player* p)
{
 d->mOwner = p;
}

void BosonCommandFrame::slotProduceUnit(int unitType)
{
 emit signalProduceUnit(unitType, d->mFactory, d->mOwner);
}
void BosonCommandFrame::slotStopProduction(int unitType)
{
 emit signalStopProduction(unitType, d->mFactory, d->mOwner);
}

void BosonCommandFrame::slotShowUnit(Unit* unit)
{
 kdDebug() << k_funcinfo << endl;
 for (unsigned int i = 0; i < d->mOrderButton.count(); i++) {
	if (d->mOrderButton[i]->commandType() == BosonCommandWidget::CommandUnitSelected) {
		if (d->mOrderButton[i]->unit() == unit) {
			kdDebug() << "unit already displayed - update..." << endl;
			d->mOrderButton[i]->slotUnitChanged(unit);
			return;
		}
	} else if (d->mOrderButton[i]->commandType() == BosonCommandWidget::CommandNothing) {
		kdDebug() << "show unit at " << i << endl;
		d->mOrderButton[i]->setUnit(unit);
		return;
	}
 }
 initOrderButtons(d->mOrderButton.count() + 1);
 kdDebug() << "display unit" << endl;
 d->mOrderButton[d->mOrderButton.count() - 1]->setUnit(unit);
}

void BosonCommandFrame::productionAdvanced(Unit* factory, double percentage)
{
 if (d->mFactory != factory) {
	return;
 }
 if (!factory->isFacility()) {
	kdError() << k_lineinfo << "NOT factory" << endl;
	return;
 }
 if (!((Facility*)factory)->hasProduction()) {
	kdDebug() << k_funcinfo << "no production" << endl;
	return;
 }
 for (unsigned int i = 0; i < d->mOrderButton.count(); i++) {
	BosonCommandWidget* c = d->mOrderButton[i];
	if (c->commandType() == BosonCommandWidget::CommandUnit) {
		if (c->unitType() == ((Facility*)factory)->currentProduction()) {
			c->advanceProduction(percentage);
		}
	}
 }
}

void BosonCommandFrame::slotFacilityProduces(Facility* f)
{
 if (!f) {
	kdError() << k_funcinfo << "NULL facility" << endl;
	return;
 }
 if (d->mFactory == f) {
	slotSetProduction(f);
 }
}

void BosonCommandFrame::slotProductionCompleted(Facility* f)
{
 if (!f) {
	kdError() << k_funcinfo << "NULL facility" << endl;
	return;
 }
 if (d->mFactory == f) {
	slotSetProduction(f);
 }
}

void BosonCommandFrame::slotSetButtonsPerRow(int b)
{
 boConfig->setCommandButtonsPerRow(b);
 resetLayout();
}

void BosonCommandFrame::slotShowConstructionProgress(Facility* fac)
{
 if (!fac) {
	kdError() << k_funcinfo << "NULL facility" << endl;
	return;
 }
 d->mFacility = fac;
 d->mConstructionProgress->show();
 d->mConstructionProgress->setValue(d->mFacility->constructionProgress());
 d->mUpdateTimer.start(1000);
}

void BosonCommandFrame::slotUpdate()
{
 kdDebug() << k_funcinfo << endl;
 if (d->mFacility) {
	slotShowConstructionProgress(d->mFacility);
	if (d->mFacility->completedConstruction()) {
		slotSetProduction(d->mFacility);
	}
 } else if (d->mFactory) {
	if (!((Facility*)d->mFactory)->hasProduction()) {
		slotProductionCompleted((Facility*)d->mFactory);
	} else {
		productionAdvanced(d->mFactory, ((Facility*)d->mFactory)->productionProgress());
	}
 } else {
	d->mUpdateTimer.stop();
 }
}
