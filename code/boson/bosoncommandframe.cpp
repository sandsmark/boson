/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "bosoncommandframe.h"

#include "unit.h"
#include "player.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosonunitview.h"
#include "cell.h"
#include "bosontiles.h"

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
#include <qtooltip.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qmap.h>

#include "defines.h"

#include "bosoncommandframe.moc"

#define ORDERS_PER_COLUMN 3
#define ORDER_SPACING 3

class OrderScrollView : public QScrollView
{
//	Q_OBJECT
public:
	OrderScrollView(QWidget* parent = 0)  : QScrollView(parent) 
	{
		setVScrollBarMode(QScrollView::AlwaysOn);
	}

	~OrderScrollView() {}
	
protected:

};

class BosonCommandFramePrivate
{
public:
	BosonCommandFramePrivate()
	{
		mScrollView = 0;
		mOrderMap = 0;
		mOrderLayout = 0;
		mOrderWidget = 0;

		mUnitView = 0;

		mTiles = 0;

		mTransRef = 0;
		mInverted = 0;

		mOwner = 0;
		mFactory = 0;
	}

	QIntDict<QPushButton> mOrderButton;
	QWidget* mOrderWidget;
	OrderScrollView* mScrollView;
	QSignalMapper* mOrderMap;

	QVBoxLayout* mOrderLayout;
	QPtrList<QHBoxLayout> mHOrderLayoutList;

	BosonUnitView* mUnitView;

	BosonTiles* mTiles;

	QComboBox* mTransRef;
	QCheckBox* mInverted;

	BosonCommandFrame::OrderType mOrderType; // plain tiles, facilities, mob units, ...

	Player* mOwner;
	Unit* mFactory; // the unit that is producing

	QMap<int, int> mOrder2Type; // map order button -> unitType
};

BosonCommandFrame::BosonCommandFrame(QWidget* parent, bool editor) : QFrame(parent)
{
 init();
 if (editor) {
	initEditor();
 }

 QVBoxLayout* layout = new QVBoxLayout(this);

 d->mUnitView = new BosonUnitView(this);
 layout->addWidget(d->mUnitView, 0);

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
// d->mOrderWidget->setMinimumWidth(d->mScrollView->viewport()->width()); // might cause problems if scrollview is resized. maybe subclass QScrollView
 
// d->mOrderLayout = new QVBoxLayout(d->mScrollView->viewport(), ORDER_SPACING, ORDER_SPACING);
 d->mOrderLayout = new QVBoxLayout(d->mOrderWidget, 0, ORDER_SPACING);
 

 d->mOrderMap = new QSignalMapper(d->mOrderWidget);
 connect(d->mOrderMap, SIGNAL(mapped(int)), this, SLOT(slotHandleOrder(int)));

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
 d->mHOrderLayoutList.clear();
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
		QHBoxLayout* h = 0;
		if (d->mHOrderLayoutList.count() > i / ORDERS_PER_COLUMN) {
			h = d->mHOrderLayoutList.at(i / ORDERS_PER_COLUMN);
		} else {
			h = new QHBoxLayout(d->mOrderLayout, ORDER_SPACING);
			d->mHOrderLayoutList.append(h);
		}
		QPushButton* b = new QPushButton(d->mOrderWidget);
		h->addWidget(b);
		b->hide();
		d->mOrderButton.insert(i, b);
		connect(b, SIGNAL(clicked()), d->mOrderMap, SLOT(map()));
		d->mOrderMap->setMapping(b, i);
	}
 }
}


void BosonCommandFrame::slotShowSingleUnit(Unit* unit)
{
 if (!unit) {
	// display nothing
	d->mUnitView->setUnit(0);
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
	kdDebug() << k_funcinfo << ": we are not the owner" << endl;
	return;
 }

 const UnitProperties* prop = unit->unitProperties();
 if (!prop) {
	kdError() << k_funcinfo << ": NULL unitProperties" << endl;
	return;
 }
 if (!prop->canProduce()) {
	return;
 }
 QValueList<int> produceList = prop->produceList();
 setOrderButtons(produceList, owner);
 d->mFactory = unit;
 
 d->mOrderType = Mobiles; // kind of FIXME: it doesn't matter whether this is
                          // Mobiles or Facilities. The difference is only in 
			  // editor.cpp for the KActions.
}

