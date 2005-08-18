/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "../../bomemory/bodummymemory.h"
#include "../bosongroundtheme.h"
#include "../bosongroundthemedata.h"
#include "../unit.h"
#include "../player.h"
#include "../speciestheme.h"
#include "../speciesdata.h"
#include "../unitproperties.h"
#include "../defines.h"
#include "../upgradeproperties.h"
#include "bodebug.h"
#include "../boufo/boufoimage.h"
#include "../boufo/boufoprogress.h"
#include "../bosonviewdata.h"

#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kimageeffect.h>
#include <klocale.h>

#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>

#define BAR_WIDTH 10 // FIXME hardcoded value


void BoOrderButtonButton::setGrayOut(bool g)
{
 mGrayOut = g;
 if (mImage.isNull()) {
	return;
 }
 setImage(mImage);
}

void BoOrderButtonButton::setImage(const QImage& img)
{
 if (img.isNull()) {
	boError() << k_funcinfo << "NULL image" << endl;
	return;
 }
 mImage = img;
 if (mGrayOut) {
	QImage img = mImage.copy();
	KImageEffect::desaturate(img, 1.0);
	if (mProductionCount != 0) {
		addProductionCount(&img);
	}
	BoUfoPushButton::setIcon(img);
 } else {
	QImage img = mImage.copy();
	if (mProductionCount != 0) {
		addProductionCount(&img);
	}
	BoUfoPushButton::setIcon(img);
 }
}

void BoOrderButtonButton::setProductionCount(int c)
{
 mProductionCount = c;
 QImage img = mImage.copy();
 if (mProductionCount != 0) {
	addProductionCount(&img);
 }
 BoUfoPushButton::setIcon(img);
}

void BoOrderButtonButton::slotMouseReleaseEvent(QMouseEvent* e)
{
 if (e->button() == Qt::RightButton) {
	emit rightClicked();
	e->accept();
 }
}

void BoOrderButtonButton::addProductionCount(QImage* img)
{
 BO_CHECK_NULL_RET(img);
 if (img->isNull()) {
	return;
 }
 QColor color(green);
 QFont f;
 f.setBold(true);
 QPixmap pix(*img);
 QPainter painter(&pix);
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

 *img = pix.convertToImage();
}


class BosonOrderButtonPrivate
{
public:
	BosonOrderButtonPrivate()
	{
	}
};

BosonOrderButton::BosonOrderButton() : BoUfoWidget()
{
 d = new BosonOrderButtonPrivate;
 setName("BosonOrderButton");
 mUnit = 0;
 mGroundType = 0;
 mType = ShowNothing;

 setLayoutClass(UHBoxLayout);
 setOpaque(false);

 BoUfoWidget* display = new BoUfoWidget();
 addWidget(display);
 display->setLayoutClass(UHBoxLayout);

 mPixmapButton = new BoOrderButtonButton();
 mPixmapButton->setOpaque(false);
 display->addWidget(mPixmapButton);
 mPixmapButton->setMouseEventsEnabled(true, true);
 connect(mPixmapButton, SIGNAL(signalClicked()), this, SLOT(slotClicked()));
 connect(mPixmapButton, SIGNAL(rightClicked()), this, SLOT(slotRightClicked()));
 connect(mPixmapButton, SIGNAL(signalMouseEntered(ufo::UMouseEvent*)), this, SIGNAL(signalMouseEntered()));
 connect(mPixmapButton, SIGNAL(signalMouseExited(ufo::UMouseEvent*)), this, SIGNAL(signalMouseLeft()));
// mPixmapButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

 mHealth = new BoUfoProgress();
 mHealth->setRange(0.0, 1.0);
 display->addWidget(mHealth);
 mHealth->setOrientation(Vertical);
#if 0
 mHealth->setTextEnabled(false);
 mHealth->setFixedWidth(BAR_WIDTH);
#endif

 mHealth->setValue(0);

}

BosonOrderButton::~BosonOrderButton()
{
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

 if (!action.image()) {
	boError(220) << k_funcinfo << "NULL pixmap for action " << action.id() << endl;
	return;
 }
 setImage(*action.image());

 mHealth->hide();

 show();
}

void BosonOrderButton::setGround(unsigned int groundType, const BosonGroundTheme* theme)
{
 if (mUnit) {
	unset();
 }
 mUnit = 0;
 BO_CHECK_NULL_RET(theme);
 BO_CHECK_NULL_RET(boViewData);

 mGroundType = groundType;
 mType = ShowGround;

 QPixmap pixmap;
 BosonGroundThemeData* data = boViewData->groundThemeData(theme);
 if (!data) {
	BO_NULL_ERROR(boViewData->groundThemeData(theme));
	// AB: do NOT return - use a dummy pixmap
 }
 BosonGroundTypeData* typeData = 0;
 if (data) {
	typeData = data->groundTypeData(groundType);
	if (!typeData) {
		BO_NULL_ERROR(typeData);
		// AB: do NOT return - use a dummy pixmap
	}
 }
 if (typeData) {
	pixmap = *typeData->icon;
 } else {
	// dummy pixmap
	pixmap = QPixmap(50, 50);
	pixmap.fill(Qt::red);
 }
 setImage(pixmap.convertToImage());

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
 QImage* small = boViewData->speciesData(owner->speciesTheme())->smallOverview(unitType, owner->teamColor());
 if (!small) {
	boError(220) << k_funcinfo << "Cannot find small overview for "
			<< unitType << endl;
	return;
 }
 setImage(*small);
}

void BosonOrderButton::setImage(const QImage& image)
{
 if (image.isNull()) {
	boError() << k_funcinfo << "NULL image" << endl;
	return;
 }
 mPixmapButton->setImage(image);
 mPixmapButton->show();
}

void BosonOrderButton::slotClicked()
{
 switch (type()) {
	case ShowNothing:
		boWarning(220) << "Invalid type \"ShowNothing\"" << endl;
		break;
	case ShowGround:
		emit signalPlaceGround(groundType());
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
 const double epsilon = 0.0001;
 double h = (double)unit->health() / (double)unit->maxHealth();
 if (h > 1.0 + epsilon) {
	boWarning(220) << k_lineinfo << "health > possible health" << endl;
 }
 mHealth->setValue(h);
}


void BosonOrderButton::unset()
{
 if (mUnit) {
	disconnect(mUnit->owner(), 0, this, 0);
 }
 mUnit = 0;
 mGroundType = 0;
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
 QImage* image = mAction.image();
 if (!image) {
	boError(220) << k_funcinfo << "NULL image for action " << mAction.id() << endl;
	return;
 }
 if (percentage == 100) {
	setImage(*image);
	return;
 }
 QPixmap small;
 small.convertFromImage(*image);

 KPixmap progress(small);
 progress.setMask(QBitmap());

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

 setImage(small.convertToImage());
}

void BosonOrderButton::setGrayOut(bool g)
{
 mPixmapButton->setGrayOut(g);
}

void BosonOrderButton::setProductionCount(int count)
{
 mPixmapButton->setProductionCount(count);
}

unsigned long int BosonOrderButton::productionId() const
{
 return (type() == ShowAction) ? mAction.productionId() : 0;
}

ProductionType BosonOrderButton::productionType() const
{
 return (type() == ShowAction) ? mAction.productionType() : ProduceNothing;
}

PlayerIO* BosonOrderButton::productionOwner() const
{
 return (type() == ShowAction) ? mAction.productionOwner() : 0;
}

