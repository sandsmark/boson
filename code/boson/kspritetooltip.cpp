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
#include "kspritetooltip.h"

#include <kdebug.h>

#include <qcanvas.h>
#include <qmap.h>

class KTipManager
{
public:
	void add(int rtti, const QString& tip)
	{
		if (mRttiList.contains(rtti)) {
			return;
		}
		mRttiList.insert(rtti, tip);
	}
	void add(QCanvasItem* item, const QString& tip)
	{
		if (mItemList.contains(item)) {
			return;
		}
		mItemList.insert(item, tip);
	}
	void remove(QCanvasItem* item)
	{
		mItemList.remove(item);
	}
	void remove(int rtti)
	{
		mRttiList.remove(rtti);
	}

	QString tip(QCanvasItem* item)
	{
		QString text;
		// look first if there is a tip foir this special item. if not look for a tip
		// for this rtti
		QMap<QCanvasItem*, QString>::iterator it = mItemList.find(item);
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
	QMap<QCanvasItem*, QString> mItemList;
};

static KTipManager* tipManager = 0;
static void initTipManager()
{
 if (tipManager) {
	return;
 }
 tipManager = new KTipManager;
}

KSpriteToolTip::KSpriteToolTip(QCanvasView* v) : QToolTip(v)
{
 initTipManager();
 mView = v;
}

KSpriteToolTip::~KSpriteToolTip()
{
}

void KSpriteToolTip::add(int rtti, const QString& tip)
{
 initTipManager();
 tipManager->add(rtti, tip);
}

void KSpriteToolTip::add(QCanvasItem* item, const QString& tip)
{
 initTipManager();
 tipManager->add(item, tip);
}

void KSpriteToolTip::remove(QCanvasItem* item)
{
 if (!tipManager) {
	return;
 }
 tipManager->remove(item);
}

void KSpriteToolTip::remove(int rtti)
{
 if (!tipManager) {
	return;
 }
 tipManager->remove(rtti);
}

void KSpriteToolTip::maybeTip(const QPoint& p)
{
 if (!mView) {
	return;
 }
 QCanvasItemList sprites = mView->canvas()->collisions(mView->viewportToContents(p));
 if (sprites.isEmpty())  { // Check if there is a sprite
	return;
 }
 QCanvasItem* item = sprites.front();
 QString text = tipManager->tip(item);

 if (text != QString::null) {
	tip (QRect (item->boundingRect()), text); // display the tool tip
 }
}


