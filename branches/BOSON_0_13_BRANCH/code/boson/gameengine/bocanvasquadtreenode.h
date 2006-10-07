/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOCANVASQUADTREENODE_H
#define BOCANVASQUADTREENODE_H

#include "bogroundquadtreenode.h"

class BosonCanvas;
class BosonItem;
class Unit;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCanvasQuadTreeNode : public BoGroundQuadTreeNode
{
public:
	BoCanvasQuadTreeNode(int l, int t, int r, int b, int depth = 0);

	static BoCanvasQuadTreeNode* createTree(unsigned int width, unsigned int height);

	virtual void cellHeightChanged(const BosonMap* map, int x1, int y1, int x2, int y2);
	virtual void cellUnitsChanged(const BosonCanvas* map, int x1, int y1, int x2, int y2);

	float minZ() const
	{
		return mMinZ;
	}
	float maxZ() const
	{
		return mMaxZ;
	}
	float unitMinZ() const
	{
		return mUnitMinZ;
	}
	float unitMaxZ() const
	{
		return mUnitMaxZ;
	}

protected:
	virtual BoQuadTreeNode* createNode(int l, int t, int r, int b, int depth) const;

	void calculateUnitMinMaxZ(const BosonCanvas* canvas);

	/**
	 * Update @ref minZ and @ref maxZ according to the current @ref
	 * unitMinZ/@ref unitMaxZ and @ref groundMinZ/@ref groundMaxZ values.
	 **/
	void updateMinMaxZ();

private:
	float mUnitMinZ;
	float mUnitMaxZ;
	float mMinZ;
	float mMaxZ;
};


/**
 * Collection for @ref BoCanvasQuadTreeNode trees. See @ref BoQuadTreeCollection
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCanvasQuadTreeCollection : public BoGroundQuadTreeCollection
{
	Q_OBJECT
public:
	BoCanvasQuadTreeCollection(QObject* parent);
	~BoCanvasQuadTreeCollection();

	void cellUnitsChanged(const BosonCanvas* canvas, int x1, int y1, int x2, int y2);

public slots:
};

#endif

