/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/basic/ubasicsliderui.hpp
    begin             : Fri Jul 30 2004
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


#ifndef UBASICSLIDERUI_HPP
#define UBASICSLIDERUI_HPP

#include "../usliderui.hpp"

#include "../../util/urectangle.hpp"

namespace ufo {

class USlider;
class UEvent;
class UMouseEvent;

/**
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBasicSliderUI : public USliderUI {
	UFO_DECLARE_DYNAMIC_CLASS(UBasicSliderUI)
public:
	UBasicSliderUI(USlider * slider);
public:
	static UBasicSliderUI * createUI(UWidget * w);

	void paint(UGraphics * g, UWidget * w);

	void installUI(UWidget * w);
	void uninstallUI(UWidget * w);

	const std::string & getLafId();

	UDimension getPreferredSize(const UWidget * w);

protected: // Protected Methods
	void valueChanged(USlider * slider, int newValue);
	void mousePressed(UMouseEvent * e);
	void mouseDragged(UMouseEvent * e);

private:  // Private attributes
	/**  */
	USlider * m_slider;
	URectangle m_rect;
	bool m_isDragging;
	UPoint m_mousePress;

private:  // Private static attributes
	/** The shared look and feel id */
	static std::string m_lafId;
};

} // namespace ufo

#endif // UBASICSLIDERUI_HPP
