/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/uwidgetui.hpp
    begin             : Wed May 16 2001
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

#ifndef UWIDGETUI_HPP
#define UWIDGETUI_HPP

#include "../uobject.hpp"
#include "../util/udimension.hpp"
#include "../util/uinsets.hpp"

namespace ufo {

class UWidget;
class UGraphics;

/** base class for all widget user interfaces.
  * handels graphics and some event stuff.
  * Normally a static shared UI object is used. But if the UI object
  * handles listener, every widget should have its own UI object.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UWidgetUI : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UWidgetUI)

public:
	/** @return the shared widget ui object */
	static UWidgetUI * createUI(UWidget * w);
	/** */
	virtual void installUI(UWidget * w);
	/** uninstalls the widgetui */
	virtual void uninstallUI(UWidget * w);

	/** paints the widget */
	virtual void paint(UGraphics * g, UWidget * w);
	/** paints the border of the widget. */
	virtual void paintBorder(UGraphics * g, UWidget * w);
	virtual UInsets getBorderInsets(UWidget * w);

	virtual const std::string & getLafId();

	/** returns the size which the widget prefers to have */
	virtual UDimension getPreferredSize(const UWidget * w);
	/** returns the size which the widget must have in minimum */
	virtual UDimension getMinimumSize(const UWidget * w);
	/** returns the size which the widget prefers to have */
	virtual UDimension getMaximumSize(const UWidget * w);

	virtual int getHeightForWidth(const UWidget * w, int width);

private:  // Private attributes
	/** the shared widget ui object */
	static UWidgetUI * m_widgetUI;

	/** The shared look and feel id */
	static std::string m_lafId;
};

} // namespace ufo

#endif // UWIDGETUI_HPP
