/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/ustackwidget.hpp
    begin             : Sun Sep 25 2005
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

#include "ufo/widgets/ustackwidget.hpp"

using namespace ufo;

UFO_IMPLEMENT_CLASS(UStackWidget, UWidget)

UStackWidget::UStackWidget()
	: m_selectedIndex(0)
	, m_selectionMode(1)
{
}

UStackWidget::UStackWidget(int selectionMode)
	: m_selectedIndex(0)
	, m_selectionMode(selectionMode)
{
}

void
UStackWidget::setSelectedIndex(int index) {
	int count = getWidgetCount();

	if (m_selectionMode && index < count && index >= 0) {
		UWidget * oldWidget = getWidget(m_selectedIndex);
		if (oldWidget) {
			oldWidget->setVisible(false);
		}
		m_selectedIndex = index;
		getWidget(m_selectedIndex)->setVisible(true);

		// FIXME: is this necessary?
		invalidate();
	}
}

int
UStackWidget::getSelectedIndex() const {
	return m_selectedIndex;
}

void
UStackWidget::addImpl(UWidget * w, UObject * constraints, int index) {
	// stack is from bottom up
	// FIXME: add reverse showing attribute to UWidget
	/*if (index == -1) {
		index = 0;
	}*/
	if (index == -1) {
		index = getWidgetCount();
	}
	UWidget * oldWidget = getWidget(index);
	if (m_selectionMode && oldWidget) {
		oldWidget->setVisible(false);
	}
	UWidget::addImpl(w, constraints, index);
	if (m_selectionMode && index != m_selectedIndex) {
		w->setVisible(false);
	}
}

bool
UStackWidget::removeImpl(int index) {
	return UWidget::removeImpl(index);
}

UDimension
UStackWidget::getContentsSize(const UDimension & maxSize) const {
	UDimension ret;
	for (unsigned int i = 0; i < getWidgetCount(); ++i) {
		UWidget * w = getWidget(i);
		if (w) {
			ret.expand(w->getPreferredSize(maxSize));
		}
	}
	ret.clamp(maxSize);
	return ret;
}
