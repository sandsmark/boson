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

#include "boquadtreecollection.h"
#include "boquadtreecollection.moc"

#include "boquadtreenode.h"
#include "bodebug.h"

#include <k3staticdeleter.h>
//Added by qt3to4:
#include <Q3PtrList>

static K3StaticDeleter<BoQuadTreeCollectionManager> sd;
BoQuadTreeCollectionManager* BoQuadTreeCollectionManager::mManager = 0;

class BoQuadTreeCollectionPrivate
{
public:
	BoQuadTreeCollectionPrivate()
	{
	}
	Q3PtrList<BoQuadTreeNode> mTrees;
};

BoQuadTreeCollection::BoQuadTreeCollection(QObject* parent)
	: QObject(parent)
{
 d = new BoQuadTreeCollectionPrivate;

 BoQuadTreeCollectionManager::manager()->registerCollection(this);
}

BoQuadTreeCollection::~BoQuadTreeCollection()
{
 BoQuadTreeCollectionManager::manager()->unregisterCollection(this);

 delete d;
}

void BoQuadTreeCollection::registerTree(BoQuadTreeNode* node)
{
 BO_CHECK_NULL_RET(node);
 if (node->depth() != 0) {
	boError() << k_funcinfo << "can register root nodes only" << endl;
	return;
 }
 if (d->mTrees.contains(node)) {
	return;
 }
 d->mTrees.append(node);
}

void BoQuadTreeCollection::unregisterTree(BoQuadTreeNode* node)
{
 if (!node) {
	return;
 }
 d->mTrees.removeRef(node);
}

const Q3PtrList<BoQuadTreeNode>& BoQuadTreeCollection::trees() const
{
 return d->mTrees;
}


class BoQuadTreeCollectionManagerPrivate
{
public:
	Q3PtrList<BoQuadTreeCollection> mCollections;
};

BoQuadTreeCollectionManager::BoQuadTreeCollectionManager()
	: QObject(0)
{
 d = new BoQuadTreeCollectionManagerPrivate;
}

BoQuadTreeCollectionManager::~BoQuadTreeCollectionManager()
{
 delete d;
}

BoQuadTreeCollectionManager* BoQuadTreeCollectionManager::manager()
{
 if (!mManager) {
	sd.setObject(mManager, new BoQuadTreeCollectionManager());
 }
 return mManager;
}

void BoQuadTreeCollectionManager::registerCollection(BoQuadTreeCollection* collection)
{
 if (collection && !d->mCollections.contains(collection)) {
	d->mCollections.append(collection);
 }
}

void BoQuadTreeCollectionManager::unregisterCollection(BoQuadTreeCollection* collection)
{
 d->mCollections.removeRef(collection);
}

void BoQuadTreeCollectionManager::unregisterTree(BoQuadTreeNode* root)
{
 if (!root) {
	return;
 }
 for (Q3PtrListIterator<BoQuadTreeCollection> it(d->mCollections); it.current(); ++it) {
	it.current()->unregisterTree(root);
 }
}

