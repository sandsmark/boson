/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/utexteditui.hpp
    begin             : Wed Mar 26 2003
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

#ifndef UTEXTEDITUI_HPP
#define UTEXTEDITUI_HPP

#include "uwidgetui.hpp"

#include "../util/upoint.hpp"
#include "../util/urectangle.hpp"

namespace ufo {

class UTextEdit;

/**an interface for text widget UIs
  *@author Johannes Schmidt
  */

class UFO_EXPORT UTextEditUI : public UWidgetUI {
	UFO_DECLARE_DYNAMIC_CLASS(UTextEditUI)
public:
	/** @return The bounding rectangle of the current caret position in the
	  * 	coordinate system of the textWidget.
	  */
	virtual URectangle
	modelToView(const UTextEdit * textA, int offsA) const;

	/** @param posA The position in the coordinate system of the textWidget.
	  * @return The offset of the letter at the given position.
	  */
	virtual int
	viewToModel(const UTextEdit * textA, const UPoint & posA) const;
};

} // namespace ufo

#endif // UTEXTEDITUI_HPP
