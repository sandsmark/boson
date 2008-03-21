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
#ifndef BOTOOLTIPCREATOR_H
#define BOTOOLTIPCREATOR_H

class BosonItem;
class QString;

/**
 * This is the base class of all tooltip creators in boson. Currently only
 * creators for OpenGL widgets are implemented, we may add more for e.g. the
 * commandframe one day.
 *
 * The tooltip creators are responsible for creating actual tooltips. You can
 * give it e.g. a @ref BosonItem object in @ref createToolTip and the creator
 * class will analyze the object and create a tooltip that can be displayed.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoToolTipCreator
{
public:
	enum TipCreators {
		Basic = 0,
		Extended = 1,
		Debug = 2
	};
public:
	BoToolTipCreator() {}
	virtual ~BoToolTipCreator() {}

	/**
	 * @return TRUE if the class can create a tooltip for the OpenGL display
	 * (i.e. for items on the @ref BosonBigDisplayBase widget). Must be
	 * implemented.
	 **/
	virtual bool canCreateGLDisplayToolTip() const = 0;

	/**
	 * Note that @p item can also be NULL!
	 * @return A tooltip for an item on the @ref BosonBigDisplayBase widget.
	 * See also @ref canCreateGLDisplayToolTip. By default this returns @ref
	 * QString::null
	 **/
	virtual QString createToolTip(const BosonItem* item) const;

	// AB: we can add more, e.g. tooltips for the commandframe here!
};

/**
 * A very basic implementation of @ref BoToolTipCreator that displays the name
 * of a unit only.
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short Basic tooltips with unit names only.
 **/
class BoToolTipCreatorBasic : public BoToolTipCreator
{
public:
	BoToolTipCreatorBasic() : BoToolTipCreator() {}

	virtual bool canCreateGLDisplayToolTip() const { return true; }

	virtual QString createToolTip(const BosonItem* item) const;
};

/**
 * Extended tooltips, that display name and a few additional values (e.g. health
 * of units). This class is the default creator.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoToolTipCreatorExtended : public BoToolTipCreator
{
public:
	BoToolTipCreatorExtended() : BoToolTipCreator() {}

	virtual bool canCreateGLDisplayToolTip() const { return true; }

	virtual QString createToolTip(const BosonItem* item) const;
};

/**
 * Debugging tooltips. These tooltips can be very long, they should contain as
 * much data as necessary.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoToolTipCreatorDebug : public BoToolTipCreator
{
public:
	BoToolTipCreatorDebug() : BoToolTipCreator() {}

	virtual bool canCreateGLDisplayToolTip() const { return true; }

	virtual QString createToolTip(const BosonItem* item) const;
};

#endif

