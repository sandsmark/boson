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
#include "bogltooltip.h"
#include "bogltooltip.moc"

#include "bodebug.h"
#include "rtti.h"
#include "bosonbigdisplaybase.h"
#include "bosoncanvas.h"
#include "bosonconfig.h"
#include "bosonglfont.h"
#include "items/bosonitem.h"
#include "unit.h"

#include <qmap.h>
#include <qtimer.h>

#include <kapplication.h>
#include <klocale.h>

#define TOOLTIP_DELAY 600

class BoToolTip
{
public:
	BoToolTip();
	~BoToolTip();

	void clear();
	void setItem(BosonItem* item);
	void update();

	inline bool isEmpty() const { return (mTip.isNull() || !mItem); }

	const QString& tip() const { return mTip; }

private:
	BosonItem* mItem;
	QString mTip;
};

BoToolTip::BoToolTip()
{
 mItem = 0;
}

BoToolTip::~BoToolTip()
{
}

void BoToolTip::setItem(BosonItem* item)
{
 if (mItem == item) {
	// nothing to do.
	return;
 }
 mItem = item;
 update();
}

void BoToolTip::clear()
{
 setItem(0);
 mTip = QString::null;
}

void BoToolTip::update()
{
 if (!mItem) {
	mTip = QString::null;
	return;
 }
 if (!RTTI::isUnit(mItem->rtti())) {
	mTip = QString::null;
	return;
 }

 // AB: not sure about this. we could add a virutal method BosonItem::tooltip()
 // and make each item generate its own tooltip. we could avoid includign unit.h
 // that way.
 // but once we have configurable tooltips we would have to include
 // bosonconfig.h in unit.cpp and maybe in bosonitem.cpp which I like even less.
 Unit* u = (Unit*)mItem;
 mTip = i18n("%1\nHealth: %2").arg(u->name()).arg(u->health());
}

class BoGLToolTipPrivate
{
public:
	BoGLToolTipPrivate()
	{
	}
	QTimer mTimer;
	QTimer mUpdateTimer;
	BoToolTip mToolTip;

};

BoGLToolTip::BoGLToolTip(BosonBigDisplayBase* v) : QObject(v)
{
 d = new BoGLToolTipPrivate;
 mView = v;
 mShowTip = false;
 mUpdatePeriod = 0;

 kapp->setGlobalMouseTracking(true);
 kapp->installEventFilter(this);
 connect(&d->mTimer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdate()));

 setUpdatePeriod(DEFAULT_TOOLTIP_UPDATE_PERIOD);
}

BoGLToolTip::~BoGLToolTip()
{
 kapp->setGlobalMouseTracking(false);
 delete d;
}

bool BoGLToolTip::eventFilter(QObject* o, QEvent* event)
{
 if (!o || !event) {
	return false;
 }
 if (!o->isWidgetType()) {
	return false;
 }
 if (((BosonGLWidget*)((QWidget*)o)) != mView) {
	return false;
 }
 switch (event->type()) {
	case QEvent::MouseMove:
		hideTip();
		d->mTimer.stop();
		d->mTimer.start(toolTipDelay());
		break;
	// AB: well .. do we need to catch other events?
	// such as when window lost focus? maybe hide tips then? or when the
	// player clicked?
	default:
		break;
 }
 return false;
}

void BoGLToolTip::slotTimeOut()
{
 BO_CHECK_NULL_RET(mView);
 BO_CHECK_NULL_RET(mView->canvas());
 BO_CHECK_NULL_RET(mView->canvas()->collisions());
 if (!mView->hasMouse()) {
	hideTip();
	return;
 }
 mShowTip = true;

 BosonCollisions* c = mView->canvas()->collisions();
 BosonItem* item = c->findItemAt(mView->cursorCanvasVector());
 if (!item) {
	hideTip();
	return;
 }
 boDebug() << k_funcinfo << endl;
 d->mToolTip.setItem(item);
 if (d->mToolTip.isEmpty()) {
	hideTip();
	return;
 }
}

int BoGLToolTip::toolTipDelay()
{
 return TOOLTIP_DELAY;
}

void BoGLToolTip::hideTip()
{
 mShowTip = false;
 d->mToolTip.clear();
}

void BoGLToolTip::renderToolTip(int cursorX, int cursorY, int* viewport, BosonGLFont* font)
{
 const int cursorOffset = 15;
 const int minToolTipWidth = 100;
 const int viewportWidth = viewport[2];
 const int viewportHeight = viewport[3];
 QString tip = d->mToolTip.tip();
 if (tip.isNull()) {
	return;
 }
 BO_CHECK_NULL_RET(font);

 int tipWidth = font->width(tip);
 tipWidth = QMIN(tipWidth, minToolTipWidth);
 int x;
 int y;

 int w = 0;
 // we try to show the tip to the right of the cursor, if we have at least
 // tipWidth space, otherwise to the left if we have enough space there.
 // if both doesn't apply, we just pick the direction where we have most space
 if (viewportWidth - (cursorX + cursorOffset) >= tipWidth) {
	// to the right of the cursor
	x = cursorX + cursorOffset;
	w = viewportWidth - x;
 } else if (cursorX - cursorOffset >= tipWidth) {
	// to the left of the cursor
	x = cursorX - cursorOffset - tipWidth;
	w = tipWidth;
 } else {
	// not enough space anyway - pick where we can get most space
	if (cursorX > viewportWidth / 2) {
		x = cursorX + cursorOffset;
		w = viewportWidth - x;
	} else {
		x = QMAX(0, cursorX - cursorOffset - tipWidth);
		w = cursorX - cursorOffset;
	}
 }

 int h = font->height(tip, w);
 if (cursorY + cursorOffset + h < viewportHeight) {
	y = viewportHeight - (cursorY + cursorOffset);
 } else if (cursorY >= h + cursorOffset) {
	y = viewportHeight - (cursorY - (cursorOffset + h));
 } else {
	if (cursorY < viewportHeight / 2) {
		y = viewportHeight - (cursorY + cursorOffset);
	} else {
		y = viewportHeight - (cursorY - (cursorOffset + h));
	}
 }

 font->renderText(x, y, tip, w);
}

void BoGLToolTip::slotUpdate()
{
 d->mToolTip.update();
}

void BoGLToolTip::setUpdatePeriod(int ms)
{
 if (ms < 1) {
	// 0 would be dangerous concerning performance. 1 is bad enough already.
	boError() << k_funcinfo << "period must be greater than 1" << endl;
	return;
 }
 mUpdatePeriod = ms;
 d->mUpdateTimer.stop();
 d->mUpdateTimer.start(ms);
}

