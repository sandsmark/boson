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
#include "bogroundquadtreenode.h"
#include "bogroundquadtreenode.moc"

#include "bodebug.h"
#include "bosonmap.h"

BoGroundQuadTreeNode::BoGroundQuadTreeNode(int l, int t, int r, int b, int depth)
	: BoQuadTreeNode(l, t, r, b, depth)
{
 mGroundMinZ = 0.0f;
 mGroundMaxZ = 0.0f;
}

BoGroundQuadTreeNode::~BoGroundQuadTreeNode()
{
}

BoGroundQuadTreeNode* BoGroundQuadTreeNode::createTree(unsigned int width, unsigned int height)
{
 if (width < 1) {
	boError() << k_funcinfo << "invalid width: " << width << endl;
	width = 1;
 }
 if (height < 1) {
	boError() << k_funcinfo << "invalid height: " << height << endl;
	height = 1;
 }
 BoGroundQuadTreeNode* root = new BoGroundQuadTreeNode(0, 0, width - 1, height - 1, 0);
 root->createChilds(width, height);
 return root;
}

BoQuadTreeNode* BoGroundQuadTreeNode::createNode(int l, int t, int r, int b, int depth) const
{
 return new BoGroundQuadTreeNode(l, t, r, b, depth);
}

void BoGroundQuadTreeNode::cellTextureChanged(const BosonMap* map, int x1, int y1, int x2, int y2)
{
 if (!intersects(x1, y1, x2, y2)) {
	return;
 }
 BoQuadTreeNode* children[4];
 getChildren(children);
 for (int i = 0; i < 4; i++) {
	if (children[i]) {
		((BoGroundQuadTreeNode*)children[i])->cellTextureChanged(map, x1, y1, x2, y2);
	}
 }
 recalculateTextureInThisNode(map);
}

void BoGroundQuadTreeNode::cellHeightChanged(const BosonMap* map, int x1, int y1, int x2, int y2)
{
 if (!intersects(x1, y1, x2, y2)) {
	return;
 }
 BoQuadTreeNode* children[4];
 getChildren(children);
 for (int i = 0; i < 4; i++) {
	if (children[i]) {
		((BoGroundQuadTreeNode*)children[i])->cellHeightChanged(map, x1, y1, x2, y2);
	}
 }
 recalculateHeightInThisNode(map);
}

void BoGroundQuadTreeNode::recalculateTextureInThisNode(const BosonMap* map)
{
 Q_UNUSED(map);
}

void BoGroundQuadTreeNode::recalculateHeightInThisNode(const BosonMap* map)
{
 if (left() == right() && top() == bottom()) {
	float h = map->heightAtCorner(left(), top());
	mGroundMinZ = h;
	mGroundMaxZ = h;
	h = map->heightAtCorner(left() + 1, top());
	mGroundMinZ = QMIN(mGroundMinZ, h);
	mGroundMaxZ = QMAX(mGroundMaxZ, h);
	h = map->heightAtCorner(left() + 1, top() + 1);
	mGroundMinZ = QMIN(mGroundMinZ, h);
	mGroundMaxZ = QMAX(mGroundMaxZ, h);
	h = map->heightAtCorner(left(), top() + 1);
	mGroundMinZ = QMIN(mGroundMinZ, h);
	mGroundMaxZ = QMAX(mGroundMaxZ, h);
	return;
 }
 BoQuadTreeNode* children[4];
 getChildren(children);
 mGroundMinZ = ((BoGroundQuadTreeNode*)children[0])->groundMinZ();
 mGroundMaxZ = ((BoGroundQuadTreeNode*)children[0])->groundMaxZ();
 for (int i = 1; i < 4; i++) {
	if (children[i]) {
		mGroundMinZ = QMIN(mGroundMinZ, ((BoGroundQuadTreeNode*)children[i])->groundMinZ());
		mGroundMaxZ = QMAX(mGroundMaxZ, ((BoGroundQuadTreeNode*)children[i])->groundMaxZ());
	}
 }
}


BoGroundQuadTreeCollection::BoGroundQuadTreeCollection(QObject* parent)
	: BoQuadTreeCollection(parent)
{
}

BoGroundQuadTreeCollection::~BoGroundQuadTreeCollection()
{
}

void BoGroundQuadTreeCollection::cellHeightChanged(const BosonMap* map, int x1, int y1, int x2, int y2)
{
 for (QPtrListIterator<BoQuadTreeNode> it(trees()); it.current(); ++it) {
	((BoGroundQuadTreeNode*)it.current())->cellHeightChanged(map, x1, y1, x2, y2);
 }
}

void BoGroundQuadTreeCollection::cellTextureChanged(const BosonMap* map, int x1, int y1, int x2, int y2)
{
 for (QPtrListIterator<BoQuadTreeNode> it(trees()); it.current(); ++it) {
	((BoGroundQuadTreeNode*)it.current())->cellTextureChanged(map, x1, y1, x2, y2);
 }
}



