/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonsprite.h"

#include "../bosoncanvas.h"
#include "../rtti.h"
#include "../selectbox.h"

#include <kdebug.h>

#ifndef NO_OPENGL
BosonSprite::BosonSprite(BosonTextureArray* array, BosonCanvas* canvas)
	: GLSprite(array, canvas)
#else
BosonSprite::BosonSprite(QCanvasPixmapArray* array, BosonCanvas* canvas)
	: QCanvasSprite(array, (QCanvas*)canvas)
#endif
{
 if (canvas) {
	canvas->addItem(this);
 }
 mIsAnimated = false;
 mSelectBox = 0;
}

BosonSprite::~BosonSprite()
{
 unselect();
 if (boCanvas()) {
	boCanvas()->removeItem(this);
	boCanvas()->removeAnimation(this);
 }
}

void BosonSprite::setCanvas(BosonCanvas* c)
{
#ifndef NO_OPENGL
 bool v = isVisible();
 setVisible(false);
 if (boCanvas()) {
	boCanvas()->removeItem(this);
	boCanvas()->removeAnimation(this);
 }
 GLSprite::setCanvas(c);
 if (boCanvas()) {
	boCanvas()->addItem(this);
	// FIXME: in case the item was animated before it should be added here
	// again
	// boCanvas()->addAnimation(this);
 }
 setVisible(v);
#else
 QCanvasSprite::setCanvas((QCanvas*)c);
#endif
}

QPointArray BosonSprite::cells() const
{
 // FIXME: test with both, QCanvas and OpenGL!
 QPointArray c;
 int left, right, top, bottom;
 int n = 0;
 leftTopCell(&left, &top);
 rightBottomCell(&right, &bottom);
 left = QMAX(left, 0);
 top = QMAX(top, 0);
 right = QMIN(right, QMAX((int)boCanvas()->mapWidth() - 1, 0));
 bottom = QMIN(bottom, QMAX((int)boCanvas()->mapHeight() - 1, 0));
 int size = (right - left + 1) * (bottom - top + 1);
 if (size <= 0) {
	return QPointArray();
 }
 c.resize(size);
 for (int i = left; i <= right; i++) {
	for (int j = top; j <= bottom; j++) {
		c[n++] = QPoint(i, j);
	}
 }
 return c;
}

#ifdef NO_OPENGL
bool BosonSprite::collidesWith(QCanvasItem* item) const
{
 // this function is for non-opengl use only. it should not be called from
 // anywhere inside boson - use bosonCollidesWith() instead. this is just for
 // any calls from inside QCanvas, if they ever occur (should not)
 if(!RTTI::isUnit(item->rtti())) {
	// Never collide with selectpart, shot or fog of war
	if(item->rtti() == RTTI::SelectPart || item->rtti() == RTTI::BoShot || item->rtti() == RTTI::FogOfWar) {
		return false;
	}
	if(item->rtti() == QCanvasItem::Rtti_Rectangle) {
		QRect itemrect = ((QCanvasRectangle*)item)->boundingRectAdvanced();
		return itemrect.intersects(boundingRectAdvanced());
	}
	return QCanvasSprite::collidesWith(item);
 }
 // since it is a unit it must be a BosonSprite* too
 return bosonCollidesWith((BosonSprite*)item);
}
#endif // NO_OPENGL

bool BosonSprite::bosonCollidesWith(BosonSprite* item) const
{
  // New collision-check method for units
 if(!RTTI::isUnit(item->rtti())) {
	// Never collide with selectpart, shot or fog of war
	if(item->rtti() == RTTI::SelectPart || item->rtti() == RTTI::BoShot || item->rtti() == RTTI::FogOfWar) {
		return false;
	}
	// we have unknown item here!
	// this must not happen, since an unknown item here is a major
	// performance problem - but at least it'll be important to fix it
	// then :)
	kdWarning() << k_funcinfo << "unknown item - rtti=" << item->rtti() << endl;
	return false;
 }

 // I use centers of units as positions here
 double myx, myy, itemx, itemy;
 QRect r = boundingRectAdvanced();
 QRect r2 = item->boundingRectAdvanced();
 myx = r.center().x();
 myy = r.center().y();
 itemx = r2.center().x();
 itemy = r2.center().y();

 double itemw, itemh;
 itemw = r2.width();
 itemh = r2.height();

 if(itemw <= BO_TILE_SIZE && itemh <= BO_TILE_SIZE) {
	double dist = QABS(itemx - myx) + QABS(itemy - myy);
	return (dist < BO_TILE_SIZE);
 } else {
	for(int i = 0; i < itemw; i += BO_TILE_SIZE) {
		for(int j = 0; j < itemh; j += BO_TILE_SIZE) {
			double dist = QABS((itemx + i) - myx) + QABS((itemy + j) - myy);
			if(dist < BO_TILE_SIZE) {
				return true;
			}
		}
	}
 }
 return false;
}

void BosonSprite::setAnimated(bool a)
{
 if (mIsAnimated != a) {
	mIsAnimated = a;
	if (a) {
		boCanvas()->addAnimation(this);
	} else {
		boCanvas()->removeAnimation(this);
	}
 }
}

void BosonSprite::select(bool markAsLeader)
{
 if (mSelectBox) {
	// already selected
	return;
 }
 mSelectBox = new SelectBox(this, boCanvas(), markAsLeader);
 
}

void BosonSprite::unselect()
{
 delete mSelectBox;
 mSelectBox = 0;
}
