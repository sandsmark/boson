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

#include "uboprogress.h"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UBoProgress, UWidget)

// AB: by default min=0, max=100 which is perfect for percentage values.
// therefore if a user creates a progress widget he can use percentage values in
// setValue() without further modification
UBoProgress::UBoProgress(Orientation o)
	: UWidget(),
	m_min(0.0),
	m_max(100.0),
	m_value (50.0),
	m_orientation(o) {
}

void
UBoProgress::setMinimumValue(double v) {
	m_min = std::min(v, getMaximumValue());
	m_value = std::max(m_value, m_min);
	repaint();
}

void
UBoProgress::setMaximumValue(double v) {
	m_max = std::max(v, getMinimumValue());
	m_value = std::min(m_value, m_max);
	repaint();
}

void
UBoProgress::setValue(double v) {
	v = std::max(v, getMinimumValue());
	v = std::min(v, getMaximumValue());
	m_value = v;
	repaint();
}

void UBoProgress::setOrientation(Orientation o) {
	m_orientation = o;
	invalidate();
	repaint();
}

