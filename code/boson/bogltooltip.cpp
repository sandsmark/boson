/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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
#include "bogltooltip.h"
#include "bogltooltip.moc"

#include "../bomemory/bodummymemory.h"
#include "botooltipcreator.h"
#include "bodebug.h"
#include "bosoncanvas.h"
#include "bosonitem.h"
#include "bosonconfig.h"
#include "boufo/boufo.h"
#include "playerio.h"
#include "bo3dtools.h"

#include <qmap.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QMouseEvent>

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

Q3ValueList<int> BoToolTipCreatorFactory::availableTipCreators() const
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
	BoVector3Fixed mCursorCanvasVector;

};

BoGLToolTip::BoGLToolTip(BoUfoWidget* v) : QObject(v)
{
 d = new BoGLToolTipPrivate;
 mView = v;
 mCanvas = 0;
 mShowTip = false;
 mUpdatePeriod = 0;
 mCreator = 0;
 mPlayerIO = 0;
 mLabel = 0;

 connect(&d->mTimer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdate()));

 setUpdatePeriod(boConfig->intValue("ToolTipUpdatePeriod"));
 setToolTipCreator(boConfig->intValue("ToolTipCreator"));

 connect(mView, SIGNAL(signalMouseMoved(QMouseEvent*)),
		this, SLOT(slotMouseMoved(QMouseEvent*)));
}

BoGLToolTip::~BoGLToolTip()
{
 kapp->setGlobalMouseTracking(false);
 delete d;
}

void BoGLToolTip::setPlayerIO(PlayerIO* io)
{
 mPlayerIO = io;
}

void BoGLToolTip::setCanvas(BosonCanvas* canvas)
{
 mCanvas = canvas;
}

void BoGLToolTip::setLabel(BoUfoLabel* l)
{
 mLabel = l;
 updateLabel();
}

void BoGLToolTip::slotSetCursorCanvasVector(const BoVector3Fixed& v)
{
 d->mCursorCanvasVector = v;
}

void BoGLToolTip::slotTimeOut()
{
 BO_CHECK_NULL_RET(mView);
 if (!mPlayerIO) {
	return;
 }
 BO_CHECK_NULL_RET(mLabel);
 if (!mCanvas) {
	// this is NOT an error
	return;
 }
 if (!mView->hasMouse() || !mCanvas->onCanvas(d->mCursorCanvasVector)) {
	hideTip();
	return;
 }
 mShowTip = true;

 // TODO: find out whether there is actually the canvas below the cursor and not
 // e.g. the command frame!
 BosonItem* item = mPlayerIO->findItemAt(d->mCursorCanvasVector);
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
 updateLabel();
}

void BoGLToolTip::unsetItem(BosonItem* item)
{
 if (d->mToolTip.item() == item) {
	hideTip();
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
 updateLabel();
}

void BoGLToolTip::slotUpdate()
{
 d->mToolTip.update();
 updateLabel();
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

void BoGLToolTip::updateLabel()
{
 if (!mLabel) {
	return;
 }
 mLabel->setVisible(showTip());
 mLabel->setText(d->mToolTip.tip());
 mLabel->setSize(mLabel->preferredWidth(), mLabel->preferredHeight());
}

void BoGLToolTip::setCursorPos(const QPoint& pos)
{
 const int cursorOffset = 15;
 const int viewportWidth = mView->width();
 const int viewportHeight = mView->height();
 int width = mLabel->width();
 int height = mLabel->height();
 int x;
 int y;

 if (viewportWidth - (pos.x() + cursorOffset) >= width) {
	// to the right of the cursor
	x = pos.x() + cursorOffset;
 } else if (pos.x() - cursorOffset >= width) {
	// to the left of the cursor
	x = pos.x() - cursorOffset - width;
 } else {
	// not enough space anyway - pick where we can get most space
	if (pos.x() > viewportWidth / 2) {
		x = pos.x() + cursorOffset;
	} else {
		x = qMax(0, pos.x() - cursorOffset - width);
	}
 }

 if (pos.y() + cursorOffset + height < viewportHeight) {
	y = viewportHeight - (pos.y() + cursorOffset);
 } else if (pos.y() >= height + cursorOffset) {
	y = viewportHeight - (pos.y() - (cursorOffset + height));
 } else {
	if (pos.y() < viewportHeight / 2) {
		y = viewportHeight - (pos.y() + cursorOffset);
	} else {
		y = viewportHeight - (pos.y() - (cursorOffset + height));
	}
 }

 // AB: the code above still assumes that y is flipped, is it is with normal
 // OpenGL. however libufo expects "normal" widget coordinates again.
 y = viewportHeight - y;

 mLabel->setPos(x, y);
}

void BoGLToolTip::slotMouseMoved(QMouseEvent* e)
{
 hideTip();
 d->mTimer.stop();
 d->mTimer.start(toolTipDelay());
 setCursorPos(e->pos());
}



