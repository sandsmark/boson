/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "boquadtreenode.h"

#include "../../bomemory/bodummymemory.h"
#include <bodebug.h>

BoQuadTreeNode::BoQuadTreeNode(int l, int t, int r, int b)
	: mLeft(l),
	mTop(t),
	mRight(r),
	mBottom(b),
	mNodeSize(0),
	mTopLeft(0),
	mTopRight(0),
	mBottomLeft(0),
	mBottomRight(0)
{
 mNodeSize = (r - l + 1) * (b - t + 1);
 if (mRight < mLeft || mBottom < mTop) {
	boError() << k_funcinfo << "invalid coordinates" << endl;
	mNodeSize = 1;
 }
}

BoQuadTreeNode::~BoQuadTreeNode()
{
 delete mTopLeft;
 delete mTopRight;
 delete mBottomLeft;
 delete mBottomRight;
}

BoQuadTreeNode* BoQuadTreeNode::createNode(int l, int t, int r, int b) const
{
 return new BoQuadTreeNode(l, t, r, b);
}

BoQuadTreeNode* BoQuadTreeNode::createTree(unsigned int w, unsigned int h)
{
 if (w < 1) {
	boError() << k_funcinfo << "invalid width: " << w << endl;
	w = 1;
 }
 if (h < 1) {
	boError() << k_funcinfo << "invalid height: " << h << endl;
	h = 1;
 }
 BoQuadTreeNode* root = new BoQuadTreeNode(0, 0, w - 1, h - 1);
 root->createChilds(w, h);
 return root;
}

void BoQuadTreeNode::createChilds(unsigned int width, unsigned int height)
{
 const int l = mLeft;
 const int t = mTop;
 const int r = mRight;
 const int b = mBottom;

 if (l == r && b == t) {
	// we are a leaf
	return;
 }
 if (l > r || t > b || l < 0 || t < 0) {
	boError() << k_funcinfo << "invalid values: left="
			<< l << ", top=" << t
			<< ", right=" << r
			<< ", bottom=" << b << endl;
	return;
 }
 if (r >= (int)width) {
	boError() << k_funcinfo << "right side is too high: " << r << ", width: " << width << endl;
	return;
 }
 if (b >= (int)height) {
	boError() << k_funcinfo << "bottom side is too high: " << b << ", map height: " << height << endl;
	return;
 }
 int hmid = l + (r - l) / 2;
 int vmid = t + (b - t) / 2;

 mTopLeft = createNode(l, t, hmid, vmid);
 mTopLeft->createChilds(width, height);

 if (vmid + 1 <= b) {
	// (t != b) => we have a bottom rect.
	mBottomLeft = createNode(l, vmid + 1, hmid, b);
	mBottomLeft->createChilds(width, height);
 }
 if (hmid + 1 <= r) {
	// (l != r) => we have a right rect
	mTopRight = createNode(hmid + 1, t, r, vmid);
	mTopRight->createChilds(width, height);
 }
 if (vmid + 1 <= b && hmid + 1 <= r) {
	// ((l != r) && (t != b)) => we have a bottom-right rect
	mBottomRight = createNode(hmid + 1, vmid + 1, r, b);
	mBottomRight->createChilds(width, height);
 }
}

