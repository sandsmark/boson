/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/gl/ubasicstyle.hpp
    begin             : Sat Mar 5 2005
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

#ifndef UBASICSTYLE_HPP
#define UBASICSTYLE_HPP

#include "../ui/ustyle.hpp"

namespace ufo {

/** @short Default implementation of a UFO style.
  * @ingroup internal
  * @ingroup opengl
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UBasicStyle : public UStyle {
public:
	UBasicStyle();
	virtual void paintComponent(UGraphics * g,
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w = NULL);

	virtual void paintPrimitive(UGraphics * g,
		PrimitiveElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		uint32_t widgetState);

	virtual void paintBorder(UGraphics * g,
		uint32_t borderType,
		const URectangle & rect,
		const UStyleHints * hints,
		uint32_t widgetState);

	virtual UInsets getBorderInsets(
		ComponentElement elem,
		const UStyleHints * hints);

	virtual UInsets getInsets(
		ComponentElement elem,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w = NULL);

	virtual SubControls getSubControlAt(
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		const UPoint & pos,
		UWidget * w = NULL);

	virtual URectangle getSubControlBounds(
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		SubControls subElem,
		UWidget * w = NULL);

	virtual UDimension getSizeFromContents(
		ComponentElement elem,
		const UDimension & contentsSize,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w = NULL);

	virtual void paintCompound(
		UGraphics * g,
		const UStyleHints * hints,
		const std::string & text,
		UIcon * icon,
		const URectangle & rect,
		uint32_t widgetState,
		int acceleratorIndex = -1);

	virtual UDimension getCompoundPreferredSize(
		const UStyleHints * hints,
		const std::string & text,
		UIcon * icon);

	virtual void layoutCompound(
		const UStyleHints * hints,
		const std::string & text,
		UIcon * icon,
		const URectangle & viewRect,
		URectangle * textRect,
		URectangle * iconRect);

	virtual void install(UWidget * w);
	virtual void uninstall(UWidget * w);
};

} // namespace ufo

#endif // UBASICSTYLE_HPP
