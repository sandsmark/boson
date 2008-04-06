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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "boevent.h"

#include "../../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "boeventmatching.h"

#include <qdom.h>
#include <q3ptrlist.h>
#include <qmap.h>
//Added by qt3to4:
#include <Q3CString>

BoEvent::BoEvent(const Q3CString& name, const QString& data1, const QString& data2)
{
 init(name);
 mData1 = data1;
 mData2 = data2;
}

BoEvent::BoEvent()
{
 init(Q3CString());
}

void BoEvent::init(const Q3CString& name)
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

bool BoEvent::saveAsXML(QDomElement& root) const
{
 root.setAttribute("Name", name());
 root.setAttribute("Id", QString::number(id()));
 root.setAttribute("UnitId", QString::number(unitId()));
 root.setAttribute("DelayedDelivery", QString::number(delayedDelivery()));
 root.setAttribute("HasLocation", QString::number((int)hasLocation()));
 root.setAttribute("Data1", mData1);
 root.setAttribute("Data2", mData2);
 if (!saveVector3AsXML(location(), root, "Location")) {
	boError(360) << k_funcinfo << endl;
	return false;
 }
 if (mHasPlayerId) {
	root.setAttribute("PlayerId", QString::number(playerId()));
 }
 return true;
}

bool BoEvent::loadFromXML(const QDomElement& root)
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
	quint32 id = root.attribute("PlayerId").toULong(&ok);
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
 if (!loadVector3FromXML(&mLocation, root, "Location")) {
	boError(360) << k_funcinfo << "unable to load Location" << endl;
	return false;
 }
 mData1 = root.attribute("Data1");
 mData2 = root.attribute("Data2");
 return true;
}

bool BoEvent::matches(const BoEventMatching* m, const BoEvent* e) const
{
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
 if (!m->ignoreData1()) {
	if (data1() != e->data1()) {
		return false;
	}
 }
 if (!m->ignoreData2()) {
	if (data2() != e->data2()) {
		return false;
	}
 }
 // id(), location() and delayedDelivery() are not relevant here!
 return true;
}

