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

#include "bosonorderbutton.h"
#include "bosonorderbutton.moc"

#include "../bosontiles.h"
#include "../unit.h"
#include "../player.h"
#include "../speciestheme.h"
#include "../unitproperties.h"
#include "../defines.h"
#include "../upgradeproperties.h"
#include "bodebug.h"

#include <kgameprogress.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <klocale.h>

#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qpushbutton.h>

#define BAR_WIDTH 10 // FIXME hardcoded value

class BoToolTip : public QToolTip
{
public:
	BoToolTip(BosonOrderButton* parent) : QToolTip(parent)
	{
	}

	inline BosonOrderButton* commandWidget() const
	{
		return (BosonOrderButton*)parentWidget();
	}

protected:
	virtual void maybeTip(const QPoint& pos)
	{
		//TODO: do not re-display if already displayed and text didn't
		//change
		QString text;
		QWidget* w = (commandWidget()->orderType() == OrderUnitSelected) ? commandWidget()->childAt(pos) : 0;
		if (w == (QWidget*)commandWidget()->mHealth && commandWidget()->unit()) {
			Unit* u = commandWidget()->unit();
			text = i18n("%1\nId: %2\nHealth: %3\n").arg(u->unitProperties()->name()).arg(u->id()).arg(u->health());
		} else {
			text = mainTip();
		}

		if (text.isNull()) {
			return;
		}
		tip(QRect(commandWidget()->rect()), text);
	}

	/**
	 * Tip for the widget if there is no other tip available. Some parts of
	 * the widget (e.g. the progress bars) can provide another tip that
	 * overrides this one.
	 **/
	QString mainTip() const
	{
		QString text;
		switch (commandWidget()->orderType()) {
			case OrderNothing:
				// do not display anything
				return QString::null;
			case OrderCell:
				//TODO: place something useful here
				text = i18n("Tilenumber: %1").arg(commandWidget()->tile());
				break;
			case OrderProduce:
			{
				if(commandWidget()->productionType() == ProduceUnit) {
					const UnitProperties* prop = commandWidget()->productionOwner()->unitProperties(commandWidget()->productionId());
					text = i18n("%1\nMinerals: %2\nOil: %3").arg(prop->name()).arg(prop->mineralCost()).arg(prop->oilCost());
				} else if(commandWidget()->productionType() == ProduceTech) {
					TechnologyProperties* prop = commandWidget()->productionOwner()->speciesTheme()->technology(commandWidget()->productionId());
					text = i18n("%1\nMinerals: %2\nOil: %3").arg(prop->upgradeName()).arg(prop->mineralCost()).arg(prop->oilCost());
				} else {
					boWarning() << k_funcinfo << "Invalid productiontype when producing!" << endl;
					return QString::null;
				}
				break;
			}
			case OrderUnitSelected:
				if (!commandWidget()->unit()) {
					boWarning() << k_funcinfo << "OrderUnitSelected, but NULL unit" << endl;
					return QString::null;
				}
				text = i18n("%1\nId: %2").arg(commandWidget()->unit()->unitProperties()->name()).arg(commandWidget()->unit()->id());
				break;
			case OrderAction:
				if(commandWidget()->action() == ActionMove) {
					text = i18n("Move");
				}
				else if(commandWidget()->action() == ActionAttack) {
					text = i18n("Attack");
				}
				else if(commandWidget()->action() == ActionStop) {
					text = i18n("Stop");
				}
				else if(commandWidget()->action() == ActionFollow) {
					text = i18n("Follow");
				}
				else if(commandWidget()->action() == ActionMine) {
					text = i18n("Mine");
				}
				else if(commandWidget()->action() == ActionRepair) {
					text = i18n("Repair");
				}
				break;
		}
		return text;
	}
};

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


QSize BoButton::sizeHint() const
{
 // there is a *lot* of code in the QPushButton implementation
 // -> hope this is enough for our needs...
 if (!pixmap()) {
	return QSize(0, 0);
 }
 return QSize(pixmap()->width(), pixmap()->height());
}

void BoButton::setGrayOut(bool g)
{
 mGrayOut = g;
 setPixmap(mPixmap);
}

void BoButton::setPixmap(const QPixmap& pix)
{
 mPixmap = pix;
 if (mGrayOut) {
	KPixmap p(pix);
	KPixmapEffect::desaturate(p, 1.0);
	if (mProductionCount != 0) {
		addProductionCount(&p);
	}
	QPushButton::setPixmap(p);
 } else {
	QPixmap p(pix);
	if (mProductionCount != 0) {
		addProductionCount(&p);
	}
	QPushButton::setPixmap(p);
 }
}

