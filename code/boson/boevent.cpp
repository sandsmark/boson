/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "boevent.h"

#include "boson.h"
#include "bodebug.h"
#include "boeventmatching.h"

#include <qdom.h>
#include <qptrlist.h>

BoEvent::BoEvent(const QCString& name)
{
 init(name);
}

BoEvent::BoEvent()
{
 init(QCString());
}

void BoEvent::init(const QCString& name)
{
 mName = name;
 mId = 0;
 mDelayedDelivery = 0;
 mHasLocation = false;
 mHasPlayerId = false;
 mPlayerId = 0;
 mUnitId = 0;
}

BoEvent::~BoEvent()
{
}

bool BoEvent::save(QDomElement& root) const
{
 root.setAttribute("RTTI", rtti());
 root.setAttribute("Name", name());
 root.setAttribute("Id", QString::number(id()));
 root.setAttribute("UnitId", QString::number(unitId()));
 root.setAttribute("DelayedDelivery", QString::number(delayedDelivery()));
 root.setAttribute("HasLocation", QString::number((int)hasLocation()));
 root.setAttribute("LocationX", QString::number(location().x()));
 root.setAttribute("LocationY", QString::number(location().y()));
 root.setAttribute("LocationZ", QString::number(location().z()));
 if (mHasPlayerId) {
	// just like in BosonCanvas, we save the _index_, not the ID. but load()
	// will expect the _ID_, so there must be some class filling in the
	// correct value before load() is called.
	KPlayer* p = boGame->findPlayer(playerId());
	if (!p) {
		BO_NULL_ERROR(p);
		return false;
	}
	int index = boGame->playerList()->findRef(p);
	root.setAttribute("PlayerId", QString::number(index));
 }
 return true;
}

bool BoEvent::load(const QDomElement& root)
{
 bool ok;
 mName = root.attribute("Name");
 mId = root.attribute("Id").toULong(&ok);
 if (!ok) {
	boError(360) << k_funcinfo << "Id is not a valid number: " << root.attribute("Id") << endl;
	return false;
 }
 if (root.hasAttribute("PlayerId")) {
	// note: we load the _ID_ here, although save() saves the _index_
	unsigned long int id = root.attribute("PlayerId").toULong(&ok);
	if (!ok) {
		boError(360) << k_funcinfo << "Invalid PlayerId" << endl;
		return false;
	}
	setPlayerId(id); // set mPlayerId and mHasPlayerId
 }
 mUnitId = root.attribute("UnitId").toULong(&ok);
 if (!ok) {
	boError(360) << k_funcinfo << "Invalid UnitId" << endl;
	return false;
 }
 mDelayedDelivery = root.attribute("DelayedDelivery").toULong(&ok);
 if (!ok) {
	boError(360) << k_funcinfo << "Invalid DelayedDelivery" << endl;
	return false;
 }
 mHasLocation = (bool)root.attribute("HasLocation").toInt(&ok);
 if (!ok) {
	boError(360) << k_funcinfo << "Invalid HasLocation" << endl;
	return false;
 }
 mLocation.setX((bool)root.attribute("LocationX").toFloat(&ok));
 if (!ok) {
	boError(360) << k_funcinfo << "Invalid LocationX" << endl;
	return false;
 }
 mLocation.setY((bool)root.attribute("LocationY").toFloat(&ok));
 if (!ok) {
	boError(360) << k_funcinfo << "Invalid LocationY" << endl;
	return false;
 }
 mLocation.setZ((bool)root.attribute("LocationZ").toFloat(&ok));
 if (!ok) {
	boError(360) << k_funcinfo << "Invalid LocationZ" << endl;
	return false;
 }
 return true;
}

bool BoEvent::matches(const BoEventMatching* m, const BoEvent* e) const
{
 if (rtti() != e->rtti()) {
	return false;
 }
 if (name() != e->name()) {
	return false;
 }
 if (!m->ignoreUnitId()) {
	if (unitId() != e->unitId()) {
		return false;
	}
 }
 if (!m->ignorePlayerId()) {
	if (mHasPlayerId != e->mHasPlayerId) {
		return false;
	}
	if (mHasPlayerId) {
		if (playerId() != e->playerId()) {
			return false;
		}
	}
 }
 // id(), location() and delayedDelivery() are not relevant here!
 return true;
}

BoEvent* BoEvent::createEvent(int rtti)
{
 BoEvent* event = 0;
 switch (rtti) {
	case BoEvent::RTTIEvent:
		event = new BoEvent();
		break;
	case BoEvent::RTTIULong:
		event = new BoGenericULongEvent();
		break;
	case BoEvent::RTTIString:
		event = new BoGenericStringEvent();
		break;
	default:
		boError(360) << k_funcinfo << "unhandled rtti " << rtti << endl;
		break;
 }
 return event;
}

BoGenericULongEvent::BoGenericULongEvent()
	: BoEvent()
{
 mData1 = 0;
 mData2 = 0;
}

BoGenericULongEvent::BoGenericULongEvent(const QCString& name, unsigned long int data1, unsigned long int data2)
	: BoEvent(name)
{
 mData1 = data1;
 mData2 = data2;
}

BoGenericULongEvent::~BoGenericULongEvent()
{
}

bool BoGenericULongEvent::save(QDomElement& root) const
{
 root.setAttribute("Data1", QString::number(mData1));
 root.setAttribute("Data2", QString::number(mData2));
 return BoEvent::save(root);
}

bool BoGenericULongEvent::load(const QDomElement& root)
{
 bool ok;
 mData1 = root.attribute("Data1").toULong(&ok);
 if (!ok) {
	boError(360) << k_funcinfo << "Invalid Data1" << endl;
	return false;
 }
 mData2 = root.attribute("Data2").toULong(&ok);
 if (!ok) {
	boError(360) << k_funcinfo << "Invalid Data2" << endl;
	return false;
 }
 return BoEvent::load(root);
}

bool BoGenericULongEvent::matches(const BoEventMatching* m, const BoEvent* e) const
{
 if (!BoEvent::matches(m, e)) {
	return false;
 }
 BoGenericULongEvent* ev = (BoGenericULongEvent*)e;
 if (!m->ignoreData1()) {
	if (data1() != ev->data1()) {
		return false;
	}
 }
 if (!m->ignoreData2()) {
	if (data2() != ev->data2()) {
		return false;
	}
 }
 return true;
}


BoGenericStringEvent::BoGenericStringEvent()
	: BoEvent()
{
}

BoGenericStringEvent::BoGenericStringEvent(const QCString& name, const QString& data1, const QString& data2)
	: BoEvent(name)
{
 mData1 = data1;
 mData2 = data2;
}

BoGenericStringEvent::~BoGenericStringEvent()
{
}

bool BoGenericStringEvent::save(QDomElement& root) const
{
 root.setAttribute("Data1", mData1);
 root.setAttribute("Data2", mData2);
 return BoEvent::save(root);
}

bool BoGenericStringEvent::load(const QDomElement& root)
{
 mData1 = root.attribute("Data1");
 mData2 = root.attribute("Data2");
 return BoEvent::load(root);
}

bool BoGenericStringEvent::matches(const BoEventMatching* m, const BoEvent* e) const
{
 if (!BoEvent::matches(m, e)) {
	return false;
 }
 BoGenericStringEvent* ev = (BoGenericStringEvent*)e;
 if (!m->ignoreData1()) {
	if (data1() != ev->data1()) {
		return false;
	}
 }
 if (!m->ignoreData2()) {
	if (data2() != ev->data2()) {
		return false;
	}
 }
 return true;
}

