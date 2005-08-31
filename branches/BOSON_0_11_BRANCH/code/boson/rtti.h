/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef RTTI_H
#define RTTI_H

/**
 * This class consists of a single enum @ref Rtti which defines the rtti values
 * used in Boson. Note that @ref VisualUnit has many different rtti values as
 * there are different units. I want to implement code-independent units (i.e.
 * they get defined in index.unit files on runtime) and therefore one
 * doesn't know the actual number of rttis used by @ref VisualUnit. So don't
 * add any rttis here after @ref UnitStart!
 *
 * Note that you have to recompile nearly everything if you change this file!
 * @short Boson rttis
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class RTTI
{
public:
	RTTI() { }
	~RTTI() { }

	enum Rtti {

		Shot = 150,

		UnitStart = 200 // the IDs of the units start at this value.
		                // Do not insert RTTIs after this!
	};

	static bool isUnit(int rtti)
	{
		return (rtti >= UnitStart);
	}

	static bool isShot(int rtti)
	{
		return (rtti == Shot);
	}
};

#endif