void BoButton::setProductionCount(int c)
{
 mProductionCount = c;
 QPixmap p(mPixmap);
 if (mProductionCount != 0) {
	addProductionCount(&p);
 }
 QPushButton::setPixmap(p);
}

void BoButton::drawButton(QPainter* p)
{
 // we want to have a *visible* pixmap - not just a gray pixmap!
 bool e = isEnabled();
 clearWState(WState_Disabled);
 drawButtonLabel(p);
 if (!e) {
	setWState(WState_Disabled);
 }
}

void BoButton::mouseReleaseEvent(QMouseEvent* e)
{
 QPushButton::mouseReleaseEvent(e);
 if (e->isAccepted()) {
	return;
 }
 if (e->button() == RightButton) {
	if (hitButton(e->pos())) {
		emit rightClicked();
	}
 }
}

void BoButton::addProductionCount(QPixmap* pix)
{
 QColor color(green);
 QFont f;
 f.setBold(true);
 QPainter painter(pix);
 painter.setPen(color);
 painter.setFont(f);
 
 if (mProductionCount > 0) {
	painter.drawText(5, 5 + painter.fontMetrics().height(),
			QString::number(mProductionCount));
 } else if (mProductionCount == -1) {
	// TODO: if the i18n()'ed text does not fit into the widget change the
	// font size!
	painter.drawText(5, 5 + painter.fontMetrics().height(), i18n("Pause"));
 }
 painter.end();
}


class BosonOrderButton::BosonOrderButtonPrivate
{
public:
	BosonOrderButtonPrivate()
	{
		mPixmap = 0;

		mTip = 0;
	}

	BoButton * mPixmap;

	BoToolTip* mTip;
};

BosonOrderButton::BosonOrderButton(QWidget* parent) : QWidget(parent)
{
 d = new BosonOrderButtonPrivate;
 mUnit = 0;
 mProductionOwner = 0;
 mProductionId = 0;
 mProductionType = ProduceNothing;
 mTileNumber = 0;
 mAction = -1;
 mOrderType = OrderNothing;

 QHBoxLayout* topLayout = new QHBoxLayout(this);
 topLayout->setAutoAdd(true);

 QWidget* display = new QWidget(this);
 QHBoxLayout* displayLayout = new QHBoxLayout(display);
 mPixmap = new BoButton(display);
 connect(mPixmap, SIGNAL(clicked()), this, SLOT(slotClicked()));
 connect(mPixmap, SIGNAL(rightClicked()), this, SLOT(slotRightClicked()));
 displayLayout->addWidget(mPixmap);

 mHealth = new BoProgress(display);
 mHealth->setOrientation(Vertical);
 mHealth->setTextEnabled(false);
 mHealth->setFixedWidth(BAR_WIDTH);
 displayLayout->addWidget(mHealth);

 d->mTip = new BoToolTip(this);

 mHealth->setValue(0);
 
 mPixmap->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
 setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
 setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
}

BosonOrderButton::~BosonOrderButton()
{
 delete d->mTip;
 delete mPixmap;
 delete d;
}

void BosonOrderButton::setUnit(Unit* unit)
{
 if (!unit) {
	unset();
	hide();
	return;
 }
 mUnit = unit;
 mOrderType = OrderUnitSelected;
 displayUnitPixmap(unit);
 connect(mUnit->owner(), SIGNAL(signalUnitChanged(Unit*)), this,
		 SLOT(slotUnitChanged(Unit*)));
 slotUnitChanged(mUnit);

 show();
 mHealth->show();

 setProductionCount(0);
 setGrayOut(false);
}

