/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/layouts/ulayoutmanager.hpp
    begin             : Sat May 19 2001
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

#ifndef ULAYOUTMANAGER_HPP
#define ULAYOUTMANAGER_HPP

#include "../uobject.hpp"

namespace ufo {

class UWidget;
class UDimension;

/**the abstract layout manager class
  *@author Johannes Schmidt
  */

class UFO_EXPORT ULayoutManager : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(ULayoutManager)
public:
	virtual UDimension
	getPreferredLayoutSize(const UWidget * parent) const = 0;

	virtual UDimension
	getMinimumLayoutSize(const UWidget * parent) const = 0;

	virtual void layoutContainer(const UWidget * parent) = 0;

	virtual int getLayoutHeightForWidth(const UWidget * parent, int w) const = 0;
};

} // namespace ufo

#endif // ULAYOUTMANAGER_HPP
