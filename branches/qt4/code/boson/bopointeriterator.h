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
#ifndef BOPOINTERITERATOR_H
#define BOPOINTERITERATOR_H

/**
 * This iterator class operates on an array of pointers. The array must have n+1
 * elements, where n is the number of actual elements and the last element is
 * simply NULL. A NULL pointer in the array marks the last element. You can use
 * this class like this:
 *
 * <pre>
 * BosonWeapon** weapons = new BosonWeapon*[numberOfWeapons + 1];
 * for (int i = 0; i < numberOfWeapons; i++) {
 *     weapons[i] = new BosonWeapon(...);
 * }
 * weapons[numberOfWeapons] = 0; // last element
 * [...]
 * BoPointerIterator it(weapons);
 * for (; *it; ++it) {
 *     (*it)->shootAtSomething();
 * }
 * </pre>
 *
 * As you can see the loops exits once it reaches the NULL pointer, i.e. once
 * *it returns NULL.
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short An iterator that operates on an array of pointers
 **/
template<class T> class BoPointerIterator
{
public:
	/**
	 * Construct a NULL iterator. You should use the assignement operator to
	 * make this useful
	 **/
	BoPointerIterator()
	{
		mPointer = 0;
		mCurrentPointer = 0;
	}

	/**
	 * Construct an iterator that operates on @p pointer.
	 *
	 * @p pointer MUST be an array with n + 1 elements, where n is the
	 * number of actual pointers. The last element of the pointer array must
	 * be NULL.
	 **/
	BoPointerIterator(T** pointer)
	{
		mPointer = pointer;
		toFirst();
	}

	/**
	 * Move the iterator to the first element in the array.
	 * @return The first element in the array this iterator operates on.
	 **/
	T* toFirst()
	{
		mCurrentPointer = mPointer;
		return *mCurrentPointer;
	}

	/**
	 * @return The current element.
	 **/
	inline T* operator*()
	{
		return *mCurrentPointer;
	}

	/**
	 * Operate on @p pointer and make pointer[0] the current element.
	 **/
	BoPointerIterator& operator=(T** pointer)
	{
		mPointer = pointer;
		toFirst();
		return *this;
	}

	/**
	 * Go to the next element in the array. The iterator will be in
	 * undefined state if the current element is already the last element.
	 *
	 * Note that we don't provide the postfix operator, but the prefix
	 * operator only. I.e. you can it ++it, but not it++.
	 **/
	inline BoPointerIterator& operator++()
	{
		mCurrentPointer++;
		return *this;
	}

private:
	T** mPointer;
	T** mCurrentPointer;
};

#endif

