/*
    This file is part of the Boson game
    Copyright (C) 2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "../bosongroundtheme.h"
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
		QWidget* w = (commandWidget()->type() == BosonOrderButton::ShowUnit) ? commandWidget()->childAt(pos) : 0;
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
		switch (commandWidget()->type()) {
			case BosonOrderButton::ShowNothing:
				// do not display anything
				return QString::null;
			case BosonOrderButton::ShowCell:
				//TODO: place something useful here
				text = i18n("Texturenumber: %1").arg(commandWidget()->texture());
				break;
			case BosonOrderButton::ShowUnit:
				if (!commandWidget()->unit()) {
					boWarning(220) << k_funcinfo << "type is ShowUnit, but NULL unit" << endl;
					return QString::null;
				}
				text = i18n("%1\nId: %2").arg(commandWidget()->unit()->unitProperties()->name()).arg(commandWidget()->unit()->id());
				break;
			case BosonOrderButton::ShowAction:
				if (commandWidget()->action().isProduceAction()) {
					if(commandWidget()->action().productionType() == ProduceUnit) {
						const UnitProperties* prop = commandWidget()->productionOwner()->unitProperties(commandWidget()->productionId());
						text = i18n("%1\nMinerals: %2\nOil: %3").arg(prop->name()).arg(prop->mineralCost()).arg(prop->oilCost());
					} else if(commandWidget()->action().productionType() == ProduceTech) {
						const UpgradeProperties* prop = commandWidget()->productionOwner()->speciesTheme()->technology(commandWidget()->productionId());
						text = i18n("%1\nMinerals: %2\nOil: %3").arg(prop->upgradeName()).arg(prop->mineralCost()).arg(prop->oilCost());
					} else {
						boWarning(220) << k_funcinfo << "Invalid productiontype when producing!" << endl;
						return QString::null;
					}
				} else if (commandWidget()->action().type() == ActionPlacementPreview) {
					// Used by editor when placing units
					const UnitProperties* prop = commandWidget()->productionOwner()->unitProperties(commandWidget()->productionId());
					text = i18n("%1\nMinerals: %2\nOil: %3").arg(prop->name()).arg(prop->mineralCost()).arg(prop->oilCost());
				} else {
					text = commandWidget()->action().text();
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
 mTextureNumber = 0;
 mType = ShowNothing;

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
 mType = ShowUnit;
 displayUnitPixmap(unit);
 connect(mUnit->owner(), SIGNAL(signalUnitChanged(Unit*)), this,
		 SLOT(slotUnitChanged(Unit*)));
 slotUnitChanged(mUnit);

 show();
 mHealth->show();

 setProductionCount(0);
 setGrayOut(false);
}

void BosonOrderButton::setAction(const BoSpecificAction& action)
{
 boDebug(220) << k_funcinfo << "Setting action" << endl;
 mType = ShowAction;
 mAction = action;

 if (!action.pixmap()) {
	boError(220) << k_funcinfo << "NULL pixmap for action " << action.id() << endl;
	return;
 }
 setPixmap(*action.pixmap());

 mHealth->hide();

 show();
}

void BosonOrderButton::setGround(unsigned int texture, BosonGroundTheme* theme)
{
 if (mUnit) {
	unset();
 }
 mUnit = 0;
 BO_CHECK_NULL_RET(theme);

 mTextureNumber = texture;
 mType = ShowCell;
 setPixmap(theme->pixmap(mTextureNumber));

 mHealth->hide();

 show();
 setProductionCount(0);
 setGrayOut(false);
}

void BosonOrderButton::displayUnitPixmap(Unit* unit)
{
 if (!unit) {
	boError(220) << k_funcinfo << "NULL unit" << endl;
	return;
 }
 displayUnitPixmap(unit->type(), unit->owner());
}

void BosonOrderButton::displayUnitPixmap(unsigned long int unitType, const Player* owner)
{
 if (!owner) {
	boError(220) << k_funcinfo << "NULL owner" << endl;
	return;
 }
 QPixmap* small = owner->speciesTheme()->smallOverview(unitType);
 if (!small) {
	boError(220) << k_funcinfo << "Cannot find small overview for "
			<< unitType << endl;
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
 switch (type()) {
	case ShowNothing:
		boWarning(220) << "Invalid type \"ShowNothing\"" << endl;
		break;
	case ShowCell:
		emit signalPlaceGround((unsigned int)texture());
		break;
	case ShowUnit:
		if (!unit()) {
			boError(220) << k_lineinfo << "NULL unit" << endl;
		} else {
			// select this unit only, i.e. unselect all others
			emit signalSelectUnit(unit());
		}
		break;
	case ShowAction:
		emit signalAction(mAction);
		break;
	default:
		boError(220) << "Unknown type " << type() << endl;
		break;
 }
}

void BosonOrderButton::slotRightClicked()
{
 if (type() == ShowAction && mAction.isProduceAction()) {
	BoSpecificAction a = mAction;
	if (mAction.type() == ActionProduceUnit) {
		a.setType(ActionStopProduceUnit);
	} else if (mAction.type() == ActionProduceTech) {
		a.setType(ActionStopProduceTech);
	} else {
		boError(220) << k_funcinfo << "Produce action, but invalid actionType: " << mAction.type() << endl;
		return;
	}
	emit signalAction(a);
 }
}

void BosonOrderButton::slotUnitChanged(Unit* unit)
{
 if (unit != mUnit) {
	return;
 }
 if (!unit) {
	boError(220) << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (unit->isDestroyed()) {
	setUnit(0);
	return;
 }
 double h = (double)unit->health() * 100 / (double)unit->unitProperties()->health();
 if (unit->health() > unit->unitProperties()->health()) {
	boWarning(220) << k_lineinfo << "health > possible health" << endl;
 }
 mHealth->setValue((int)h);
}


void BosonOrderButton::unset()
{
 if (mUnit) {
	disconnect(mUnit->owner(), 0, this, 0);
 }
 mUnit = 0;
 mTextureNumber = 0;
 mAction.reset();
 mType = ShowNothing;
}

void BosonOrderButton::advanceProduction(double percentage)
{
 if (!mAction.ok()) {
	boError(220) << k_funcinfo << "No action set" << endl;
	return;
 }
 if (!mAction.unit()->owner()) {
	boError(220) << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (mAction.productionId() <= 0) {
	boError(220) << k_funcinfo << "invalid production id: " << mAction.productionId() << endl;
	return;
 }
 QPixmap* pix = mAction.pixmap();
 if (!pix) {
	boError(220) << k_funcinfo << "NULL pixmap for action " << mAction.id() << endl;
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

unsigned long int BosonOrderButton::productionId() const
{
 return (type() == ShowAction) ? mAction.productionId() : 0;
}

ProductionType BosonOrderButton::productionType() const
{
 return (type() == ShowAction) ? mAction.productionType() : ProduceNothing;
}

Player* BosonOrderButton::productionOwner() const
{
 return (type() == ShowAction) ? mAction.productionOwner() : 0;
}

