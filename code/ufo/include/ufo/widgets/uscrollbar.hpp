/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef USCROLLBAR_HPP
#define USCROLLBAR_HPP

#include "uwidget.hpp"

// we need this for proper getUI() overriding
//#include "../ui/uscrollbarui.hpp"

namespace ufo {

/**
  *@author Johannes Schmidt
  */

class UFO_EXPORT UScrollBar : public UWidget  {
	UFO_DECLARE_DYNAMIC_CLASS(UScrollBar)
	UFO_UI_CLASS(UScrollBarUI)
public:
	UScrollBar(Orientation orientationA = Horizontal,
		int valueA = 0, int visAmountA = 10,
		int minA = 0, int maxA = 100);

	Orientation getOrientation() const;
/*
public: // hides | overrides UWidget
	virtual void setUI(UScrollBarUI * ui);
	virtual UWidgetUI * getUI() const;
	virtual void updateUI();
*/
public: // Public methods

	int getValue() const;
	void setValue(int newValueA);

	int getMaximum() const;
	void setMaximum(int maxA);

	int getMinimum() const;
	void setMinimum(int minA);

	/** The visible amount is the size of knob
	  */
	int getVisibleAmount() const;
	void setVisibleAmount(int amountA);

	/** The direction determines up or down scrolling. The return value
	  * my differ if the scroll knob is almost on top/bottom.
	  */
	virtual int getUnitIncrement(Direction directionA) const;
	virtual void setUnitIncrement(int incA);

	/** The amount to scroll for a block increment (e.g. page up/down).
	  * The direction determines up or down scrolling. The return value
	  * my differ if the scroll knob is almost on top/bottom.
	  */
	virtual int getBlockIncrement(Direction direction) const;
	virtual void setBlockIncremenet(int incA);

public: // Public signals
	USignal2<UScrollBar*, int> & sigValueChanged();
	/** Deprecated! */
	USignal2<UScrollBar*, int> & sigScrollPos();

protected: // Protected methods
	void processMouseWheelEvent(UMouseWheelEvent * e);

// private:
protected: // Protected attributes
	Orientation m_orientation;
	int m_value;
	int m_visAmount;
	int m_minValue;
	int m_maxValue;

	int m_unitIncrement;
	int m_blockIncrement;

	USignal2<UScrollBar*, int> m_sigValueChanged;
};

//
// inline implementation
//

inline USignal2<UScrollBar*, int> &
UScrollBar::sigValueChanged() {
	return m_sigValueChanged;
}

inline USignal2<UScrollBar*, int> &
UScrollBar::sigScrollPos() {
	return m_sigValueChanged;
}

} // namespace ufo

#endif // USCROLLBAR_HPP
