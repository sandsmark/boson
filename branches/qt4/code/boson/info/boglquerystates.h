/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann <b_mann@gmx.de>

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
#ifndef BOGLQUERYSTATES_H
#define BOGLQUERYSTATES_H

class QStringList;
class QListView;

class BoGLQueryStatesPrivate;
class BoGLQueryStates
{
public:
	BoGLQueryStates();
	virtual ~BoGLQueryStates();

	/**
	 * Initialize this object and retrieve implementation dependant values
	 * from OpenGL (such as e.g. GL_MAX_TEXTURE_SIZE).
	 *
	 * This won't retrieve values that can be changed using OpenGL commands
	 * (it won't retrieve states). See @ref getStates for this.
	 **/
	void init();
	
	void getStates();

	/**
	 * @return A list of all values that are implementation dependant (but
	 * cannot be changed using OpenGL commands). The list contains strings
	 * that look like "GL_MAX_TEXTURE_SIZE = 1024"
	 **/
	QStringList implementationValueList() const;

	/*
	 * This will call @ref getStates first, i.e. will give you a current set
	 * of states.
	 * Don't use it in rendering code (except for debugging), as it is slow.
	 * @return A list of all states, including those values, that can't be
	 * changed using OpenGL commands (see @ref init).
	 **/
	QStringList stateList();

	/**
	 * This will NOT call @ref getStates. If @ref getStates has not yet been
	 * called, the results are undefined.
	 * @return The GL values that were current when @ref getStates was
	 * called last time.
	 **/
	QStringList oldStateList() const;

	/**
	 * @return Same as @ref oldStateList, but this contains the values of
	 * keys that are used with glEnable() only.
	 **/
	QStringList oldStateEnabledList() const;

	/**
	 * This compares two lists that come from two @ref stateList calls and
	 * returns a list containing the differences of these lists.
	 *
	 * If one list contains a key entry that is not in the other list, that
	 * entry is ignored. Only differences of values are returned.
	 * @param _l1 A list that is expected to be from @ref stateList. An
	 * entry that does not match the expected format causes a warning.
	 * @param _l2 See @p _l1
	 **/
	static QStringList getDifferences(const QStringList& _l1, const QStringList& _l2);

private:
	BoGLQueryStatesPrivate* d;
};

#endif
