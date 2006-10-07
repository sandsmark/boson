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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ubodrawableicon.h"

#include <bogl.h>

#include <ufo/udrawable.hpp>
#include <ufo/util/udimension.hpp>

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UBoDrawableIcon, UIcon)

UBoDrawableIcon::UBoDrawableIcon(UDrawable * drawable)
	: UIcon()
	, m_drawable(drawable)
{
 if (m_drawable) {
	trackPointer(m_drawable);
 }
}

UBoDrawableIcon::~UBoDrawableIcon()
{
}

void UBoDrawableIcon::paintIcon(UGraphics * g, const URectangle & rect, const UStyleHints *, uint32_t)
{
 if (!m_drawable) {
	return;
 }
 UDimension size = rect.getSize();
 size.clamp(getIconSize());
 if (!size.isEmpty()) {
	m_drawable->paintDrawable(g, URectangle(rect.getLocation(), size));
 }
}

UDimension UBoDrawableIcon::getIconSize() const
{
 if (!m_drawable) {
	return UDimension(0, 0);
 }
 return m_drawable->getDrawableSize();
}

