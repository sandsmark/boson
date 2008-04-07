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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosonorderbutton.h"
#include "bosonorderbutton.moc"

#include "../../../bomemory/bodummymemory.h"
#include "../../gameengine/bosongroundtheme.h"
#include "../../gameengine/unit.h"
#include "../../gameengine/player.h"
#include "../../gameengine/speciestheme.h"
#include "../../gameengine/unitproperties.h"
#include "../../gameengine/upgradeproperties.h"
#include "../../boufo/boufoimage.h"
#include "../../boufo/boufoprogress.h"
#include "../../boufo/boufodrawable.h"
#include "../../boufo/boufostandalonefont.h"
#include "../../boufo/boufomanager.h"
#include "../../defines.h"
#include "../../speciesdata.h"
#include "../../bosonviewdata.h"
#include "../../bosongroundthemedata.h"
#include "bodebug.h"

#include <klocale.h>

#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qimage.h>
//Added by qt3to4:
#include <QMouseEvent>

#define BAR_WIDTH 10 // FIXME hardcoded value

static void drawProgress(float progress, float w, float h)
{
 // progress is a percentage value 0..100
 if (progress >= 100.0f) {
	return;
 }
 if (progress < 0.0f) {
	progress = 0.0f;
 }
 glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 float factor_100 = (qMin(100.0f - progress, 12.5f)) / 12.5f;
 float factor_875 = (qMin(87.5f  - progress, 12.5f)) / 12.5f;
 float factor_750 = (qMin(75.0f  - progress, 12.5f)) / 12.5f;
 float factor_625 = (qMin(62.5f  - progress, 12.5f)) / 12.5f;
 float factor_500 = (qMin(50.0f  - progress, 12.5f)) / 12.5f;
 float factor_375 = (qMin(37.5f  - progress, 12.5f)) / 12.5f;
 float factor_250 = (qMin(25.0f  - progress, 12.5f)) / 12.5f;
 float factor_125 = (qMin(12.5f  - progress, 12.5f)) / 12.5f;

 // AB: this code could be made faster, but this would make it less readable
 glBegin(GL_TRIANGLE_FAN);
	glVertex2f(w/2.0f, h/2.0f);
	glVertex2f(w/2.0f, 0);

	if (factor_100 >= 0.0f) { // progress < 100.0f
		glVertex2f((w/2.0f) * (1.0f - factor_100), 0);
	}
	if (factor_875 >= 0.0f) { // progress < 87.5f
		glVertex2f(0, (h/2.0f) * (factor_875));
	}
	if (factor_750 >= 0.0f) { // progress < 75.0f
		glVertex2f(0, h/2.0f + (h/2.0f) * (factor_750));
	}
	if (factor_625 >= 0.0f) { // progress < 62.5f
		glVertex2f((w/2.0f) * (factor_625), h);
	}
	if (factor_500 >= 0.0f) { // progress < 50.0f
		glVertex2f(w/2.0f + (w/2.0f) * (factor_500), h);
	}
	if (factor_375 >= 0.0f) { // progress < 37.5f
		glVertex2f(w, h/2.0f + (h/2.0f) * (1.0f - factor_375));
	}
	if (factor_250 >= 0.0f) { // progress < 25.0f
		glVertex2f(w, (h/2.0f) * (1.0f - factor_250));
	}
	if (factor_125 >= 0.0f) { // progress < 12.5f
		glVertex2f(w/2.0f + (w/2.0f) * (1.0f - factor_125), 0);
	}
 glEnd();



 glColor3f(255, 255, 255);
 glDisable(GL_BLEND);
}


class BoOrderButtonDrawable : public BoUfoDrawable
{
public:
	BoOrderButtonDrawable() : BoUfoDrawable()
	{
		mProductionStatus = BosonOrderButton::CanProduce;
		mProductionCount = 0;
		mProgressPercentage = 0.0f;

		mFont = new BoUfoStandaloneFont(BoUfoManager::currentUfoManager());
	}
	~BoOrderButtonDrawable()
	{
		delete mFont;
	}

	virtual void render(int x, int y, int w, int h);

	virtual int drawableWidth() const;
	virtual int drawableHeight() const;

	void setImage(const BoUfoImage& img)
	{
		mImage = img;
	}
	void setProductionStatus(BosonOrderButton::ProductionStatus s)
	{
		mProductionStatus = s;
	}
	void setProductionCount(int c)
	{
		mProductionCount = c;
	}
	void setProgressPercentage(float p)
	{
		mProgressPercentage = p;
	}

private:
	BoUfoImage mImage;
	BosonOrderButton::ProductionStatus mProductionStatus;
	int mProductionCount;
	float mProgressPercentage;

	BoUfoStandaloneFont* mFont;
};

