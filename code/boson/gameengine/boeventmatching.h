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
#ifndef BOEVENTMATCHING_H
#define BOEVENTMATCHING_H

class Boson;
class BoEvent;
class QCString;
class QDomElement;
template<class T1, class T2> class QMap;

class BoEventMatching
{
public:
	BoEventMatching();
	~BoEventMatching();

	const BoEvent* event() const
	{
		return mEvent;
	}

	bool ignoreUnitId() const
	{
		return mIgnoreUnitId;
	}

	bool ignorePlayerId() const
	{
		return mIgnorePlayerId;
	}

	bool ignoreData1() const
	{
		return mIgnoreData1;
	}

	bool ignoreData2() const
	{
		return mIgnoreData2;
	}

	bool saveAsXML(QDomElement& root) const;
	bool loadFromXML(const QDomElement& root);

	bool matches(const BoEvent* event) const;

private:
	// AB: the name and the rtti of an event are never ignored
	// same for the location, which is not relevant for matching
	bool mIgnoreUnitId;
	bool mIgnorePlayerId;
	bool mIgnoreData1;
	bool mIgnoreData2;
	BoEvent* mEvent;
};

#endif

