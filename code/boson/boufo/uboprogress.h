/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2004 by Andreas Beckermann
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


#ifndef UBOPROGRESS_H
#define UBOPROGRESS_H

#include <ufo/widgets/uwidget.hpp>
#include <ufo/util/ucolor.hpp>

namespace ufo {

class UFO_EXPORT UBoProgress : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UBoProgress)
	UFO_UI_CLASS(UBoProgressUI)
public:
	UBoProgress(Orientation orientation = Horizontal);

	void setOrientation(Orientation o);
	Orientation getOrientation() const {
		return m_orientation;
	}
	void setMinimumValue(double v);
	void setMaximumValue(double v);
	void setValue(double v);
	double getMinimumValue() const {
		return m_min;
	}
	double getMaximumValue() const {
		return m_max;
	}
	double getValue() const {
		return m_value;
	}

	void setStartColor(const UColor& color);
	const UColor& startColor() const {
		return m_startcolor;
	}
	void setEndColor(const UColor& color);
	const UColor& endColor() const {
		return m_endcolor;
	}
	void setColor(const UColor& color);

protected:

private:  // Private attributes
	double m_min;
	double m_max;
	double m_value;
	Orientation m_orientation;
	UColor m_startcolor;
	UColor m_endcolor;
};

} // namespace ufo

#endif // UBOLABELUI_H

