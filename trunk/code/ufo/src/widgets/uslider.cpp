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

#include "ufo/events/umousewheelevent.hpp"

using namespace ufo;


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(USlider, UWidget)


USlider::USlider()
	: m_orientation(Horizontal)
	, m_min(0)
	, m_max(100)
	, m_value(0)
	, m_unitIncrement(1)
	, m_blockIncrement(10)
{
	setEventState(UEvent::MouseWheel, true);
}
USlider::USlider(Orientation orientation)
	: m_orientation(orientation)
	, m_min(0)
	, m_max(100)
	, m_value(0)
	, m_unitIncrement(1)
	, m_blockIncrement(10)
{
	setEventState(UEvent::MouseWheel, true);
}

USlider::USlider(int min, int max, int value)
	: m_orientation(Horizontal)
	, m_min(min)
	, m_max(max)
	, m_value(value)
	, m_unitIncrement(1)
	, m_blockIncrement(10)
{
	setEventState(UEvent::MouseWheel, true);
}

USlider::USlider(Orientation orientationA, int min, int max, int value)
	: m_orientation(orientationA)
	, m_min(min)
	, m_max(max)
	, m_value(value)
	, m_unitIncrement(1)
	, m_blockIncrement(10)
{
	setEventState(UEvent::MouseWheel, true);
}


void
USlider::setMinimum(int min) {
	m_min = min;
	if (m_value < m_min) {
		setValue(m_min);
	}
}

void
USlider::setMaximum(int min) {
	m_max = min;
	if (m_value > m_max) {
		setValue(m_max);
	}
}

void
USlider::setValue(int newValue) {
	// clamp
	int value = std::max(m_min, newValue);
	m_value = std::min(m_max/* - m_visAmount*/, value);

	repaint();

	m_sigValueChanged(this, m_value);
}

int
USlider::getValue() const {
	return m_value;
}


int
USlider::getUnitIncrement() const {
	return m_unitIncrement;
}

void
USlider::setUnitIncrement(int inc) {
	m_unitIncrement = inc;
}


int
USlider::getBlockIncrement() const {
	return m_blockIncrement;
}

void
USlider::setBlockIncrement(int inc) {
	m_blockIncrement = inc;
}

/*
int
USlider::getMajorTickSpacing() const {
	return m_majorTickSpacing;
}

void
USlider::setMajorTickSpacing(int spacing) {
	m_majorTickSpacing = spacing;
}
*/

void
USlider::processMouseWheelEvent(UMouseWheelEvent * e) {
	//Direction dir = (e->getDelta() > 0) ? Up : Down;

	setValue(m_value + e->getWheelRotation() * getUnitIncrement());
	/*if (dir == Down) {
	} else {
		setValue(m_value + e->getWheelRotation());// * getUnitIncrement(dir));
	}*/
}
