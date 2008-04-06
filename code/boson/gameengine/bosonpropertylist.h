/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// AB: note: the license is _L_GPL

#ifndef BOSONPROPERTYLIST_H
#define BOSONPROPERTYLIST_H

#include <kdebug.h>

#include "kgamemessage.h"
#include "kgameproperty.h"
#include "kgamepropertyhandler.h"

#include <QList>

#define POLICY_ERROR kError() << k_funcinfo << "policy() is not PolicyLocal. BosonPropertyList does not support this policy()!";

/**
 * A @ref QList container providing a @ref KGamePropertyBase interface.
 *
 * An object of this class must be registered to a @ref KGamePropertyHandler,
 * like every @ref KGameProperty object.
 *
 * This class is saved (see @ref save) and loaded (see @ref load) whenever the
 * @ref KGamePropertyHandler is saved and loaded. No additional save/load code
 * needs to be written.
 *
 * Every type used in this template must provide operators << and >> for
 * QDataStream.
 **/
template<class T>
class BosonPropertyList : public KGamePropertyBase
{
public:
	BosonPropertyList() : KGamePropertyBase()
	{
	}

	BosonPropertyList(const BosonPropertyList<T> &a) : KGamePropertyBase()
	{
		mList = a.mList;
	}

	inline int count() const
	{
		return mList.count();
	}

	inline bool isEmpty() const
	{
		return mList.isEmpty();
	}

	void append(const T& item)
	{
		if (!policy() != PolicyLocal) {
			POLICY_ERROR;
			return;
		}
		mList.append(item);
	}
	const T& first() const
	{
		return mList.first();
	}

	void removeFirst()
	{
		if (!policy() != PolicyLocal) {
			POLICY_ERROR;
			return;
		}
		mList.removeFirst();
	}
	void pop_front()
	{
		removeFirst();
	}

	void removeLast()
	{
		if (!policy() != PolicyLocal) {
			POLICY_ERROR;
			return;
		}
		mList.removeLast();
	}
	void pop_back()
	{
		removeLast();
	}

	void removeAll(const T& item)
	{
		mList.removeAll(item);
	}

	void clear()
	{
		if (!policy() != PolicyLocal) {
			POLICY_ERROR;
			return;
		}
		mList.clear();
		if (isEmittingSignal()) {
			emit emitSignal();
		}
	}

	bool contains(const T& item) const
	{
		return mList.contains(item);
	}

	operator QList<T>() const
	{
		return mList;
	}

	const T& operator[](int i) const
	{
		return mList[i];
	}
	T& operator[](int i)
	{
		return mList[i];
	}

	void load(QDataStream& s)
	{
		mList.clear();

		qint32 count;
		T data;
		s >> count;

		for (qint32 i = 0; i < count; i++) {
			s >> data;
			mList.append(data);
		}
		if (isEmittingSignal()) {
			emit emitSignal();
		}
	}

	void save(QDataStream &s)
	{
		s << (qint32)mList.count();
		for (int i = 0; i < mList.count(); i++) {
			s << mList[i];
		}
	}

private:
	QList<T> mList;
};

#endif
