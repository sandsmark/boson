/*
    This file is part of the Boson game
    Copyright (C) 2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosontiles.h"
#include "unit.h"
#include "player.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "defines.h"

#include <kgameprogress.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <klocale.h>

#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "bosoncommandwidget.moc"

#define BAR_WIDTH 10 // FIXME hardcoded value


// KProgress has been renamed to KGameProgress after KDE3 beta1. KProgress is
// now useless for us. But For Beta1 it's still ok.
class BoProgress : public KGameProgress
{
	public:
		BoProgress(QWidget* p) : KGameProgress(p) { }

		virtual void setGeometry(int x, int y, int w, int h)
		{
			KPixmap pix(QPixmap(w, h));
			pix.fill(green);
			KPixmapEffect::gradient(pix, green, red, KPixmapEffect::VerticalGradient);
			setBarPixmap(pix);
			KGameProgress::setGeometry(x, y, w, h);
		}
};

class BoButton : public QPushButton
{
public:
	BoButton(QWidget* p) : QPushButton(p)
	{
		mGrayOut = false;
		mProductionCount = 0;
	}

	virtual QSize sizeHint() const
	{
		// there is a *lot* of code in the QPushButton implementation
		// -> hope this is enough for our needs...
		if (!pixmap()) {
			return QSize(0, 0);
		}
		return QSize(pixmap()->width(), pixmap()->height());
	}
	void setGrayOut(bool g)
	{
		mGrayOut = g;
		setPixmap(mPixmap);
	}

	virtual void setPixmap(const QPixmap& pix)
	{
		mPixmap = pix;
		if (mGrayOut) {
			KPixmap p(pix);
			KPixmapEffect::desaturate(p, 1.0);
			if (mProductionCount > 0) {
				addProductionCount(&p);
			}
			QPushButton::setPixmap(p);
		} else {
			QPixmap p(pix);
			if (mProductionCount > 0) {
				addProductionCount(&p);
			}
			QPushButton::setPixmap(p);
		}
	}

	void setProductionCount(int c)
	{
		mProductionCount = c;
		QPixmap p(mPixmap);
		if (mProductionCount > 0) {
			addProductionCount(&p);
		}
		QPushButton::setPixmap(p);
	}
protected:
	virtual void drawButton(QPainter* p)
	{
		// we want to have a *visible* pixmap - not just a gray pixmap!
		bool e = isEnabled();
		clearWState(WState_Disabled);
		drawButtonLabel(p);
		if (!e) {
			setWState(WState_Disabled);
		}
	}

	void addProductionCount(QPixmap* pix)
	{
		QPainter painter(pix);
		painter.setPen(black);
		painter.setBrush(black);
		painter.drawText(5, 5 + painter.fontMetrics().height(), QString::number(mProductionCount));
		painter.end();
	}

private:
	bool mGrayOut;
	int mProductionCount;
	QPixmap mPixmap; // FIXME: this means addiditional memory space for *every* command button!!!
};

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

		mOwner = 0;
	}

	BoButton * mPixmap;

	int mTileNumber;
	int mUnitType;
	Unit* mUnit;

	CommandType mCommandType;

	QHBoxLayout* mTopLayout;

	BoProgress* mHealth;
	BoProgress* mReload;

	Player* mOwner; // kind of a workaround: we need this to display the progress of production
};

BosonCommandWidget::BosonCommandWidget(QWidget* parent) : QWidget(parent)
{
 d = new BosonCommandWidgetPrivate;
 d->mTopLayout = new QHBoxLayout(this);
 d->mTopLayout->setAutoAdd(true);

 QWidget* display = new QWidget(this);
 QHBoxLayout* displayLayout = new QHBoxLayout(display);
 d->mPixmap = new BoButton(display);
 connect(d->mPixmap, SIGNAL(clicked()), this, SLOT(slotClicked()));
 displayLayout->addWidget(d->mPixmap);

 d->mHealth = new BoProgress(display);
 d->mHealth->setOrientation(Vertical);
 d->mHealth->setTextEnabled(false);
 d->mHealth->setFixedWidth(BAR_WIDTH);
 displayLayout->addWidget(d->mHealth);

 d->mReload = new BoProgress(display);
 d->mReload->setOrientation(Vertical);
 d->mReload->setTextEnabled(false);
 d->mReload->setFixedWidth(BAR_WIDTH);
 displayLayout->addWidget(d->mReload);

 d->mHealth->setValue(0);
 d->mReload->setValue(0);
 
 d->mPixmap->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
 setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
 setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
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
	unset();
	hide();
	return;
 }
 d->mUnit = unit;
 d->mCommandType = CommandUnitSelected;
 displayUnitPixmap(unit);
 connect(d->mUnit->owner(), SIGNAL(signalUnitChanged(Unit*)), this,
		 SLOT(slotUnitChanged(Unit*)));
 slotUnitChanged(d->mUnit);

 d->mHealth->show();
 d->mReload->show(); // TODO don't show if unit cannot shoot

 show();
 setProductionCount(0);
 setEnabled(true);
 setGrayOut(false);
}

void BosonCommandWidget::setUnit(int unitType, Player* owner)
{
 if (!owner) {
	kdError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (d->mUnit) {
	unset();
 }
 d->mUnit = 0;
 displayUnitPixmap(unitType, owner);

 const UnitProperties* prop = owner->speciesTheme()->unitProperties(unitType);
 if (!prop) {
	kdError() << k_funcinfo << "No unit properties for " << unitType
			<< endl;
	return;
 }
 setToolTip(i18n("%1\nMinerals: %2\nOil: %3").arg(prop->name()).arg(
			prop->mineralCost()).arg(prop->oilCost()));
 
 d->mUnitType = unitType;
 d->mOwner = owner;
 d->mCommandType = CommandUnit;
 
 d->mHealth->hide();
 d->mReload->hide();

 show();
 // note: setEnabled() and setGrayOut() are handled in BosonCommandFrame for
 // this! (same for setProductionCount())
}

void BosonCommandWidget::setCell(int tileNo, BosonTiles* tileSet)
{
 if (d->mUnit) {
	unset();
 }
 d->mUnit = 0;

 d->mTileNumber = tileNo;
 setPixmap(tileSet->tile(d->mTileNumber));
 d->mCommandType = CommandCell;

 d->mHealth->hide();
 d->mReload->hide();

 show();
 setProductionCount(0);
 setEnabled(true);
 setGrayOut(false);
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

void BosonCommandWidget::setToolTip(const QString& text)
{
 QToolTip::remove(this);
 QToolTip::add(this, text);
}

void BosonCommandWidget::slotClicked()
{
 switch (commandType()) {
	case CommandNothing:
		kdWarning() << "Invalud Command Type \"Nothing\"" << endl;
		break;
	case CommandCell:
		emit signalPlaceCell(tile());
		break;
	case CommandUnit:
		emit signalProduceUnit(unitType());
		break;
	case CommandUnitSelected:
		if (!unit()) {
			kdError() << k_lineinfo << "NULL unit" << endl;
		} else {
			// shall we do something here? maybe make this unit the
			// only selected unit if there are several units
			// selected? or can we omit this signal?
//			emit signalUnit(d->mUnit);
		}
		break;
	default:
		kdError() << "Unknown Command Type " << commandType() << endl;
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

int BosonCommandWidget::tile() const
{
 if (d->mCommandType == CommandCell) {
	return d->mTileNumber;
 } else {
	return -1;
 }
}

int BosonCommandWidget::unitType() const
{
 if (d->mCommandType == CommandUnit) {
	return d->mUnitType;
 } else {
	return -1;
 }
}

Unit* BosonCommandWidget::unit() const
{
 if (d->mCommandType == CommandUnitSelected) {
	return d->mUnit;
 } else {
	return 0;
 }
}

BosonCommandWidget::CommandType BosonCommandWidget::commandType() const
{
 return d->mCommandType;
}

void BosonCommandWidget::unset()
{
 if (d->mUnit) {
	disconnect(d->mUnit->owner(), 0, this, 0);
 }
 d->mUnit = 0;
 d->mCommandType = CommandNothing;
 d->mOwner = 0;
}

void BosonCommandWidget::advanceProduction(double percentage)
{
// kdDebug() << k_funcinfo << percentage << endl;
 if (!d->mOwner) {
	kdError() << k_funcinfo << "NULL owner" << endl;
 }
 QPixmap small = *d->mOwner->speciesTheme()->smallOverview(d->mUnitType);
 if (percentage == 100) {
	setPixmap(small);
	return;
 }

 KPixmap progress(small);
 KPixmapEffect::intensity(progress, 1.4);

 QBitmap mask(progress.width(), progress.height());
 mask.fill(Qt::color1);
 QPainter p;
 p.begin(&mask);
 p.setBrush(Qt::color0);
 
 // this stuff (sizes) is evil and probably not working with other pixmap sizes.
 // I'm too lazy to do it right

 // well, according to the QPoint documentation this is called 
 // "mahattan length" -> it would be correct to use 
 // sqrt(pow(mask.width(),2) + pow(mask.height(), 2)),
 // but this is faster.
 int pieSize = mask.width() + mask.height(); 
 p.drawPie((mask.width()-pieSize)/2, (mask.height() - pieSize)/2, pieSize, 
		pieSize, 16*90, -16*360*(percentage/100));
 p.end();

 progress.setMask(mask);

 p.begin(&small);
 p.drawPixmap(0, 0, progress);
 p.end();

 setPixmap(small);
}

void BosonCommandWidget::setGrayOut(bool g)
{
 d->mPixmap->setGrayOut(g);
}

void BosonCommandWidget::setProductionCount(int count)
{
 d->mPixmap->setProductionCount(count);
}
