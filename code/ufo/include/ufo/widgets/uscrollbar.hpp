/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uscrollbar.hpp
    begin             : Wed Apr 17 2002
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

#ifndef USCROLLBAR_HPP
#define USCROLLBAR_HPP

#include "uabstractslider.hpp"

namespace ufo {

/** @short A scroll bar may be used to scroll the contents of a scroll pane.
  * @ingroup widgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UScrollBar : public UAbstractSlider  {
	UFO_DECLARE_DYNAMIC_CLASS(UScrollBar)
	UFO_UI_CLASS(UScrollBarUI)
	UFO_STYLE_TYPE(UStyle::CE_ScrollBar)
public:
	UScrollBar(Orientation orientationA = Horizontal,
		int valueA = 0, int visAmountA = 10,
		int minA = 0, int maxA = 100);

public: // Public methods

	/** The visible amount is the size of knob
	  */
	int getVisibleAmount() const;
	void setVisibleAmount(int amountA);

protected: // Overrides UWidget
	virtual UDimension getContentsSize(const UDimension & maxSize) const;
	virtual void processMouseEvent(UMouseEvent * e);

private: // Private attributes
	/** True if mouse is dragging the slider knob. */
	bool m_isDragging;
	/** The last mouse press on this slider. */
	UPoint m_mousePress;
	int m_visAmount;
};

} // namespace ufo

#endif // USCROLLBAR_HPP
