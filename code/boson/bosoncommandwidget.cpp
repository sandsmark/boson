/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosoncommandwidget.h"

#include <kprogress.h>

#include <qpixmap.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "bosontiles.h"
#include "unit.h"
#include "player.h"
#include "speciestheme.h"
#include "unitproperties.h"

#include "bosoncommandwidget.moc"

#define BAR_WIDTH 10 // FIXME hardcoded value

class BosonCommandWidget::BosonCommandWidgetPrivate
{
public:
	BosonCommandWidgetPrivate()
	{
		mPixmap = 0;

		mTileNumber = -1;
		mUnitType = -1;
		mUnit = 0;

		mCommandType = CommandNothing;

		mTopLayout = 0;

		mHealth = 0;
		mReload = 0;
	}

	QPushButton* mPixmap;

	int mTileNumber;
	int mUnitType;
	Unit* mUnit;

	CommandType mCommandType;

	QVBoxLayout* mTopLayout;

	KProgress* mHealth; // problem: we want a gradient here...
	KProgress* mReload;
};

BosonCommandWidget::BosonCommandWidget(QWidget* parent) : QWidget(parent)
{
 d = new BosonCommandWidgetPrivate;
 d->mTopLayout = new QVBoxLayout(this);
 d->mTopLayout->setAutoAdd(true);

 QWidget* display = new QWidget(this);
 QHBoxLayout* displayLayout = new QHBoxLayout(display);
 d->mPixmap = new QPushButton(display, "Overview");
 connect(d->mPixmap, SIGNAL(clicked()), this, SLOT(slotClicked()));
 displayLayout->addWidget(d->mPixmap);

 d->mHealth = new KProgress(display);
// mHealth->setBarStyle(KProgress::Solid);
 d->mHealth->setOrientation(Vertical);
 d->mHealth->setTextEnabled(false);
 d->mHealth->setFixedWidth(BAR_WIDTH);
// mHealth->setBarColor(QColor::Green);
 d->mHealth->setBarColor(green);
 displayLayout->addWidget(d->mHealth);

 d->mReload = new KProgress(display);
 d->mReload->setOrientation(Vertical);
 d->mReload->setTextEnabled(false);
 d->mReload->setFixedWidth(BAR_WIDTH);
 d->mReload->setBarColor(green);
 displayLayout->addWidget(d->mReload);

 d->mHealth->setValue(0);
 d->mReload->setValue(0);

 
}

BosonCommandWidget::~BosonCommandWidget()
{
 if (d->mPixmap) {
	delete d->mPixmap;
 }
 delete d;
}

void BosonCommandWidget::setUnit(Unit* unit)
{
 if (!unit) {
	if (d->mUnit) {
		disconnect(d->mUnit->owner(), 0, this, 0);
	}
	d->mUnit = 0;
	d->mCommandType = CommandNothing;
	hide();
	return;
 }
 d->mUnit = unit;
 d->mCommandType = CommandUnitSelected;
 displayUnitPixmap(unit);
 connect(d->mUnit->owner(), SIGNAL(signalUnitChanged(Unit*)), this,
		 SLOT(slotUnitChanged(Unit*)));
 slotUnitChanged(d->mUnit);

 // TODO: connect to changes on that unit! maybe 
 // connec(unit->owner(), SIGNAL(signalUnitPropertyChanged()), this,
 // SLOT(slotUpdateSelectedUnit()));
 // or so



 show();
}

void BosonCommandWidget::setUnit(int unitType, Player* owner)
{
 if (!owner) {
	kdError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (d->mUnit) {
	disconnect(d->mUnit->owner(), 0, this, 0);
 }
 d->mUnit = 0;
 displayUnitPixmap(unitType, owner);

 const UnitProperties* prop = owner->speciesTheme()->unitProperties(unitType);
 if (!prop) {
	kdError() << k_funcinfo << "No unit properties for " << unitType
			<< endl;
	return;
 }
 setToolTip(prop->name());
 
 d->mUnitType = unitType;
 d->mCommandType = CommandUnit;
 show();
}

void BosonCommandWidget::setCell(int tileNo, BosonTiles* tileSet)
{
 if (d->mUnit) {
	disconnect(d->mUnit->owner(), 0, this, 0);
 }
 d->mUnit = 0;

 d->mTileNumber = tileNo;
 setPixmap(tileSet->tile(d->mTileNumber));
 d->mCommandType = CommandCell;
 show();
}

void BosonCommandWidget::displayUnitPixmap(Unit* unit) 
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 displayUnitPixmap(unit->type(), unit->owner());
}

void BosonCommandWidget::displayUnitPixmap(int unitType, Player* owner)
{
 if (!owner) {
	kdError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 QPixmap* small = owner->speciesTheme()->smallOverview(unitType);
 if (!small) {
	kdError() << k_funcinfo << "Cannot find small overview for "
			<< unitType << endl;
	return;
 }
 setPixmap(*small);
}

void BosonCommandWidget::setPixmap(const QPixmap& pixmap)
{
 d->mPixmap->setPixmap(pixmap);
 d->mPixmap->show();
}

void BosonCommandWidget::mousePressEvent(QMouseEvent* e)
{ // obsolete
 // a click on this widget is always a "hit" (see QButton::hitButton()).
// slotClicked();
}

void BosonCommandWidget::setToolTip(const QString& text)
{
 QToolTip::remove(this);
 QToolTip::add(this, text);
}

void BosonCommandWidget::slotClicked()
{
 switch (d->mCommandType) {
	case CommandNothing:
		kdWarning() << "Invalud Command Type \"Nothing\"" << endl;
		break;
	case CommandCell:
		emit signalPlaceCell(d->mTileNumber);
		break;
	case CommandUnit:
		emit signalProduceUnit(d->mUnitType);
		break;
	case CommandUnitSelected:
		if (!d->mUnit) {
			kdError() << k_lineinfo << "NULL unit" << endl;
		} else {
			// shall we do something here? maybe make this unit the
			// only selected unit if there are several units
			// selected? or can we omit this signal?
//			emit signalUnit(d->mUnit);
		}
		break;
	default:
		kdError() << "Unknown Command Type " << d->mCommandType << endl;
		break;
 }
}

void BosonCommandWidget::slotUnitChanged(Unit* unit)
{
 if (unit != d->mUnit) {
	return;
 }
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (unit->isDestroyed()) {
	setUnit(0);
	return;
 }
 double h = (double)unit->health() * 100 / (double)unit->unitProperties()->health();
 if (unit->health() > unit->unitProperties()->health()) {
	kdWarning() << k_lineinfo << "health > possible health" << endl;
 }
 d->mHealth->setValue(h);

 double r = 100 - ((double)unit->reloadState() * 100 / (double)unit->unitProperties()->reload());
 d->mReload->setValue(r);
}

