/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/uscrollablewidget.cpp
    begin             : Wed Jun 5 2002
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

#include "ufo/widgets/uscrollablewidget.hpp"

#include "ufo/layouts/uborderlayout.hpp"

#include "ufo/ugraphics.hpp"

namespace ufo {

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UScrollableWidget, UWidget)

UScrollableWidget::UScrollableWidget()
	: m_unitIncrement(0)
	, m_blockIncrement(0)
	, m_viewSize(UDimension(100, 100))
{
	setLayout(new UBorderLayout());
}

int
UScrollableWidget::getUnitIncrement(const URectangle & visibleRectA,
		Orientation orientationA, Direction directionA) const {
	// FIXME: doh! what a hack
	int ret = (m_unitIncrement) ? m_unitIncrement : 1;
	
	if (directionA == Up) {
		ret = -ret;
	}
	return ret;
}

void
UScrollableWidget::setUnitIncrement(int incrementA) {
	m_unitIncrement = incrementA;
}

int
UScrollableWidget::getBlockIncrement(const URectangle & visibleRectA,
		Orientation orientationA, Direction directionA) const {
	int ret = (m_blockIncrement) ? m_blockIncrement : 10;
	
	if (directionA == Up) {
		ret = -ret;
	}
	return ret;
}

void
UScrollableWidget::setBlockIncrement(int incrementA) {
	m_blockIncrement = incrementA;
}

UDimension
UScrollableWidget::getPreferredViewportSize() const {
	return m_viewSize;
}

void
UScrollableWidget::setPreferredViewportSize(const UDimension & viewSize) {
	m_viewSize = viewSize;
}

void
UScrollableWidget::clipPaint(UGraphics * g, int x, int y, int w, int h) {
	UPoint pos = pointToRootPoint(x, y);

	//graphics->pushClipRect();
	URectangle old_rect = g->getClipRect();
	g->setClipRect(URectangle(pos.x, pos.y, w, h));
	
	paint(g);

	//graphics->popClipRect();
	g->setClipRect(old_rect);
}

} // namespace ufo
