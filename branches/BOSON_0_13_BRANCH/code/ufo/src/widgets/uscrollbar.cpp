/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/uscrollbar.cpp
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

#include "ufo/widgets/uscrollbar.hpp"

#include "ufo/events/umouseevent.hpp"

#include "ufo/umodel.hpp"
#include "ufo/ui/ustylehints.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UScrollBar, UAbstractSlider)

UScrollBar::UScrollBar(Orientation orientation,
		int valueA, int visAmountA ,
		int minA, int maxA)
	: UAbstractSlider()
	, m_isDragging(false)
	, m_mousePress()
	, m_visAmount(visAmountA)
{
	if (orientation != Horizontal) {
		setOrientation(orientation);
	}
	setCssType("scrollbar");
	// init sub controls for style
	getSliderModel()->subControls = UStyle::SubControls(
		UStyle::SC_ScrollBarSubLine | UStyle::SC_ScrollBarAddLine |
		UStyle::SC_ScrollBarSubPage | UStyle::SC_ScrollBarAddPage |
		UStyle::SC_ScrollBarSlider
	);
	// init active sub controls for style
	getSliderModel()->activeSubControls = UStyle::SC_None;
}

//
// public methods
//


int
UScrollBar::getVisibleAmount() const {
	return m_visAmount;
}

void
UScrollBar::setVisibleAmount(int amountA) {
	m_visAmount = amountA;
}

//
// protected methods
//

UDimension
UScrollBar::getContentsSize(const UDimension & maxSize) const {
	if (getStyleHints()->orientation == Horizontal) {
		return UDimension(32, 16);
	} else {
		return UDimension(16, 32);
	}
}

void
UScrollBar::processMouseEvent(UMouseEvent * e) {
	switch (e->getType()) {
		case UEvent::MousePressed: {
			e->consume();
			UStyle::SubControls subctrl = getStyle()->getSubControlAt(
				UStyle::CE_ScrollBar, getSize(), getStyleHints(),
				getModel(), e->getLocation()
			);
			getSliderModel()->activeSubControls = subctrl;
			switch (subctrl) {
				case UStyle::SC_ScrollBarSlider: {
					URectangle rect = getStyle()->getSubControlBounds(
						UStyle::CE_ScrollBar, getSize(), getStyleHints(),
						getModel(), UStyle::SC_ScrollBarSlider
					);
					m_isDragging = true;
					m_mousePress = rect.getLocation() - e->getLocation();
					repaint();
				}
				break;
				case UStyle::SC_ScrollBarSubLine:
					setValue(getValue() - getUnitIncrement());
				break;
				case UStyle::SC_ScrollBarAddLine:
					setValue(getValue() + getUnitIncrement());
				break;
				case UStyle::SC_ScrollBarSubPage:
					setValue(getValue() - getBlockIncrement());
				break;
				case UStyle::SC_ScrollBarAddPage:
					setValue(getValue() + getBlockIncrement());
				break;
				default:
				break;
			}
		}
		break;
		case UEvent::MouseReleased:
			getSliderModel()->activeSubControls = UStyle::SC_None;
			m_isDragging = false;
			repaint();
		break;
		case UEvent::MouseDragged:
		if (m_isDragging) {
			e->consume();
			URectangle rect = getStyle()->getSubControlBounds(
				UStyle::CE_ScrollBar, getSize(), getStyleHints(),
				getModel(), UStyle::SC_ScrollBarSlider
			);
			UPoint rel(e->getLocation() - rect.getLocation() + m_mousePress);

			float delta;
			if (getOrientation() == Vertical) {
				delta = (rel.y *
					float(getMaximum()) / (getHeight() - rect.h));
			} else {
				delta = (rel.x *
					float(getMaximum()) / (getWidth() - rect.w));
			}
			int idelta = 0;
			// rounding
			if (delta > 0) {
				idelta = int(delta + 0.5f);
			} else {
				idelta = int(delta - 0.5f);
			}
			setValue(getValue() + idelta);
		}
		break;
		default:
		break;
	}

}
