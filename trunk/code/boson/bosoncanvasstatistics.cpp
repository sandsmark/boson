/*
    This file is part of the Boson game
    Copyright (C) 2003-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosoncanvasstatistics.h"

#include "boitemlist.h"
#include "bosoncanvas.h"
#include "rtti.h"
#include "items/bosonitem.h"

class BosonCanvasStatisticsPrivate
{
public:
	BosonCanvasStatisticsPrivate()
	{
	}

	// For debugging only
	QMap<int, int> mWorkCounts; // How many units are doing what work
	QMap<int, unsigned int> mItemCount;
};

BosonCanvasStatistics::BosonCanvasStatistics(BosonCanvas* parent)
{
 d = new BosonCanvasStatisticsPrivate;
 mCanvas = parent;
}

BosonCanvasStatistics::~BosonCanvasStatistics()
{
 delete d;
}

unsigned int BosonCanvasStatistics::mapHeight() const
{
 return mCanvas->mapHeight();
}

unsigned int BosonCanvasStatistics::mapWidth() const
{
 return mCanvas->mapWidth();
}

unsigned int BosonCanvasStatistics::animationsCount() const
{
 return mCanvas->animationsCount();
}

unsigned int BosonCanvasStatistics::allItemsCount() const
{
 return mCanvas->allItemsCount();
}

unsigned int BosonCanvasStatistics::itemCount(int rtti) const
{
 return d->mItemCount[rtti];
}

unsigned int BosonCanvasStatistics::effectsCount() const
{
 return mCanvas->effectsCount();
}

QMap<int, int>* BosonCanvasStatistics::workCounts() const
{
 return &d->mWorkCounts;
}

void BosonCanvasStatistics::resetWorkCounts()
{
 QMap<int,int>::Iterator it;
 for (it = d->mWorkCounts.begin(); it != d->mWorkCounts.end(); ++it) {
	(*it) = 0;
 }
}

void BosonCanvasStatistics::updateItemCount()
{
 d->mItemCount.clear();
 const BoItemList* allItems = mCanvas->allItems();
 BoItemList::ConstIterator it = allItems->begin();
 for (; it != allItems->end(); ++it) {
	int rtti = (*it)->rtti();
	if (RTTI::isUnit(rtti)) {
		rtti = RTTI::UnitStart;
	}
	if (!d->mItemCount.contains(rtti)) {
		d->mItemCount.insert(rtti, 0);
	}
	d->mItemCount[rtti] += 1;
 }
}

void BosonCanvasStatistics::increaseWorkCount(int work)
{
 d->mWorkCounts[work]++;
}