void BosonCommandFrame::hideOrderButtons()
{
 d->mFactory = 0;
 QIntDictIterator<QPushButton> it(d->mOrderButton);
 while (it.current()) {
	it.current()->hide();
	++it;
 }
}

void BosonCommandFrame::setOrderPixmap(unsigned int id, const QPixmap& p)
{
 if (!d->mOrderButton[id]) {
	kdError() << k_funcinfo << "cannot find order button " << id << endl;
	return;
 }
 d->mOrderButton[id]->setPixmap(p);
 d->mOrderButton[id]->setFixedSize(d->mOrderButton[id]->sizeHint());
 d->mOrderButton[id]->show();
}

void BosonCommandFrame::setOrderTooltip(unsigned int id, const QString& text)
{
 if (!d->mOrderButton[id]) {
	kdError() << k_funcinfo << "cannot find order button " << id << endl;
	return;
 }
 QToolTip::remove(d->mOrderButton[id]);
 QToolTip::add(d->mOrderButton[id], text);
}

void BosonCommandFrame::setOrderButtons(QValueList<int> produceList, Player* owner)
{
 initOrderButtons(produceList.count());
 for (unsigned int i = 0; i < produceList.count(); i++) {
	setOrderButton(i, produceList[i], owner);
 }
}

void BosonCommandFrame::setOrderButton(unsigned int button, int unitType, Player* owner)
{
 if (!owner) {
	kdError() << k_funcinfo << ": no owner" << endl;
	return;
 }
 if (!owner->speciesTheme()) {
	kdError() << k_funcinfo << ": player has no species theme" << endl;
	return;
 }

 QPixmap* small = owner->speciesTheme()->smallOverview(unitType);
 if (!small) {
	kdError() << k_funcinfo << ": cannot find small overview for " 
			<< unitType << endl;
	return;
 }
 setOrderPixmap(button, *small);
 d->mOrder2Type.insert(button, unitType);

 const UnitProperties* prop = owner->speciesTheme()->unitProperties(unitType);
 if (!prop) {
	kdError() << k_funcinfo << "No unit properties for " << unitType 
			<< endl;
	return;
 }
 setOrderTooltip(button, prop->name());
}

void BosonCommandFrame::slotHandleOrder(int index)
{
 switch (d->mOrderType) {
	case PlainTiles:
	case Small:
	case Big1:
	case Big2:
		emit signalCellSelected(d->mOrder2Type[index], '0');
		break;
	case Facilities:
	case Mobiles:
		emit signalUnitSelected(d->mOrder2Type[index], d->mFactory, d->mOwner);
		break;
	default:
		kdError() << "unexpected construction index " << index << endl;
		break;
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
			QPixmap p = d->mTiles->plainTile((Cell::GroundType)groundType);
			setOrderPixmap(i, p);
			d->mOrder2Type.insert(i, groundType);
		}
		break;
	case Small:
		hideOrderButtons();
		initOrderButtons(9);
		for (int i = 0; i < 9; i++) {
			int tile = Cell::smallTileNumber(i, trans, inverted);
			setOrderPixmap(i, d->mTiles->tile(tile));
			d->mOrder2Type.insert(i, tile);
		}
		break;
	case Big1:
		hideOrderButtons();
		initOrderButtons(4);
		for (int i = 0; i < 4; i++) {
			QPixmap p = d->mTiles->big1(i, trans, inverted);
			setOrderPixmap(i, p);
			d->mOrder2Type.insert(i, Cell::getBigTransNumber(
					trans, (inverted ? 4 : 0) + i));
			// FIXME: big tiles are currently placed as small ones
			// only
		}
		break;
	case Big2:
		hideOrderButtons();
		initOrderButtons(4);
		for (int i = 0; i < 4; i++) {
			QPixmap p = d->mTiles->big2(i, trans, inverted);
			setOrderPixmap(i, p);
			d->mOrder2Type.insert(i, Cell::getBigTransNumber(
					trans, (inverted ? 12 : 8) + i));
			// FIXME: big tiles are currently placed as small ones
			// only
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
