/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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

#include "ufo/events/umousewheelevent.hpp"

//#include "ufo/ui/uuimanager.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UScrollBar, UWidget)

UScrollBar::UScrollBar(Orientation orientationA,
		int valueA, int visAmountA ,
		int minA, int maxA)
	: m_orientation(orientationA)
	, m_value(valueA)
	, m_visAmount(visAmountA)
	, m_minValue(minA)
	, m_maxValue(maxA)
	, m_unitIncrement(1)
	, m_blockIncrement(10)
{
	setEventState(UEvent::MouseWheel, true);
}

//*
//* hides | overrides UWidget
//*
/*
void
UScrollBar::setUI(UScrollBarUI * ui) {
	UWidget::setUI(ui);
}

UWidgetUI *
UScrollBar::getUI() const {
	return static_cast<UScrollBarUI*>(UWidget::getUI());
}

void
UScrollBar::updateUI() {
	setUI(static_cast<UScrollBarUI*>(getUIManager()->getUI(this)));
}
*/
//*
//* public methods
//*



Orientation
UScrollBar::getOrientation() const {
	return m_orientation;
}

int
UScrollBar::getValue() const {
	return m_value;
}

void
UScrollBar::setValue(int newValueA) {
	// clamp
	int value = std::max(m_minValue, newValueA);
	m_value = std::min(m_maxValue - m_visAmount, value);

	m_sigValueChanged(this, m_value);
}


int
UScrollBar::getMaximum() const {
	return m_maxValue;
}

void
UScrollBar::setMaximum(int maxA) {
	m_maxValue = maxA;
	if (m_value > m_maxValue) {
		setValue(m_maxValue);
	}
}


int
UScrollBar::getMinimum() const {
	return m_minValue;
}

void
UScrollBar::setMinimum(int minA) {
	m_minValue = minA;
	if (m_value < m_minValue) {
		setValue(m_minValue);
	}
}


int
UScrollBar::getVisibleAmount() const {
	return m_visAmount;
}

void
UScrollBar::setVisibleAmount(int amountA) {
	m_visAmount = amountA;
}


int
UScrollBar::getUnitIncrement(Direction directionA) const {
	if (directionA == Up) {
		return - m_unitIncrement;
	} else {
		return m_unitIncrement;
	}
}

void
UScrollBar::setUnitIncrement(int incA) {
	m_unitIncrement = incA;
}


int
UScrollBar::getBlockIncrement(Direction directionA) const {
	if (directionA == Up) {
		return - m_blockIncrement;
	} else {
		return m_blockIncrement;
	}
}

void
UScrollBar::setBlockIncremenet(int incA) {
	m_blockIncrement = incA;
}


void
UScrollBar::processMouseWheelEvent(UMouseWheelEvent * e) {
	Direction dir = (e->getDelta() > 0) ? Up : Down;

	if (dir == Down) {
		setValue(m_value - e->getWheelRotation() * getUnitIncrement(dir));
	} else {
		setValue(m_value + e->getWheelRotation() * getUnitIncrement(dir));
	}
}
