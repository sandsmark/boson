/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#ifndef ULAYOUTMANAGER_HPP
#define ULAYOUTMANAGER_HPP

#include "../uobject.hpp"
#include "../util/udimension.hpp"

namespace ufo {

class UWidget;

/** @short The layout manager handles the size and position of child widgets
  *  of a container.
  * @author Johannes Schmidt
  */

class UFO_EXPORT ULayoutManager : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(ULayoutManager)
public:
	/** @return The preferred size of the container using the given
	  *  maximum dimension.
	  */
	virtual UDimension
	getPreferredLayoutSize(const UWidget * container,
		const UDimension & maxSize) const = 0;
	/** @convenience */
	virtual UDimension
	getPreferredLayoutSize(const UWidget * container) const {
		return getPreferredLayoutSize(container, UDimension::maxDimension);
	}

	/** Relayouts all child widgets within the given container.
	  */
	virtual void layoutContainer(const UWidget * container) = 0;

public: // deprecated
	/** @deprecated */
	virtual UDimension
	getMinimumLayoutSize(const UWidget * /* container */) const { return UDimension(); }
};

} // namespace ufo

#endif // ULAYOUTMANAGER_HPP
