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
#include "bocanvasquadtreenode.h"
#include "bocanvasquadtreenode.moc"

#include "bodebug.h"
#include "bosoncanvas.h"
#include "rtti.h"
#include "bosonitem.h"
#include "cell.h"
#include "boitemlist.h"
#include "bosonprofiling.h"

BoCanvasQuadTreeNode::BoCanvasQuadTreeNode(int l, int t, int r, int b, int depth)
	: BoGroundQuadTreeNode(l, t, r, b, depth)
{
 mUnitMinZ = 0.0f;
 mUnitMaxZ = 0.0f;
 mMinZ = 0.0f;
 mMaxZ = 0.0f;
}

BoCanvasQuadTreeNode* BoCanvasQuadTreeNode::createTree(unsigned int width, unsigned int height)
{
 if (width < 1) {
	boError() << k_funcinfo << "invalid width: " << width << endl;
	width = 1;
 }
 if (height < 1) {
	boError() << k_funcinfo << "invalid height: " << height << endl;
	height = 1;
 }
 BoCanvasQuadTreeNode* root = new BoCanvasQuadTreeNode(0, 0, width - 1, height - 1, 0);
 root->createChilds(width, height);
 return root;
}

void BoCanvasQuadTreeNode::cellHeightChanged(const BosonMap* map, int x1, int y1, int x2, int y2)
{
 BoGroundQuadTreeNode::cellHeightChanged(map, x1, y1, x2, y2);
 updateMinMaxZ();
}

BoQuadTreeNode* BoCanvasQuadTreeNode::createNode(int l, int t, int r, int b, int depth) const
{
 return new BoCanvasQuadTreeNode(l, t, r, b, depth);
}

void BoCanvasQuadTreeNode::cellUnitsChanged(const BosonCanvas* canvas, int x1, int y1, int x2, int y2)
{
 if (!contains(x1, y1, x2, y2)) {
	return;
 }
 BoQuadTreeNode* children[4];
 getChildren(children);
 for (int i = 0; i < 4; i++) {
	if (children[i]) {
		((BoCanvasQuadTreeNode*)children[i])->cellUnitsChanged(canvas, x1, y1, x2, y2);
	}
 }
 calculateUnitMinMaxZ(canvas);
}

void BoCanvasQuadTreeNode::calculateUnitMinMaxZ(const BosonCanvas* canvas)
{
 // leaf
 if (left() == right() && top() == bottom()) {
	const Cell* cell = canvas->cell(left(), top());
	BO_CHECK_NULL_RET(cell);

	if (cell->unitCount() == 0) {
		mUnitMinZ = 0.0f;
		mUnitMaxZ = 0.0f;
	} else {
		const BoItemList* list = cell->items();
		BoItemList::const_iterator it = list->begin();
		bool firstUnit = true;
		for (; it != list->end(); ++it) {
			if (!RTTI::isUnit((*it)->rtti())) {
				continue;
			}
			const float z1 = (*it)->z();
			const float z2 = (*it)->z() + (*it)->depth();
			if (firstUnit) {
				mUnitMinZ = z1;
				mUnitMaxZ = z2;
				firstUnit = false;
			} else {
				mUnitMinZ = QMAX(mUnitMinZ, z2);
				mUnitMaxZ = QMAX(mUnitMaxZ, z2);
			}
		}
	}
	updateMinMaxZ();
	return;
 }

 // not leaf
 BoQuadTreeNode* children[4];
 getChildren(children);
 bool firstChild = true;
 for (int i = 0; i < 4; i++) {
	if (children[i]) {
		if (firstChild) {
			mUnitMinZ = ((BoCanvasQuadTreeNode*)children[0])->unitMinZ();
			mUnitMaxZ = ((BoCanvasQuadTreeNode*)children[0])->unitMaxZ();
			firstChild = false;
		} else {
			mUnitMinZ = QMIN(mUnitMinZ, ((BoCanvasQuadTreeNode*)children[i])->unitMinZ());
			mUnitMaxZ = QMAX(mUnitMaxZ, ((BoCanvasQuadTreeNode*)children[i])->unitMaxZ());
		}
	}
 }
 updateMinMaxZ();
}

void BoCanvasQuadTreeNode::updateMinMaxZ()
{
 if (unitMinZ() == unitMaxZ()) {
	// no units here
	mMinZ = groundMinZ();
	mMaxZ = groundMaxZ();
	return;
 }
 mMinZ = QMIN(groundMinZ(), unitMinZ());
 mMaxZ = QMAX(groundMaxZ(), unitMaxZ());
}


BoCanvasQuadTreeCollection::BoCanvasQuadTreeCollection(QObject* parent)
	: BoGroundQuadTreeCollection(parent)
{
}

BoCanvasQuadTreeCollection::~BoCanvasQuadTreeCollection()
{
}

void BoCanvasQuadTreeCollection::cellUnitsChanged(const BosonCanvas* canvas, int x1, int y1, int x2, int y2)
{
 PROFILE_METHOD
 for (QPtrListIterator<BoQuadTreeNode> it(trees()); it.current(); ++it) {
	((BoCanvasQuadTreeNode*)it.current())->cellUnitsChanged(canvas, x1, y1, x2, y2);
 }
}

