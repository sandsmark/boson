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

class KSpriteToolTip::KSpriteToolTipPrivate
{
public:
	KSpriteToolTipPrivate()
	{
	
	}

	QMap<int, QString> mRttiList;
	QMap<QCanvasItem*, QString> mItemList;
};


KSpriteToolTip::KSpriteToolTip(QCanvasView* v) : QToolTip(v)
{
 d = new KSpriteToolTipPrivate;
 mView = v;
}

KSpriteToolTip::~KSpriteToolTip()
{
 delete d;
}

void KSpriteToolTip::add(int rtti, const QString& tip)
{
 if (d->mRttiList.contains(rtti)) {
	return;
 }
 d->mRttiList.insert(rtti, tip);
}

void KSpriteToolTip::add(QCanvasItem* item, const QString& tip)
{
 if (d->mItemList.contains(item)) {
	return;
 }
//TODO remove on destruction of the item
 d->mItemList.insert(item, tip);
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
 QString text;
 QCanvasItem* item = sprites.front();

 // look first if there is a tip foir this special item. if not look for a tip
 // for this rtti
 QMap<QCanvasItem*, QString>::iterator itemIt = d->mItemList.find(item);
 if (itemIt != d->mItemList.end()) {
	text = itemIt.data();
 } else {
	QMap<int, QString>::iterator rttiIt = d->mRttiList.find(item->rtti());
	if (rttiIt != d->mRttiList.end()) {
		text = rttiIt.data();
	}
 }

 if (text != QString::null) {
	tip (QRect (p, p), text); // display the tool tip
 }
}


