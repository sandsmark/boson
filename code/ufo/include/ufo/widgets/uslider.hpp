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

#include "uwidget.hpp"


namespace ufo {

/** A slider.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT USlider : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(USlider)
	UFO_UI_CLASS(USliderUI)
public:
	/** Creates a slider with horizontal orientation, a minimum value of 0,
	  * a maximum of 100 and a value of 0.
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
	Orientation getOrientation() const;
	//void setOrientation(Orientation orientation);

	int getMinimum() const;
	void setMinimum(int min);

	int getMaximum() const;
	void setMaximum(int max);

	int getValue() const;
	void setValue(int newValue);

	//int getMajorTickSpacing() const;
	/** Not yet implemented:
	  * Sets the spacing for major ticks.
	  * Major ticks are the distance between major tick marks.
	  * Only important for UI and when using snap to ticks.
	  */
	//void setMajorTickSpacing(int spacing);

	/** The amount to scroll for one unit (e.g. mouse wheel events). */
	int getUnitIncrement() const;
	void setUnitIncrement(int inc);

	/** The amount to scroll for a block increment (e.g. page up/down).
	  */
	int getBlockIncrement() const;
	void setBlockIncrement(int inc);

public:
	USignal2<USlider*, int> & sigValueChanged();

protected: // Protected methods
	void processMouseWheelEvent(UMouseWheelEvent * e);

private: // Private attributes
	/**  */
	Orientation m_orientation;
	int m_min;
	int m_max;
	int m_value;

	int m_unitIncrement;
	int m_blockIncrement;

	USignal2<USlider*, int> m_sigValueChanged;
};

//
// inline implementation
//

inline Orientation
USlider::getOrientation() const {
	return m_orientation;
}


inline int
USlider::getMinimum() const {
	return m_min;
}

inline int
USlider::getMaximum() const {
	return m_max;
}

inline USignal2<USlider*, int> &
USlider::sigValueChanged() {
	return m_sigValueChanged;
}

} // namespace ufo

#endif // USLIDER_HPP
