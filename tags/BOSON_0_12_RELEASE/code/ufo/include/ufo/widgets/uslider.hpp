/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uslider.hpp
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


#ifndef USLIDER_HPP
#define USLIDER_HPP

#include "uabstractslider.hpp"


namespace ufo {

class USliderModel;

/** @short A vertical or horizontal slider.
  * @ingroup widgets
  *
  * A slider is used to control a bounded value.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT USlider : public UAbstractSlider {
	UFO_DECLARE_DYNAMIC_CLASS(USlider)
	UFO_UI_CLASS(USliderUI)
	UFO_STYLE_TYPE(UStyle::CE_Slider)
public:
	/** Creates a slider with horizontal orientation, a minimum value of 0,
	  * a maximum of 99 and a value of 0.
	  */
	USlider();
	/** Creates a slider with the given orientation, a minimum value of 0,
	  * a maximum of 100 and a value of 0.
	  */
	USlider(Orientation orientation);
	/** Creates a slider with horizontal orientation and a value of 0.
	  * @param min The minimum value
	  * @param max The maximum value
	  * @param value The initial value
	  */
	USlider(int min, int max, int value = 0);
	/** Creates a slider with the given values.
	  * @param orientation The visible orientation (horizontal or vertival).
	  * @param min The minimum value
	  * @param max The maximum value
	  * @param value The initial value
	  */
	USlider(Orientation orientation, int min, int max, int value);

public: // Public methods

protected: // Overrides UWidget
	virtual UDimension getContentsSize(const UDimension & maxSize) const;
	virtual void processMouseEvent(UMouseEvent * e);

private: // Private attributes
	/** True if mouse is dragging the slider knob. */
	bool m_isDragging;
	/** The last mouse press on this slider. */
	UPoint m_mousePress;
};

} // namespace ufo

#endif // USLIDER_HPP
