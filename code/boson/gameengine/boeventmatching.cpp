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

#include "boeventmatching.h"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "boevent.h"

#include <qdom.h>

BoEventMatching::BoEventMatching()
{
 mIgnoreUnitId = false;
 mIgnorePlayerId = false;
 mIgnoreData1 = false;
 mIgnoreData2 = false;
 mEvent = 0;
}

BoEventMatching::~BoEventMatching()
{
 delete mEvent;
}

bool BoEventMatching::saveAsXML(QDomElement& root) const
{
 if (!mEvent) {
	boError(360) << k_funcinfo << "cannot save a matching with NULL event" << endl;
	return false;
 }
 QDomDocument doc = root.ownerDocument();
 QDomElement e = doc.createElement("Event");
 if (!mEvent->saveAsXML(e)) {
	boError(360) << k_funcinfo << "cannot save event" << endl;
	return false;
 }
 root.appendChild(e);
 root.setAttribute("IgnoreUnitId", ignoreUnitId());
 root.setAttribute("IgnorePlayerId", ignorePlayerId());
 root.setAttribute("IgnoreData1", ignoreData1());
 root.setAttribute("IgnoreData2", ignoreData2());
 return true;
}

bool BoEventMatching::loadFromXML(const QDomElement& root)
{
 if (mEvent) {
	boError(360) << k_funcinfo << "event not NULL" << endl;
	return false;
 }
 QDomElement e = root.namedItem("Event").toElement();
 if (e.isNull()) {
	boError(360) << k_funcinfo << "no Event tag" << endl;
	return false;
 }
 mEvent = new BoEvent();
 if (!mEvent->loadFromXML(e)) {
	boError(360) << k_funcinfo << "could not load event" << endl;
	return false;
 }

 bool ok;
 // this macro just makes the code more readable. instead of 
 // mIgnoreUnitId = root.attribute("UnitId").toInt(&ok); if (!ok) { ... }
 // we just write LOAD_IGNORE(UnitId).
#define LOAD_IGNORE(x) mIgnore##x = root.attribute("Ignore" #x).toInt(&ok); \
		if (!ok) { boError(360) << k_funcinfo << "not a valid number for " "Ignore" #x << endl; return false; }
 LOAD_IGNORE(UnitId)
 LOAD_IGNORE(PlayerId)
 LOAD_IGNORE(Data1)
 LOAD_IGNORE(Data2)
#undef LOAD_IGNORE
 return true;
}

bool BoEventMatching::matches(const BoEvent* e) const
{
 if (!mEvent) {
	BO_NULL_ERROR(mEvent);
	return false;
 }
 return mEvent->matches(this, e);
}

