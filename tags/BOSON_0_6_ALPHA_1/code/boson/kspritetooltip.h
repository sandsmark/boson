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
#ifndef KSPRITETOOLTIP_H
#define KSPRITETOOLTIP_H

#include <qtooltip.h>

class QCanvasView;
class QCanvasItem;

/**
 * Simply create a new KSpriteToolTip for every @ref QCanvasView your
 * application offers. Then add tooltips using @ref add. You may either provide
 * the @ref QCanvasItem::rtti or a pointer to a @ref QCanvasItem.
 *
 * Note that the @ref QCanvasItem::rtti matches <em>all</em> items with this
 * rtti, but the pointer version only this single item.
 *
 * If you provide a tooltip on a special @ref QCanvasItem which already exists
 * an rtti-tooltip for, then only the @ref QCanvasItem tip is used.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KSpriteToolTip : public QToolTip
{
public:
	KSpriteToolTip(QCanvasView* v);
	virtual ~KSpriteToolTip();

	/**
	 * Add a tip that gets displayed for every item which @ref
	 * QCanvasItem::rtti equals rtti. Note that you can override this tip if
	 * you add a tip providing the pointer of the @ref QCanvasItem
	 * @param rtti The @ref QCanvasItem::rtti this tip is displayed for
	 * @param tip Well... the tip!
	 **/
	static void add(int rtti, const QString& tip);

	/**
	 * Add a tip for a single item only. Note that this tip does
	 * <em>not</em> get removed once the unit is deleted! Use @ref remove in
	 * your destructor!
	 *
	 * This tip overrides any rtti-dependant tip.
	 * @param item The @ref QCanvasItem this tip is displayed for
	 * @param tip Well... the tip!
	 **/
	static void add(QCanvasItem* item, const QString& tip);
	static void remove(QCanvasItem* item);
	static void remove(int rtti);
	
protected:
	virtual void maybeTip(const QPoint& pos);

private:
	QCanvasView* mView;
};

#endif
