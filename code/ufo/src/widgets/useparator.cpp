/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/useparator.cpp
    begin             : Fri Aug 10 2001
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

#include "ufo/widgets/useparator.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(USeparator, UWidget)

USeparator::USeparator(Orientation orientation)
{
	if (orientation != Horizontal) {
		setOrientation(orientation);
	}
}

UDimension
USeparator::getContentsSize(const UDimension & maxSize) const {
	Orientation orient = getOrientation();
	if (orient == NoOrientation) {
		if (getParent()) {
			if (getParent()->getOrientation() == Horizontal) {
				orient = Vertical;
			} else {
				orient = Horizontal;
			}
		}
	}
	if (orient == Vertical) {
		return UDimension(2, 0);
	} else {
		return UDimension(0, 2);
	}
}
