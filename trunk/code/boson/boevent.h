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
#ifndef BOEVENT_H
#define BOEVENT_H

#include "bo3dtools.h"

#include <qstring.h>

class QDomElement;
class QDomNodeList;
class BoEventMatching;

// AB: we use QCString for performance reasons here. all event names will always
// be english names with only ascii chracters. QCString uses qstrcmp() and
// therefore strcmp() for comparisons, which works very well together with pure
// const char* strings.
class BoEvent
{
public:
	enum RTTIs {
		RTTIEvent = 1,
		RTTIULong = 2,
		RTTIString = 3
	};
public:
	/**
	 * Construct a new event.
	 *
	 * Note that this event is invalid until you call @ref
	 * BoEventManager::queueEvent.
	 **/
	BoEvent(const QCString& name);

	/**
	 * @overload
	 * Used for loading only
	 **/
	BoEvent();

	virtual ~BoEvent();

	/**
	 * @return A @ref BoEvent object for @p rtti. The object has no name, so
	 * it can be used for loading events only!
	 **/
	static BoEvent* createEvent(int rtti);

	virtual bool save(QDomElement& root) const;
	virtual bool load(const QDomElement& root);

	/**
	 * @return TRUE if this event matches the event @p e under the matching
	 * @p m. The matching @p m describes which parameters are relevant when
	 * comparing the events.
	 **/
	virtual bool matches(const BoEventMatching* m, const BoEvent* e) const;

	/**
	 * @return The RTTI of the event class.
	 **/
	virtual int rtti() const
	{
		return RTTIEvent;
	}

	void setPlayerId(unsigned long int playerId)
	{
		mHasPlayerId = true;
		mPlayerId = playerId;
	}
	unsigned long int playerId() const
	{
		return mPlayerId;
	}
	void setUnitId(unsigned long int unitId)
	{
		mUnitId = unitId;
	}
	unsigned long int unitId() const
	{
		return mUnitId;
	}

	/**
	 * Make @ref hasLocation return TRUE and set the location. See @ref
	 * hasLocation and @ref location.
	 * @param pos The location in canvas coordinates
	 **/
	void setLocation(const BoVector3 pos)
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
	const BoVector3& location() const
	{
		return mLocation;
	}

	/**
	 * Deliver after @p advanceCalls advance calls.
	 *
	 * atm 20 advance calls are ~1 second (at default game speed)
	 **/
	void setDelayedDelivery(unsigned long int advanceCalls)
	{
		mDelayedDelivery = advanceCalls;
	}

	unsigned long int delayedDelivery() const
	{
		return mDelayedDelivery;
	}

	/**
	 * Called by @ref BoEventManager::queueEvent only.
	 **/
	void setId(unsigned long int id)
	{
		mId = id;
	}

	/**
	 * The ID is supposed to be used in debugging only at the moment.
	 * @return The id of the event.
	 **/
	unsigned long int id() const
	{
		return mId;
	}

	QCString name() const
	{
		return mName;
	}


private:
	void init(const QCString& name);

private:
	unsigned long int mId;
	QCString mName;
	unsigned long int mDelayedDelivery;
	bool mHasLocation;
	BoVector3 mLocation;
	unsigned long int mUnitId;
	bool mHasPlayerId;
	unsigned long int mPlayerId;
};

/**
 * Generic event that stores up to 4 ULong values.
 **/
class BoGenericULongEvent : public BoEvent
{
public:
	BoGenericULongEvent(const QCString& name, unsigned long int data1 = 0, unsigned long int data2 = 0);

	/**
	 * @overload
	 * Used for loading only
	 **/
	BoGenericULongEvent();

	~BoGenericULongEvent();

	virtual int rtti() const
	{
		return RTTIULong;
	}

	unsigned long int data1() const
	{
		return mData1;
	}
	unsigned long int data2() const
	{
		return mData2;
	}

	virtual bool save(QDomElement& root) const;
	virtual bool load(const QDomElement& root);
	virtual bool matches(const BoEventMatching* m, const BoEvent* e) const;

private:
	unsigned long int mData1;
	unsigned long int mData2;
};

/**
 * This event may be in particular useful for script-only use: scripts might use
 * a generic name , e.g. "ScriptEvent", and emit this event, for delivery to
 * scripts only. Then a (maybe different) script processes the event, according
 * to the string data.
 **/
class BoGenericStringEvent : public BoEvent
{
public:
	BoGenericStringEvent(const QCString& name, const QString& data1 = QString::null, const QString& data2 = QString::null);

	/**
	 * @overload
	 * Used for loading only
	 **/
	BoGenericStringEvent();

	~BoGenericStringEvent();

	virtual int rtti() const
	{
		return RTTIString;
	}

	const QString& data1() const
	{
		return mData1;
	}
	const QString& data2() const
	{
		return mData2;
	}

	virtual bool save(QDomElement& root) const;
	virtual bool load(const QDomElement& root);
	virtual bool matches(const BoEventMatching* m, const BoEvent* e) const;

private:
	QString mData1;
	QString mData2;

};

#endif

