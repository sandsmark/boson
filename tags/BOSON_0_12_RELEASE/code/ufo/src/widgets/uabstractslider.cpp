/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/uabstractslider.cpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/widgets/uabstractslider.hpp"

#include "ufo/events/umousewheelevent.hpp"

#include "ufo/umodel.hpp"
#include "ufo/ui/ustyle.hpp"
#include "ufo/ui/ustylehints.hpp"

using namespace ufo;


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UAbstractSlider, UWidget)


USliderModel*
_createSliderModel(UWidgetModel * model, int min, int max, int val, int unit, int block) {
	USliderModel * c = new USliderModel();
	c->widgetState = model->widgetState;
	c->subControls = UStyle::SubControls(
		UStyle::SC_ScrollBarSubLine | UStyle::SC_ScrollBarAddLine |
		UStyle::SC_ScrollBarSubPage | UStyle::SC_ScrollBarAddPage |
		UStyle::SC_ScrollBarSlider
	);
	c->minimum = min;
	c->maximum = max;
	if (c->maximum <= 0) {
		c->maximum = 1;
	}
	c->tickInterval = 0;
	c->sliderPosition = 0;
	c->sliderValue = val;
	c->unitIncrement = unit;
	c->blockIncrement = block;
	delete (model);
	return c;
}

UAbstractSlider::UAbstractSlider()
{
	m_model = _createSliderModel(m_model, 0, 99, 0, 1, 10);
}

int
UAbstractSlider::getMinimum() const {
	return getSliderModel()->minimum;
}

int
UAbstractSlider::getMaximum() const {
	return getSliderModel()->maximum;
}

void
UAbstractSlider::setMinimum(int min) {
	getSliderModel()->minimum = min;
	if (getValue() < min) {
		setValue(min);
	}
}

void
UAbstractSlider::setMaximum(int max) {
	if (max > 0) {
	getSliderModel()->maximum = max;
	if (getValue() > max) {
		setValue(max);
	}
	}
}

void
UAbstractSlider::setRange(int min, int max) {
	setMinimum(min);
	setMaximum(max);
}

void
UAbstractSlider::setValue(int newValue) {
	// clamp
	int value = std::max(getMinimum(), newValue);
	value = std::min(getMaximum()/* - m_visAmount*/, value);

	if (getSliderModel()->sliderValue != value) {
		getSliderModel()->sliderValue = value;

		repaint();
		m_sigValueChanged(this);
	}
}

int
UAbstractSlider::getValue() const {
	return getSliderModel()->sliderValue;
}


int
UAbstractSlider::getUnitIncrement() const {
	return getSliderModel()->unitIncrement;
}

void
UAbstractSlider::setUnitIncrement(int inc) {
	getSliderModel()->unitIncrement = inc;
}


int
UAbstractSlider::getBlockIncrement() const {
	return getSliderModel()->blockIncrement;
}

void
UAbstractSlider::setBlockIncrement(int inc) {
	getSliderModel()->blockIncrement = inc;
}


USliderModel *
UAbstractSlider::getSliderModel() const {
	return static_cast<USliderModel*>(m_model);
}

void
UAbstractSlider::processMouseWheelEvent(UMouseWheelEvent * e) {
	setValue(getValue() - e->getWheelRotation() * getUnitIncrement());
	e->consume();
}
