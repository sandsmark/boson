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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOGROUNDQUADTREENODE_H
#define BOGROUNDQUADTREENODE_H

#include "boquadtreenode.h"
#include "boquadtreecollection.h"
#include "../math/borect.h"

class BosonMap;

/**
 * This class extends the default quad tree by min/max z values for the cells at
 * each node.
 *
 * It is highly recommended to use @ref BosonMap::registerQuadTree with every
 * tree of this class, otherwise the quadtree may get out of date e.g. when used
 * in editor.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGroundQuadTreeNode : public BoQuadTreeNode
{
public:
	BoGroundQuadTreeNode(int l, int t, int r, int b, int depth = 0);
	virtual ~BoGroundQuadTreeNode();

	/**
	 * Create a quadtree on a map of size @p width * @p height.
	 * @return The root of the tree
	 **/
	static BoGroundQuadTreeNode* createTree(unsigned int width, unsigned int height);

	virtual void cellTextureChanged(const BosonMap* map, int x1, int y1, int x2, int y2);
	virtual void cellHeightChanged(const BosonMap* map, int x1, int y1, int x2, int y2);

	float groundMinZ() const
	{
		return mGroundMinZ;
	}
	float groundMaxZ() const
	{
		return mGroundMaxZ;
	}

	BoRect3Float groundBoundingBox() const
	{
		return BoRect3Float((float)left(), (float)-top(), groundMinZ(),
				(float)(right() + 1), (float)-(bottom() + 1), groundMaxZ());
	}

protected:
	virtual BoQuadTreeNode* createNode(int l, int t, int r, int b, int depth) const;

	void recalculateTextureInThisNode(const BosonMap* map);
	void recalculateHeightInThisNode(const BosonMap* map);

	// TODO: move to BoQuadTreeNode

private:
	float mGroundMinZ;
	float mGroundMaxZ;
};

class BoGroundQuadTreeCollection : public BoQuadTreeCollection
{
	Q_OBJECT
public:
	BoGroundQuadTreeCollection(QObject* parent);
	~BoGroundQuadTreeCollection();

	void cellHeightChanged(const BosonMap* map, int x1, int y1, int x2, int y2);
	void cellTextureChanged(const BosonMap* map, int x1, int y1, int x2, int y2);
};


#endif

