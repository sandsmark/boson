/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2005 by Andreas Beckermann
    email             : b_mann at gmx.de
                             -------------------

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


#ifndef UBODRAWABLEICON_H
#define UBODRAWABLEICON_H

#include <ufo/uicon.hpp>

namespace ufo {

class UDrawable;

class UFO_EXPORT UBoDrawableIcon : public UIcon {
	UFO_DECLARE_DYNAMIC_CLASS(UBoDrawableIcon)
public:
	UBoDrawableIcon(UDrawable* drawable);
	~UBoDrawableIcon();

public: // implements UIcon
	virtual void paintIcon(UGraphics * g, const URectangle & rect, const UStyleHints * hints, uint32_t widgetState = 0);

	virtual UDimension getIconSize() const;

private:
	UDrawable * m_drawable;
};

} // namespace ufo

#endif

