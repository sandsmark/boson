/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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

#include "boglobal.h"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"

#include <kstaticdeleter.h>

#include <qptrlist.h>
#include <qintdict.h>

static KStaticDeleter<BoGlobal> sd;

BoGlobal* BoGlobal::mBoGlobal = 0;

class BoGlobalPrivate
{
public:
	BoGlobalPrivate()
	{
	}
	QPtrList<BoGlobalObjectBase> mObjects;

	// used by setPointer() and pointer().
	QIntDict<void> mPointersById;
};

BoGlobal::BoGlobal()
{
 d = new BoGlobalPrivate;
 mBoInfo = 0;
 mBosonConfig = 0;
 mBosonProfiling = 0;
 mBosonData = 0;
 mBosonAudio = 0;
 mBoItemListHandler = 0;
}

BoGlobal::~BoGlobal()
{
 delete d;
}

void BoGlobal::initStatic()
{
 if (mBoGlobal) {
	return;
 }
 sd.setObject(mBoGlobal, new BoGlobal());
}

void BoGlobal::registerObject(BoGlobalObjectBase* o, bool initFirst)
{
 if (!o) {
	return;
 }
 if (initFirst) {
	d->mObjects.prepend(o);
 } else {
	d->mObjects.append(o);
 }
}

void BoGlobal::unregisterObject(BoGlobalObjectBase* o)
{
 // do NOT delete! o is created on the stack.
 d->mObjects.removeRef(o);
}

void BoGlobal::initGlobalObjects()
{
 static bool initialized = false;
 if (initialized) {
	boWarning() << k_funcinfo << "already called" << endl;
	return;
 }
 initialized = true;

 // we copy the list, so that objects in the list are allowed to call
 // register/unregisterObject() here
 QPtrList<BoGlobalObjectBase> objects = d->mObjects;
 QPtrListIterator<BoGlobalObjectBase> it(objects);
 for (; it.current(); ++it) {
	it.current()->loadObject();
 }
}

void BoGlobal::destroyGlobalObjects()
{
 // remove in reverse order
 BoGlobalObjectBase* o = 0;
 for (o = d->mObjects.last(); o; o = d->mObjects.prev()) {
	o->deleteObject();
 }
}

void BoGlobal::setPointer(void* pointer, int id)
{
 // AB: if anyone can imagine a nice way to do this without a switch... let me
 // know!
 switch ((BoGlobalObjectBase::ObjectType)id) {
	case BoGlobalObjectBase::BoGlobalConfig:
		setBosonConfig((BosonConfig*)pointer);
		break;
	case BoGlobalObjectBase::BoGlobalInfo:
		setBoInfo((BoInfo*)pointer);
		break;
	case BoGlobalObjectBase::BoGlobalProfiling:
		setBosonProfiling((BosonProfiling*)pointer);
		break;
	case BoGlobalObjectBase::BoGlobalData:
		setBosonData((BosonData*)pointer);
		break;
	case BoGlobalObjectBase::BoGlobalItemListHandler:
		setBoItemListHandler((BoItemListHandler*)pointer);
		break;
	case BoGlobalObjectBase::BoGlobalAudio:
		setBosonAudio((BosonAudioInterface*)pointer);
		break;
	default:
		break;
 }
 if (pointer) {
	if (d->mPointersById[id] != 0) {
		d->mPointersById.insert(id, pointer);
	}
 } else {
	d->mPointersById.remove(id);
 }
}

void* BoGlobal::pointer(int id) const
{
 return d->mPointersById[id];
}

BoGlobalObjectBase::BoGlobalObjectBase(bool initFirst)
{
 BoGlobal::initStatic();
 BoGlobal::boGlobal()->registerObject(this, initFirst);
}

BoGlobalObjectBase::~BoGlobalObjectBase()
{
 if (BoGlobal::boGlobal()) { // set to NULL by KStaticDeleter when it has been deleted!
	BoGlobal::boGlobal()->unregisterObject(this);
 }
}