void BoOrderButtonDrawable::render(int x, int y, int w, int h)
{
 mImage.paint(QRect(x, y, w, h));
 if (mProductionStatus != BosonOrderButton::CanProduce) {
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	if (mProductionStatus == BosonOrderButton::CannotProduce) {
		glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
	} else if (mProductionStatus == BosonOrderButton::Producing) {
		glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
		glVertex2i(x, y);
		glVertex2i(x + w, y);
		glVertex2i(x + w, y + h);
		glVertex2i(x, y + h);
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
	glPopAttrib();
 }
 glTranslatef((float)x, (float)y, 0);
 drawProgress(mProgressPercentage, (float)w, (float)h);
 glTranslatef(-(float)x, -(float)y, 0);

 if (mProductionCount > 0) {
	if (mProductionStatus == BosonOrderButton::CanProduce && mProgressPercentage >= 100.0f) {
		mFont->drawString(i18n("Ready"), x + 5, y + 8 + 2*mFont->height());
	}
	mFont->drawString(QString::number(mProductionCount), x + 5, y + 5 + mFont->height());
 } else if (mProductionCount == -1) {
	// TODO: if the i18n()'ed text does not fit into the widget change the
	// font size!
	mFont->drawString(i18n("Pause"), x + 5, y + 5 + mFont->height());
 }
}

int BoOrderButtonDrawable::drawableWidth() const
{
 return mImage.width();
}

int BoOrderButtonDrawable::drawableHeight() const
{
 return mImage.height();
}


BoOrderButtonButton::BoOrderButtonButton()
	: BoUfoPushButton()
{
 setTakesKeyboardFocus(false);
 connect(this, SIGNAL(signalMouseReleased(QMouseEvent*)),
		this, SLOT(slotMouseReleaseEvent(QMouseEvent*)));

 mDrawable = new BoOrderButtonDrawable();
 mDrawable->setProductionStatus(BosonOrderButton::CanProduce);
 mDrawable->setProductionCount(0);
 mDrawable->setProgressPercentage(100.0f);

 BoUfoPushButton::setIcon(*mDrawable);
}

BoOrderButtonButton::~BoOrderButtonButton()
{
 delete mDrawable;
}

void BoOrderButtonButton::setProductionStatus(BosonOrderButton::ProductionStatus s)
{
 mDrawable->setProductionStatus(s);
}

void BoOrderButtonButton::setImage(const BoUfoImage& img)
{
 mDrawable->setImage(img);
}

void BoOrderButtonButton::setProductionCount(int c)
{
 mDrawable->setProductionCount(c);
}

void BoOrderButtonButton::setProgressPercentage(float percentage)
{
 mDrawable->setProgressPercentage(percentage);
}

void BoOrderButtonButton::slotMouseReleaseEvent(QMouseEvent* e)
{
 if (e->button() == Qt::RightButton) {
	emit signalRightClicked();
	e->accept();
 } else if (e->button() == Qt::LeftButton) {
	emit signalLeftClicked();
	e->accept();
 }
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
 mProductionStatus = CanProduce;

 setLayoutClass(UHBoxLayout);
 setOpaque(false);

 BoUfoWidget* display = new BoUfoWidget();
 addWidget(display);
 display->setLayoutClass(UHBoxLayout);

 mPixmapButton = new BoOrderButtonButton();
 mPixmapButton->setOpaque(false);
 display->addWidget(mPixmapButton);
 mPixmapButton->setMouseEventsEnabled(true, true);
 connect(mPixmapButton, SIGNAL(signalLeftClicked()), this, SLOT(slotLeftClicked()));
 connect(mPixmapButton, SIGNAL(signalRightClicked()), this, SLOT(slotRightClicked()));
 connect(mPixmapButton, SIGNAL(signalMouseEntered(ufo::UMouseEvent*)), this, SIGNAL(signalMouseEntered()));
 connect(mPixmapButton, SIGNAL(signalMouseExited(ufo::UMouseEvent*)), this, SIGNAL(signalMouseLeft()));
// mPixmapButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

 mHealth = new BoUfoProgress();
 mHealth->setRange(0.0, 1.0);
 display->addWidget(mHealth);
 mHealth->setOrientation(Qt::Vertical);
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
 connect(mUnit->owner(), SIGNAL(signalUnitChanged(Unit*)),
		this, SLOT(slotUnitChanged(Unit*)));
 slotUnitChanged(mUnit);

 show();

 mHealth->show();

 setProductionCount(0);
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
 mAction = BoSpecificAction();

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
}

void BosonOrderButton::displayUnitPixmap(Unit* unit)
{
 if (!unit) {
	boError(220) << k_funcinfo << "NULL unit" << endl;
	return;
 }
 displayUnitPixmap(unit->type(), unit->owner());
}

void BosonOrderButton::displayUnitPixmap(quint32 unitType, const Player* owner)
{
 if (!owner) {
	boError(220) << k_funcinfo << "NULL owner" << endl;
	return;
 }
 BoUfoImage* small = boViewData->speciesData(owner->speciesTheme())->smallOverview(unitType, owner->teamColor());
 if (!small) {
	boError(220) << k_funcinfo << "Cannot find small overview for "
			<< unitType << endl;
	return;
 }
 setImage(*small);
}

void BosonOrderButton::setImage(const BoUfoImage& image, float progressPercentage)
{
 if (image.isNull()) {
	boError() << k_funcinfo << "NULL image" << endl;
	return;
 }
 mPixmapButton->setImage(image);
 mPixmapButton->setProgressPercentage(progressPercentage);
 mPixmapButton->show();
}

void BosonOrderButton::slotLeftClicked()
{
 if (type() == ShowAction && mProductionStatus == CannotProduce) {
	return;
 }

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
 if (type() == ShowAction && mProductionStatus == CannotProduce) {
	return;
 }

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
 BoUfoImage* image = mAction.image();
 if (!image) {
	boError(220) << k_funcinfo << "NULL image for action " << mAction.id() << endl;
	return;
 }
 setImage(*image, percentage);
}

void BosonOrderButton::setProductionStatus(ProductionStatus s)
{
 mProductionStatus = s;
 mPixmapButton->setProductionStatus(s);
}

void BosonOrderButton::setProductionCount(int count)
{
 mPixmapButton->setProductionCount(count);
}

quint32 BosonOrderButton::productionId() const
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

