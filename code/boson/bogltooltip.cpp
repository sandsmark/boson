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
#include "items/bosonitem.h"
#include "bosonbigdisplaybase.h"
#include "bosoncanvas.h"

#include <qmap.h>
#include <qtimer.h>

#include <kapplication.h>

#define TOOLTIP_DELAY 600

class BoTipManager
{
public:
	void add(int rtti, const QString& tip)
	{
		if (mRttiList.contains(rtti)) {
			return;
		}
		mRttiList.insert(rtti, tip);
	}
	void add(BosonItem* item, const QString& tip)
	{
		if (mItemList.contains(item)) {
			return;
		}
		mItemList.insert(item, tip);
	}
	void remove(BosonItem* item)
	{
		mItemList.remove(item);
	}
	void remove(int rtti)
	{
		mRttiList.remove(rtti);
	}
	void ignore(int rtti)
	{
		if (mIgnoreRTTIList.contains(rtti)) {
			return;
		}
		mIgnoreRTTIList.append(rtti);
	}
	void ignore(BosonItem* item)
	{
		if (mIgnoreItemList.contains(item)) {
			return;
		}
		mIgnoreItemList.append(item);
	}
	void unignore(int rtti)
	{
		mIgnoreRTTIList.remove(rtti);
	}
	void unignore(BosonItem* item)
	{
		mIgnoreItemList.remove(item);
	}

	bool testIgnore(BosonItem* item) const
	{
		bool i = mIgnoreItemList.contains(item);
		if (i) {
			return i;
		}
		return mIgnoreRTTIList.contains(item->rtti());
	}

	QString tip(BosonItem* item)
	{
		QString text;
		// look first if there is a tip for this special item. if not look for a tip
		// for this rtti
		QMap<BosonItem*, QString>::iterator it = mItemList.find(item);
		if (it != mItemList.end()) {
			text = it.data();
		} else {
			QMap<int, QString>::iterator rttiIt = mRttiList.find(item->rtti());
			if (rttiIt != mRttiList.end()) {
				text = rttiIt.data();
			}
		}
		return text;
	}

private:
	QMap<int, QString> mRttiList;
	QMap<BosonItem*, QString> mItemList;

	// the ignore-lists are relevant for BoGLToolTip::maybeTip()
	QValueList<int> mIgnoreRTTIList;
	QValueList<BosonItem*> mIgnoreItemList;
};

static BoTipManager* tipManager = 0;


class BoGLToolTipPrivate
{
public:
	BoGLToolTipPrivate()
	{
	}
	QTimer mTimer;

};

BoGLToolTip::BoGLToolTip(BosonBigDisplayBase* v) : QObject(v)
{
 initTipManager();
 d = new BoGLToolTipPrivate;
 mView = v;
 mShowTip = false;
 kapp->setGlobalMouseTracking(true);
 kapp->installEventFilter(this);
 connect(&d->mTimer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
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
 if (!mView->hasMouse()) {
	hideTip();
	return;
 }
 mShowTip = true;

 BosonCanvas* c = mView->canvas();
 BosonItem* item = c->findItemAt(mView->cursorCanvasVector());
 if (!item) {
	hideTip();
	return;
 }
 boDebug() << k_funcinfo << endl;
 mCurrentTip = tipManager->tip(item);
 if (mCurrentTip.isNull()) {
	// we should hide it. but for testing...
	hideTip();
	return;
 }
}

void BoGLToolTip::initTipManager()
{
 if (tipManager) {
	return;
 }
 tipManager = new BoTipManager;
}


void BoGLToolTip::add(int rtti, const QString& tip)
{
 initTipManager();
 tipManager->add(rtti, tip);
}

void BoGLToolTip::add(BosonItem* item, const QString& tip)
{
 initTipManager();
 tipManager->add(item, tip);
}

void BoGLToolTip::remove(BosonItem* item)
{
 if (!tipManager) {
	return;
 }
 tipManager->remove(item);
}

void BoGLToolTip::remove(int rtti)
{
 if (!tipManager) {
	return;
 }
 tipManager->remove(rtti);
}

void BoGLToolTip::ignore(int rtti)
{
 if (!tipManager) {
	return;
 }
 tipManager->ignore(rtti);
}

void BoGLToolTip::ignore(BosonItem* item)
{
 if (!tipManager) {
	return;
 }
 tipManager->ignore(item);
}

int BoGLToolTip::toolTipDelay()
{
 return TOOLTIP_DELAY;
}

void BoGLToolTip::hideTip()
{
 mShowTip = false;
 mCurrentTip = QString::null;
}

