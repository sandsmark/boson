/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/ulookandfeel.hpp
    begin             : Fri May 18 2001
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef ULOOKANDFEEL_HPP
#define ULOOKANDFEEL_HPP

#include "../uobject.hpp"

#include "uuidefs.hpp"

namespace ufo {

class UStyle;

/**
  * @author Johannes Schmidt
  */

class UFO_EXPORT ULookAndFeel : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(ULookAndFeel)
public:
	/** A hash map which has saved some default functions to retrieve ui
	  * classes.
	  */
	virtual UUIMap getDefaults() = 0;

	virtual UThemeMap getThemeDefaults() = 0;

	virtual UStyle * getStyle() = 0;

	/** Reload possibly lost data during recreation of GLX context.
	  */
	virtual void refresh() = 0;

	virtual std::string getName() = 0;
};

} // namespace ufo

#endif // ULOOKANDFEEL_HPP
