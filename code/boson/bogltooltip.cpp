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

#include "botooltipcreator.h"
#include "bodebug.h"
#include "bosonbigdisplaybase.h"
#include "bosoncanvas.h"
#include "bosonfont/bosonglfont.h"
#include "items/bosonitem.h"
#include "bosonconfig.h"

#include <qmap.h>
#include <qtimer.h>

#include <kapplication.h>
#include <klocale.h>

#define TOOLTIP_DELAY 600


class BoToolTipCreatorFactoryPrivate
{
public:
	BoToolTipCreatorFactoryPrivate()
	{
	}
	QMap<int, QString> mNames;
};

BoToolTipCreatorFactory::BoToolTipCreatorFactory()
{
 d = new BoToolTipCreatorFactoryPrivate;
 registerTipCreator(BoToolTipCreator::Basic, i18n("Basic Tooltips"));
 registerTipCreator(BoToolTipCreator::Extended, i18n("Extended Tooltips"));
 registerTipCreator(BoToolTipCreator::Debug, i18n("Tooltips for debugging"));
}

BoToolTipCreatorFactory::~BoToolTipCreatorFactory()
{
 delete d;
}

QValueList<int> BoToolTipCreatorFactory::availableTipCreators() const
{
 return d->mNames.keys();
}

QString BoToolTipCreatorFactory::tipCreatorName(int type) const
{
 return d->mNames[type];
}

BoToolTipCreator* BoToolTipCreatorFactory::tipCreator(int type) const
{
 BoToolTipCreator* tipCreator = 0;
 switch (type) {
	case BoToolTipCreator::Basic:
		tipCreator = new BoToolTipCreatorBasic();
		break;
	default:
		boWarning() << k_funcinfo << "Unknown tooltip creator type " << type << endl;
		// no break - Extended is default
	case BoToolTipCreator::Extended:
		tipCreator = new BoToolTipCreatorExtended();
		break;
	case BoToolTipCreator::Debug:
		tipCreator = new BoToolTipCreatorDebug();
		break;
 }
 return tipCreator;
}

void BoToolTipCreatorFactory::registerTipCreator(int type, const QString& name)
{
 if (d->mNames.contains(type)) {
	boError() << k_funcinfo << "type=" << type << " already registered!" << endl;
	return;
 }
 d->mNames.insert(type, name);
}



class BoToolTip
{
public:
	BoToolTip();
	~BoToolTip();

	/**
	 * Use @p creator to create the tooltip. Note that this class will
	 * take ownership of the object and therefore will delete it
	 **/
	void setToolTipCreator(BoToolTipCreator* creator);

	/**
	 * Clear the tooltip. This removes any data from this class and ensures
	 * that @ref isEmpty returns TRUE. This also ensures that the class
	 * doesn't point to invalid objects (e.g. to deleted items)
	 **/
	void clear();

	/**
	 * Make this tooltip display an item.
	 **/
	// AB: we could also add set*() functions for e.g. cells or whatever we
	// like
	void setItem(BosonItem* item);

	BosonItem* item() const { return mItem; }

	/**
	 * Update the data of the tooltip. This does <em>not</em> check what is
	 * currently under the cursor, but updates the content of the tooltip
	 * assuming the last called set*() function (e.g. @ref setItem) is still
	 * valid.
	 *
	 * We do not check what is currently under the cursor, because it might
	 * be expensive under some circumstances and update() could get called
	 * pretty often.
	 **/
	void update();

	/**
	 * @return TRUE if this tooltip is empty, i.e. does not contain any
	 * data. No tooltip should be displayed then.
	 **/
	inline bool isEmpty() const { return (mTip.isNull() || !mItem); }

	/**
	 * @return The tooltip that has been created the last time @ref update
	 * was called.
	 **/
	const QString& tip() const { return mTip; }

private:
	BoToolTipCreator* mCreator;
	BosonItem* mItem;
	QString mTip;
};

BoToolTip::BoToolTip()
{
 mCreator = 0;
 mItem = 0;
}

BoToolTip::~BoToolTip()
{
 delete mCreator;
}

void BoToolTip::setToolTipCreator(BoToolTipCreator* creator)
{
 delete mCreator;
 mCreator = creator;
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
 if (!mCreator) {
	clear();
	return;
 }
 mTip = mCreator->createToolTip(mItem);
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
 mCreator = 0;

 kapp->setGlobalMouseTracking(true);
 kapp->installEventFilter(this);
 connect(&d->mTimer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdate()));

 setUpdatePeriod(boConfig->intValue("ToolTipUpdatePeriod"));
 setToolTipCreator(boConfig->intValue("ToolTipCreator"));
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
 if (!mView->canvas()) {
	// this is NOT an error
	return;
 }
 BO_CHECK_NULL_RET(mView->canvas()->collisions());
 if (!mView->hasMouse() || !mView->canvas()->onCanvas(mView->cursorCanvasVector())) {
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
// boDebug() << k_funcinfo << endl;
 d->mToolTip.setItem(item);
 if (d->mToolTip.isEmpty()) {
	hideTip();
	return;
 }
}

void BoGLToolTip::unsetItem(BosonItem* item)
{
 if (d->mToolTip.item() == item) {
	d->mToolTip.setItem(0);
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

void BoGLToolTip::renderToolTip(int cursorX, int cursorY, const int* viewport, BosonGLFont* font)
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
 font->begin();

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

void BoGLToolTip::setToolTipCreator(int type)
{
 BoToolTipCreatorFactory factory;
 BoToolTipCreator* creator = factory.tipCreator(type);
 d->mToolTip.setToolTipCreator(creator);
}