void BosonOrderButton::setProduction(ProductionType type, unsigned long int id, Player* owner)
{
 if (!owner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (mUnit) {
	unset();
 }
 mUnit = 0;
 mProductionType = type;
 mProductionId = id;
 mProductionOwner = owner;
 mOrderType = OrderProduce;

 if(type == ProduceUnit) {
	displayUnitPixmap(id, owner);
 } else {
	displayTechPixmap(id, owner);
 }

 mHealth->hide();

 show();
 // note: setGrayOut() and setProductionCount() are handled in 
 // BosonCommandFrame for this!
}

void BosonOrderButton::setAction(UnitAction action, Player* owner)
{
 mOrderType = OrderAction;
 mAction = (int)action;

 if (!owner->speciesTheme()->actionPixmap(action)) {
	boError() << k_funcinfo << "NULL pixmap for action " << action << endl;
	return;
 }
 setPixmap(*owner->speciesTheme()->actionPixmap(action));

 mHealth->hide();

 show();
}

void BosonOrderButton::setCell(int tileNo, BosonTiles* tileSet)
{
 if (mUnit) {
	unset();
 }
 if (!tileSet) {
	boError() << k_funcinfo << "NULL tileset" << endl;
	return;
 }
 mUnit = 0;

 mTileNumber = tileNo;
 mOrderType = OrderCell;
 setPixmap(tileSet->tile(mTileNumber));

 mHealth->hide();

 show();
 setProductionCount(0);
 setGrayOut(false);
}

void BosonOrderButton::displayUnitPixmap(Unit* unit) 
{
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 displayUnitPixmap(unit->type(), unit->owner());
}

void BosonOrderButton::displayUnitPixmap(unsigned long int unitType, Player* owner)
{
 if (!owner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 QPixmap* small = owner->speciesTheme()->smallOverview(unitType);
 if (!small) {
	boError() << k_funcinfo << "Cannot find small overview for "
			<< unitType << endl;
	return;
 }
 setPixmap(*small);
}

void BosonOrderButton::displayTechPixmap(unsigned long int id, Player* owner)
{
 if (!owner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 QPixmap* small = owner->speciesTheme()->techPixmap(id);
 if (!small) {
	boError() << k_funcinfo << "Cannot find pixmap for  " << id << endl;
	return;
 }
 setPixmap(*small);
}

void BosonOrderButton::setPixmap(const QPixmap& pixmap)
{
 mPixmap->setPixmap(pixmap);
 mPixmap->show();
}

void BosonOrderButton::slotClicked()
{
 switch (orderType()) {
	case OrderNothing:
		boWarning() << "Invalud order Type \"Nothing\"" << endl;
		break;
	case OrderCell:
		emit signalPlaceCell(tile());
		break;
	case OrderProduce:
		emit signalProduce(productionType(), productionId());
		break;
	case OrderUnitSelected:
		if (!unit()) {
			boError() << k_lineinfo << "NULL unit" << endl;
		} else {
			// select this unit only, i.e. unselect all others
			emit signalSelectUnit(unit());
		}
		break;
	case OrderAction:
		emit signalAction(mAction);
		break;
	default:
		boError() << "Unknown order Type " << orderType() << endl;
		break;
 }
}

void BosonOrderButton::slotRightClicked()
{
 switch (orderType()) {
	case OrderProduce:
		emit signalStopProduction(productionType(), productionId());
		break;
	default:
		break;
 }
}

void BosonOrderButton::slotUnitChanged(Unit* unit)
{
 if (unit != mUnit) {
	return;
 }
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (unit->isDestroyed()) {
	setUnit(0);
	return;
 }
 double h = (double)unit->health() * 100 / (double)unit->unitProperties()->health();
 if (unit->health() > unit->unitProperties()->health()) {
	boWarning() << k_lineinfo << "health > possible health" << endl;
 }
 mHealth->setValue((int)h);
}


void BosonOrderButton::unset()
{
 if (mUnit) {
	disconnect(mUnit->owner(), 0, this, 0);
 }
 mUnit = 0;
 mProductionId = 0;
 mProductionType = ProduceNothing;
 mTileNumber = 0;
 mAction = -1;
 mOrderType = OrderNothing;
 mProductionOwner = 0;
}

void BosonOrderButton::advanceProduction(double percentage)
{
 if (!mProductionOwner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (mProductionId <= 0) {
	boError() << k_funcinfo << "production id: " << mProductionId << endl;
	return;
 }
 QPixmap* pix;
 if(mProductionType == ProduceUnit) {
	pix = mProductionOwner->speciesTheme()->smallOverview(mProductionId);
 } else {
	pix = mProductionOwner->speciesTheme()->techPixmap(mProductionId);
 }
 if (!pix) {
	boError() << k_funcinfo << "NULL pixmap for " << mProductionId << endl;
	return;
 }
 QPixmap small(*pix);
 if (percentage == 100) {
	setPixmap(small);
	return;
 }

 KPixmap progress(small);
 progress.setMask(QBitmap()); // something strange is going on... why is this necessary for pixmaps with alpha channel? we already deleted the alpha pixmal in SpeciesTheme...
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
 p.drawPie((mask.width() - pieSize)/2, (mask.height() - pieSize)/2, pieSize,
		pieSize, 16*90, (int)(-16*360*(percentage/100)));
 p.end();

 progress.setMask(mask);

 p.begin(&small);
 p.drawPixmap(0, 0, progress);
 p.end();

 setPixmap(small);
}

void BosonOrderButton::setGrayOut(bool g)
{
 mPixmap->setGrayOut(g);
}

void BosonOrderButton::setProductionCount(int count)
{
 mPixmap->setProductionCount(count);
}

