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
#ifndef BOQUADTREECOLLECTION_H
#define BOQUADTREECOLLECTION_H

#include <qobject.h>

class BoQuadTreeNode;
template<class T> class QPtrList;

class BoQuadTreeCollectionPrivate;
/**
 * @short A collection of quad trees
 *
 * This class simply provides a way to register and unregister trees and stores
 * all registered trees in a list that can be accessed using @ref trees.
 *
 * The @ref BoQuadTreeCollectionManager class and the @ref
 * BoQuadTreeNode destructor make automatically sure that a deleted tree is
 * unregistered from all collections, so manual unregistering is not required.
 *
 * This class really only maintains a list of trees, it doesn't do anything
 * useful with it. For this you should subclass the class. For example a
 * subclass may call cellHeightChanged() on all registered trees, whenever the
 * collection is notified that the height of a cell on the map got changed.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoQuadTreeCollection : public QObject
{
	Q_OBJECT
public:
	BoQuadTreeCollection(QObject* parent);
	virtual ~BoQuadTreeCollection();

	void registerTree(BoQuadTreeNode* root);
	void unregisterTree(BoQuadTreeNode* root);

	const QPtrList<BoQuadTreeNode>& trees() const;

private:
	BoQuadTreeCollectionPrivate* d;
};

class BoQuadTreeCollectionManagerPrivate;
/**
 * @internal
 *
 * There is at most one instance of this class and it is globally accessible. It
 * keeps track of all @ref BoQuadTreeCollection objects and notifies them when a
 * tree gets deleted (i.e. unregisters the tree from those collections).
 *
 * For this the destructor of the tree calls @ref unregisterTree.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoQuadTreeCollectionManager : public QObject
{
	Q_OBJECT
public:
	BoQuadTreeCollectionManager();
	~BoQuadTreeCollectionManager();

	static BoQuadTreeCollectionManager* manager();

	/**
	 * Call @ref BoQuadTreeCollection::unregisterTree on all known
	 * collections.
	 **/
	void unregisterTree(BoQuadTreeNode* root);

	/**
	 * @internal
	 **/
	void registerCollection(BoQuadTreeCollection* collection);

	/**
	 * @internal
	 **/
	void unregisterCollection(BoQuadTreeCollection* collection);

private:
	static BoQuadTreeCollectionManager* mManager;
	BoQuadTreeCollectionManagerPrivate* d;
};

#endif

