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

#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qintdict.h>
#include <qsignalmapper.h>
#include <qpixmap.h>
#include <qscrollview.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qmap.h>
#include <qgrid.h>

#include "defines.h"

#include "bosoncommandframe.moc"

#define ORDERS_PER_ROW 3
#define ORDER_SPACING 3

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

		mUnitView = 0;

		mTiles = 0;

		mTransRef = 0;
		mInverted = 0;

		mOwner = 0;
		mFactory = 0;
	}

	QIntDict<BosonCommandWidget> mOrderButton;
	QGrid* mOrderWidget;
	OrderScrollView* mScrollView;


	BosonUnitView* mUnitView;

	BosonTiles* mTiles;

	QComboBox* mTransRef;
	QCheckBox* mInverted;

	BosonCommandFrame::OrderType mOrderType; // plain tiles, facilities, mob units, ...

	Player* mOwner;
	Unit* mFactory; // the unit that is producing

	QMap<int, int> mOrder2Type; // map order button -> unitType
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
 d->mOrderWidget = new QGrid(ORDERS_PER_ROW, d->mScrollView->viewport());
 d->mScrollView->addChild(d->mOrderWidget);
 d->mScrollView->viewport()->setBackgroundMode(backgroundMode());
// d->mOrderWidget->setMinimumWidth(d->mScrollView->viewport()->width()); // might cause problems if scrollview is resized. maybe subclass QScrollView
 
 show();
}

void BosonCommandFrame::init()
{
 d = new BosonCommandFramePrivate;

 setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
 setMinimumSize(230, 200); // FIXME hardcoded value

 setFrameStyle(QFrame::Raised | QFrame::Panel);
 setLineWidth(5);

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
	}
 }
}


void BosonCommandFrame::slotShowSingleUnit(Unit* unit)
{
 if (!unit) {
	// display nothing
	d->mUnitView->setUnit(0);
	for (unsigned int i = 0; i < d->mOrderButton.count(); i++) {
		d->mOrderButton[i]->setUnit(0);
	}
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

void BosonCommandFrame::slotSetConstruction(Unit* unit)
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

 // don't display construction items of units of other players. 
 if (owner != d->mOwner) {
	return;
 }

 const UnitProperties* prop = unit->unitProperties();
 if (!prop->canProduce()) {
	return;
 }
 if (!unit->isFacility()) {
	kdError() << k_lineinfo << "Only facilities can produce" << endl;
	return;
 }
 QValueList<int> produceList = prop->produceList();
 setOrderButtons(produceList, owner, (Facility*)unit);
 d->mFactory = unit;
 connect(d->mFactory->owner(), SIGNAL(signalProductionAdvanced(Unit*, double)),
		this, SLOT(slotProductionAdvanced(Unit*, double)));
 
 d->mOrderType = Mobiles; // kind of FIXME: it doesn't matter whether this is
                          // Mobiles or Facilities. The difference is only in 
                          // editor.cpp for the KActions.
}

void BosonCommandFrame::hideOrderButtons()
{
 if (d->mFactory) {
	disconnect(d->mFactory->owner(), 0, this, 0);
 }
 d->mFactory = 0;
 QIntDictIterator<BosonCommandWidget> it(d->mOrderButton);
 while (it.current()) {
	it.current()->setUnit(0);
	++it;
 }
}

void BosonCommandFrame::setOrderButtons(QValueList<int> produceList, Player* owner, Facility* factory)
{
 initOrderButtons(produceList.count());
 for (unsigned int i = 0; i < produceList.count(); i++) {
	if (factory) {
		int unitType = -1;
		if (factory->hasProduction()) {
			unitType = factory->currentProduction();
		}
		if (unitType > 0) {
			if (produceList[i] != unitType) {
				d->mOrderButton[i]->setGrayOut(true);
			} else {
				d->mOrderButton[i]->setGrayOut(false);
			}
			d->mOrderButton[i]->setEnabled(false);
		} else {
			d->mOrderButton[i]->setEnabled(true);
			d->mOrderButton[i]->setGrayOut(false);
		}
	} else {
		d->mOrderButton[i]->setEnabled(true);
		d->mOrderButton[i]->setGrayOut(false);
	}
	d->mOrderButton[i]->setUnit(produceList[i], owner);
 }
}

void BosonCommandFrame::slotEditorConstruction(int index, Player* owner)
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
	case PlainTiles:
	case Small:
	case Big1:
	case Big2:
		slotRedrawTiles(); // rename: redrawtiles
		break;
	case Mobiles:
		setOrderButtons(theme->allMobiles(), owner);
		break;
	case Facilities:
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
	case PlainTiles:
		hideOrderButtons();
		initOrderButtons(Cell::GroundLast - 1);
		for (int i = 0; i < 5; i++) {
			int groundType = i + 1;
			d->mOrderButton[i]->setCell(groundType, d->mTiles);
		}
		break;
	case Small:
		hideOrderButtons();
		initOrderButtons(9);
		for (int i = 0; i < 9; i++) {
			int tile = Cell::smallTileNumber(i, trans, inverted);
			d->mOrderButton[i]->setCell(tile, d->mTiles);
		}
		break;
	case Big1:
		hideOrderButtons();
		initOrderButtons(4);
		for (int i = 0; i < 4; i++) {
			d->mOrderButton[i]->setCell(Cell::getBigTransNumber(
					trans, (inverted ? 4 : 0) + i), 
					d->mTiles);
		}
		break;
	case Big2:
		hideOrderButtons();
		initOrderButtons(4);
		for (int i = 0; i < 4; i++) {
			d->mOrderButton[i]->setCell(Cell::getBigTransNumber(
					trans, (inverted ? 12 : 8) + i), 
					d->mTiles);
		}
		break;
	case Facilities:
		break;
	case Mobiles:
		break;
	default:
		kdError() << "unexpected construction index " << index << endl;
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

void BosonCommandFrame::slotProductionAdvanced(Unit* factory, double percentage)
{
 if (d->mFactory != factory) {
	return;
 }
 if (!factory->isFacility()) {
	kdError() << k_lineinfo << "NOT factory" << endl;
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
 slotSetConstruction(f);
}

void BosonCommandFrame::slotProductionCompleted(Facility* f)
{
 if (!f) {
	kdError() << k_funcinfo << "NULL facility" << endl;
	return;
 }
 slotSetConstruction(f);
}

