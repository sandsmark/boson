/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uabstractslider.hpp
    begin             : Wed Mar 16 2005
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

#ifndef UABSTRACTSLIDER_HPP
#define UABSTRACTSLIDER_HPP

#include "uwidget.hpp"

namespace ufo {

class USliderModel;

/** @short An abstract slider.
  * @ingroup abstractwidgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UAbstractSlider : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UAbstractSlider)
public:
	/** Creates a slider with horizontal orientation, a minimum value of 0,
	  * a maximum of 99 and a value of 0.
	  */
	UAbstractSlider();

public: // Public methods
	int getMinimum() const;
	void setMinimum(int min);

	int getMaximum() const;
	void setMaximum(int max);

	void setRange(int min, int max);

	int getValue() const;
	void setValue(int newValue);

	/** The amount to scroll for one unit (e.g. mouse wheel events). */
	int getUnitIncrement() const;
	void setUnitIncrement(int inc);

	/** The amount to scroll for a block increment (e.g. page up/down).
	  */
	int getBlockIncrement() const;
	void setBlockIncrement(int inc);

public:
	USignal1<UAbstractSlider*> & sigValueChanged();

protected: // Protected methods
	/** For convenience */
	USliderModel * getSliderModel() const;
protected: // Overrides UWidget
	virtual void processMouseWheelEvent(UMouseWheelEvent * e);

private: // Private signals
	/**  */
	USignal1<UAbstractSlider*> m_sigValueChanged;
};

//
// inline implementation
//

inline USignal1<UAbstractSlider*> &
UAbstractSlider::sigValueChanged() {
	return m_sigValueChanged;
}

} // namespace ufo

#endif // UABSTRACTSLIDER_HPP
