/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/uscrollpane.cpp
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

#include "ufo/widgets/uscrollpane.hpp"

#include "ufo/widgets/uscrollbar.hpp"
#include "ufo/widgets/uviewport.hpp"

#include "ufo/layouts/uborderlayout.hpp"
#include "ufo/events/umousewheelevent.hpp"

//#include "ufo/borders/uborderfactory.hpp"

#include "ufo/util/ucolor.hpp"

namespace ufo {

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UScrollPane, UWidget)

UScrollPane::UScrollPane(UScrollableWidget * viewA)
	: m_viewport(new UViewport())
	, m_horizontal(NULL)
	, m_vertical(NULL)
	, m_autoAdd(true)
{
	setLayout(new UBorderLayout());
	add(m_viewport, UBorderLayout::Center);

	m_vertical = new ScrollBar(this, Vertical);
	m_vertical->sigScrollPos().connect(slot(*this, &UScrollPane::on_scroll));
	trackPointer(m_vertical);

	m_horizontal = new ScrollBar(this, Horizontal);
	m_horizontal->sigScrollPos().connect(slot(*this, &UScrollPane::on_scroll));
	trackPointer(m_horizontal);

	setScrollable(viewA);

	setEventState(UEvent::MouseWheel, true);

	// FIXME !
	// to UI class?
	//setBorder(UBorderFactory::createBevelBorder(Lowered));
	setBorder(LoweredBevelBorder);//UBorderFactory::createBevelBorder(Lowered));
	//setBorder(UBorderFactory::createLineBorder(UColor::red));
}


void
UScrollPane::setScrollable(UScrollableWidget * viewA) {
	UWidget * oldView = m_viewport->getView();
	if (oldView) {
		oldView->reference();
	}

	// save old border type and install empty border
	BorderType border = NoBorder;
	if (viewA) {
		border = viewA->getBorder();
		viewA->setBorder(NoBorder);
	}

	m_viewport->setView(viewA);

	// reset the old border
	if (oldView) {
		oldView->setBorder(m_oldBorderType);
		oldView->unreference();
	}
	m_oldBorderType = border;

	if (viewA && isInValidHierarchy()) {
		installScrollable();
	}
	//invalidate();
}

UScrollableWidget *
UScrollPane::getScrollable() const {
	return dynamic_cast<UScrollableWidget*>(m_viewport->getView());
}

void
UScrollPane::setAutoAddingScrollBars(bool b) {
	m_autoAdd = b;
}

bool
UScrollPane::isAutoAddingScrollBars() const {
	return m_autoAdd;
}

void
UScrollPane::addedToHierarchy() {
	UWidget::addedToHierarchy();
	installScrollable();
}

void
UScrollPane::processMouseWheelEvent(UMouseWheelEvent * e) {
	// lazily send a copy to the vertical scroll bar
	UMouseWheelEvent * newE = new UMouseWheelEvent(
		m_vertical,
		e->getType(),
		e->getModifiers(),
		UPoint(),
		e->getDelta(),
		e->getWheel()
	);

	m_vertical->dispatchEvent(newE);
}


void
UScrollPane::on_scroll(UScrollBar * scrollBarA, int amountA) {
	if (scrollBarA->getOrientation() == Horizontal) {
		UPoint pos = m_viewport->getViewLocation();
		pos.x = amountA;
		m_viewport->setViewLocation(pos);
	} else if (scrollBarA->getOrientation() == Vertical) {
		UPoint pos = m_viewport->getViewLocation();
		pos.y = amountA;
		m_viewport->setViewLocation(pos);
	}
}

void
UScrollPane::installScrollable() {
	UScrollableWidget * view = dynamic_cast<UScrollableWidget*>(m_viewport->getView());
	if (!view) {
		return;
	}

	view->invalidateSelf();
	view->validate();

	UDimension size;// = view->getSize();
	if (size.isEmpty()) {
		size = view->getPreferredSize();
	}
	m_vertical->setMaximum(size.h);
	m_horizontal->setMaximum(size.w);
	m_vertical->setMinimum(0);
	m_horizontal->setMinimum(0);

	UDimension viewSize = view->getPreferredViewportSize();
	m_vertical->setVisibleAmount(viewSize.h);
	m_horizontal->setVisibleAmount(viewSize.w);

	// check for auto adding scroll bars
	if (m_autoAdd) {
		if (viewSize.h >= size.h) {
			if (m_vertical->getParent()) {
				remove(m_vertical);
			}
		} else {
			if (!m_vertical->getParent()) {
				add(m_vertical, UBorderLayout::East);
			}
		}
		if (viewSize.w >= size.w) {
			if (m_horizontal->getParent()) {
				remove(m_horizontal);
			}
		} else {
			if (!m_horizontal->getParent()) {
				add(m_horizontal, UBorderLayout::South);
			}
		}
	} else {
		if (!m_vertical->getParent()) {
			add(m_vertical, UBorderLayout::East);
		}
		if (!m_horizontal->getParent()) {
			add(m_horizontal, UBorderLayout::South);
		}
	}
}

//
// protected classes
// ScrollBar
//

int
UScrollPane::ScrollBar::getUnitIncrement(Direction directionA) const {
	UScrollableWidget * w = m_scrollPane->getScrollable();
	return w->getUnitIncrement(m_scrollPane->getWidget(0)->getBounds(),
		getOrientation(), directionA);
}

int
UScrollPane::ScrollBar::getBlockIncrement(Direction directionA) const {
	UScrollableWidget * w = m_scrollPane->getScrollable();
	return w->getBlockIncrement(m_scrollPane->getWidget(0)->getBounds(),
		getOrientation(), directionA);
}

} // namespace ufo

