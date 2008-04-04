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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOQUADTREENODE_H
#define BOQUADTREENODE_H

template<class T> class BoVector3;
typedef BoVector3<float> BoVector3Float;


/**
 * A quad tree.
 *
 * A quad tree operates on some kind of two dimensional data (in the following
 * we use "map" to describe this). The root node of the tree covers the whole
 * map and has four child nodes - upper left, upper right, lower left and lower
 * right, each covering 1/4 of the parent node. Once a node covers only a single
 * cell of the map, it is a leaf and has no children.
 *
 * Since a quadtree is just a tree, its depth is log(n), where n is the maximum
 * of width and height of the map. So for a 512x512 map, the depth is 9.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoQuadTreeNode
{
public:
	/**
	 * @param l The left side of the rect (i.e. the x coordinate of the
	 * most-left cell).
	 * @param t The top side of the rect (i.e. the y coordinate of the
	 * most-top cell).
	 * @param r The right side of the rect (i.e. the x coordinate of the
	 * most-right cell). This eqals @p l, if width of the rect is 1.
	 * @param b The bottom side of the rect (i.e. the y coordinate of the
	 * most-bottom cell). This equals @p t if the height of the rect is 1.
	 * @param depth The depth of this node in the tree. See @ref depth.
	 **/
	BoQuadTreeNode(int l, int t, int r, int b, int depth = 0);
	virtual ~BoQuadTreeNode();

	/**
	 * @return The depth of the node in the tree. The root is at 0.
	 **/
	int depth() const
	{
		return mDepth;
	}

	/**
	 * Create a quadtree on a map of size @p width * @p height.
	 * @return The root of the tree
	 **/
	static BoQuadTreeNode* createTree(unsigned int width, unsigned int height);

	/**
	 * @return The left side of this node as specified in the constructor.
	 **/
	inline int left() const
	{
		return mLeft;
	}

	/**
	 * @return The right side of this node as specified in the constructor.
	 **/
	inline int right() const
	{
		return mRight;
	}

	/**
	 * @return The topside of this node as specified in the constructor.
	 **/
	inline int top() const
	{
		return mTop;
	}

	/**
	 * @return The bottom side of this node as specified in the constructor.
	 **/
	inline int bottom() const
	{
		return mBottom;
	}

	/**
	 * @return The number of cells covered by this node
	 **/
	inline unsigned int nodeSize() const
	{
		return mNodeSize;
	}

	/**
	 * @return The top-left child, or NULL if there is none.
	 **/
	inline BoQuadTreeNode* topLeftNode() const
	{
		return mTopLeft;
	}

	/**
	 * @return The top-right child, or NULL if there is none.
	 **/
	inline BoQuadTreeNode* topRightNode() const
	{
		return mTopRight;
	}

	/**
	 * @return The bottom-left child, or NULL if there is none.
	 **/
	inline BoQuadTreeNode* bottomLeftNode() const
	{
		return mBottomLeft;
	}

	/**
	 * @return The bottom-right child, or NULL if there is none.
	 **/
	inline BoQuadTreeNode* bottomRightNode() const
	{
		return mBottomRight;
	}

	/**
	 * @param width The width of the map (not of this node!)
	 * @param height The height of the map (not of this node!)
	 **/
	void createChilds(unsigned int width, unsigned int height);

	bool intersects(int x1, int y1, int x2, int y2) const
	{
		if (x1 > right() || x2 < left()) {
			return false;
		}
		if (y1 > bottom() || y2 < top()) {
			return false;
		}
		return true;
	}

	bool contains(int x1, int y1, int x2, int y2) const
	{
		if (x1 >= left() && x2 <= right() && y1 >= top() && y2 <= bottom()) {
			return true;
		} else {
			return false;
		}
	}

	void getChildren(BoQuadTreeNode** nodes) const
	{
		nodes[0] = topLeftNode();
		nodes[1] = topRightNode();
		nodes[2] = bottomLeftNode();
		nodes[3] = bottomRightNode();
	}

protected:
	virtual BoQuadTreeNode* createNode(int l, int t, int r, int b, int depth) const;

private:
	// AB: note that we cannot easily save additional information such as
	// maxheight or texmap information here, because we use the tree in
	// editor mode as well, and there these information may change at any
	// time without letting the tree know.
	int mLeft;
	int mTop;
	int mRight;
	int mBottom;
	int mNodeSize;
	int mDepth;
	BoQuadTreeNode* mTopLeft;
	BoQuadTreeNode* mTopRight;
	BoQuadTreeNode* mBottomLeft;
	BoQuadTreeNode* mBottomRight;
};


#endif

