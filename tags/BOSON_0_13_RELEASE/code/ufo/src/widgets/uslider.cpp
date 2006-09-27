/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/uslider.cpp
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

#include "ufo/widgets/uslider.hpp"

#include "ufo/events/umouseevent.hpp"

#include "ufo/umodel.hpp"
#include "ufo/ui/ustylehints.hpp"

using namespace ufo;


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(USlider, UAbstractSlider)


USlider::USlider()
	: UAbstractSlider()
	, m_isDragging(false)
	, m_mousePress()
{
	// init sub controls for style
	getSliderModel()->subControls = UStyle::SubControls(
		UStyle::SC_SliderHandle | UStyle::SC_SliderGroove
	);
	// init active sub controls for style
	getSliderModel()->activeSubControls = UStyle::SC_None;
}

USlider::USlider(Orientation orientation)
	: UAbstractSlider()
	, m_isDragging(false)
	, m_mousePress()
{
	setOrientation(orientation);
	// init sub controls for style
	getSliderModel()->subControls = UStyle::SubControls(
		UStyle::SC_SliderHandle | UStyle::SC_SliderGroove
	);
	// init active sub controls for style
	getSliderModel()->activeSubControls = UStyle::SC_None;
}

USlider::USlider(int min, int max, int value)
	: UAbstractSlider()
	, m_isDragging(false)
	, m_mousePress()
{
	setRange(min, max);
	setValue(value);
	// init sub controls for style
	getSliderModel()->subControls = UStyle::SubControls(
		UStyle::SC_SliderHandle | UStyle::SC_SliderGroove
	);
	// init active sub controls for style
	getSliderModel()->activeSubControls = UStyle::SC_None;
}

USlider::USlider(Orientation orientation, int min, int max, int value)
	: UAbstractSlider()
	, m_isDragging(false)
	, m_mousePress()
{
	setOrientation(orientation);
	setRange(min, max);
	setValue(value);
	// init sub controls for style
	getSliderModel()->subControls = UStyle::SubControls(
		UStyle::SC_SliderHandle | UStyle::SC_SliderGroove
	);
	// init active sub controls for style
	getSliderModel()->activeSubControls = UStyle::SC_None;
}

UDimension
USlider::getContentsSize(const UDimension & maxSize) const {
	if (getStyleHints()->orientation == Vertical) {
		return UDimension(0, 100);
	} else {
		return UDimension(100, 0);
	}
}

void
USlider::processMouseEvent(UMouseEvent * e) {
	switch (e->getType()) {
		case UEvent::MousePressed: {
			e->consume();
			UPoint pos = e->getLocation();
			URectangle rect = getStyle()->getSubControlBounds(
				UStyle::CE_Slider, getSize(), getStyleHints(),
				getModel(), UStyle::SC_SliderHandle
			);
			//if (m_rect.contains(pos)) {
			if (rect.contains(pos)) {
				// dragging
				m_isDragging = true;
				m_mousePress = rect.getLocation() - e->getLocation();
				getSliderModel()->activeSubControls = UStyle::SC_SliderHandle;
				repaint();
			} else {
				m_isDragging = false;
				// block scrolling
				int value = getValue();
				int delta = 0;
				if (getOrientation() == Vertical) {
					if (pos.y <= rect.y) {
						delta = - getBlockIncrement();//Up)
					} else {
						delta = getBlockIncrement();//Down)
					}
				} else {
					if (pos.x <= rect.x) {
						delta = - getBlockIncrement();//Up)
					} else {
						delta = getBlockIncrement();//Down)
					}
				}
				if (delta) {
					setValue(value + delta);
				}
			}
		}
		break;
		case UEvent::MouseReleased:
			e->consume();
			getSliderModel()->activeSubControls = UStyle::SC_None;
			m_isDragging = false;
			repaint();
		break;
		case UEvent::MouseDragged:
		if (m_isDragging) {
			e->consume();
			URectangle rect = getStyle()->getSubControlBounds(
				UStyle::CE_Slider, getSize(), getStyleHints(),
				getModel(), UStyle::SC_SliderHandle
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
