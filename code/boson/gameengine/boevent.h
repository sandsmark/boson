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
#ifndef BOEVENT_H
#define BOEVENT_H

#include "../bo3dtools.h"

#include <qstring.h>

class QDomElement;
class QDomNodeList;
class BoEventMatching;
class Boson;
template <class T1, class T2> class QMap;

// AB: we use QCString for performance reasons here. all event names will always
// be english names with only ascii chracters. QCString uses qstrcmp() and
// therefore strcmp() for comparisons, which works very well together with pure
// const char* strings.
class BoEvent
{
public:
	/**
	 * Construct a new event.
	 *
	 * Note that this event is invalid until you call @ref
	 * BoEventManager::queueEvent.
	 * @param _name The name of this event. See @ref name
	 * @param _data1 Am (optional) parameter to this event. You can use
	 * QString only, but you can encode nearly every value into a @ref
	 * QString easily (e.g. @ref QString::number for ints). Note that the
	 * type of the parameters should depend on the @p _name only. See also
	 * @ref data1
	 * @param _data2 Just like @p _data1.
	 **/
	BoEvent(const QString& _name, const QString& _data1 = QString::null, const QString& _data2 = QString::null);

	/**
	 * @overload
	 * Used for loading only. You cannot use this before @ref load was
	 * called, as a name is missing.
	 **/
	BoEvent();

	virtual ~BoEvent();

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	/**
	 * @return TRUE if this event matches the event @p e under the matching
	 * @p m. The matching @p m describes which parameters are relevant when
	 * comparing the events.
	 **/
	virtual bool matches(const BoEventMatching* m, const BoEvent* e) const;

	void setPlayerId(quint32 playerId)
	{
		mHasPlayerId = true;
		mPlayerId = playerId;
	}
	quint32 playerId() const
	{
		return mPlayerId;
	}
	void setUnitId(quint32 unitId)
	{
		mUnitId = unitId;
	}
	quint32 unitId() const
	{
		return mUnitId;
	}

	/**
	 * Make @ref hasLocation return TRUE and set the location. See @ref
	 * hasLocation and @ref location.
	 * @param pos The location in canvas coordinates
	 **/
	void setLocation(const BoVector3Fixed pos)
	{
		mHasLocation = true;
		mLocation = pos;
	}

	/**
	 * @return Whether this event is fixed to a @ref location. This can be
	 * used to find out whether a player can "see" the event. If he can't
	 * the event is supposed not to be delivered to that player.
	 **/
	bool hasLocation() const
	{
		return mHasLocation;
	}

	/**
	 * The location of an event can (and should) be used to find out if a
	 * player can "see" the event. If the event has a location (see @ref
	 * hasLocation) but cannot see that point (i.e. it is fogged), then the
	 * player doesn't learn about the event.
	 *
	 * @return The location where the event has been raised. This only valid
	 * if @ref hasLocation is TRUE. The location is in canvas coordinates.
	 **/
	const BoVector3Fixed& location() const
	{
		return mLocation;
	}

	/**
	 * Deliver after @p advanceCalls advance calls.
	 *
	 * atm 20 advance calls are ~1 second (at default game speed)
	 **/
	void setDelayedDelivery(quint32 advanceCalls)
	{
		mDelayedDelivery = advanceCalls;
	}

	quint32 delayedDelivery() const
	{
		return mDelayedDelivery;
	}

	/**
	 * Called by @ref BoEventManager::queueEvent only.
	 **/
	void setId(quint32 id)
	{
		mId = id;
	}

	/**
	 * The ID is supposed to be used in debugging only at the moment.
	 * @return The id of the event.
	 **/
	quint32 id() const
	{
		return mId;
	}

	/**
	 * @return The name of the event, as specified in the constructor. The
	 * name of an event uniquely identifies the type of the event, including
	 * the types of the expected parameters (see especially @ref data1, @ref
	 * data2).
	 **/
	QString name() const
	{
		return mName;
	}

	/**
	 * @return Optional parameter 1. The value of this depends completely on
	 * the event. Default is @ref QString::null
	 **/
	const QString& data1() const
	{
		return mData1;
	}
	/**
	 * @return Optional parameter 2. The value of this depends completely on
	 * the event. Default is @ref QString::null
	 **/
	const QString& data2() const
	{
		return mData2;
	}


private:
	void init(const QString& name);

private:
	quint32 mId;
	QString mName;
	quint32 mDelayedDelivery;
	bool mHasLocation;
	BoVector3Fixed mLocation;
	quint32 mUnitId;
	bool mHasPlayerId;
	quint32 mPlayerId;

	QString mData1;
	QString mData2;
};

#endif

