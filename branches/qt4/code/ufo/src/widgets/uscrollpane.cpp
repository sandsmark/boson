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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/widgets/uscrollpane.hpp"

#include "ufo/widgets/uscrollbar.hpp"
#include "ufo/widgets/uviewport.hpp"

#include "ufo/layouts/uborderlayout.hpp"
#include "ufo/events/umousewheelevent.hpp"

//#include "ufo/borders/uborderfactory.hpp"

#include "ufo/util/ucolor.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UScrollPane, UWidget)

UScrollPane::UScrollPane(UScrollableWidget * viewA)
	: m_viewport(new UViewport())
	, m_horizontal(NULL)
	, m_vertical(NULL)
	, m_autoAdd(true)
{
	setLayout(new UBorderLayout(0, 0));
	add(m_viewport, UBorderLayout::Center);

	m_vertical = new UScrollBar(Vertical);//this, Vertical);
	m_vertical->sigValueChanged().connect(slot(*this, &UScrollPane::on_scroll));
	trackPointer(m_vertical);

	m_horizontal = new UScrollBar();//this, Horizontal);
	m_horizontal->sigValueChanged().connect(slot(*this, &UScrollPane::on_scroll));
	trackPointer(m_horizontal);

	setScrollable(viewA);

	setEventState(UEvent::MouseWheel, true);

	// FIXME: should this be set via style sheets?
	setCssClass("transparent");

	// FIXME !
	// to UI class?
	//setBorder(UBorderFactory::createBevelBorder(Lowered));
	//setBorder(LoweredBevelBorder);//UBorderFactory::createBevelBorder(Lowered));
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

	if (m_vertical->isVisible()) {
		m_vertical->dispatchEvent(newE);
	}
}

void
UScrollPane::processWidgetEvent(UWidgetEvent * e) {
	switch (e->getType()) {
		case UEvent::WidgetAdded:
			installScrollable();
		break;
		case UEvent::WidgetResized:
			installScrollable();
		break;
		default:
		break;
	}
	UWidget::processWidgetEvent(e);
}

void
UScrollPane::on_scroll(UAbstractSlider * slider) {
	if (slider->getOrientation() == Horizontal) {
		UPoint pos = m_viewport->getViewLocation();
		pos.x = slider->getValue();
		m_viewport->setViewLocation(pos);
	} else if (slider->getOrientation() == Vertical) {
		UPoint pos = m_viewport->getViewLocation();
		pos.y = slider->getValue();
		m_viewport->setViewLocation(pos);
	}
}

void
UScrollPane::installScrollable() {
	UWidget * view = m_viewport->getView();
	UScrollableWidget * scrollableView = dynamic_cast<UScrollableWidget*>(view);
	if (!view) {
		return;
	}

	view->invalidateSelf();
	view->validateSelf();
	validateSelf();

	UDimension size;// = view->getSize();
	if (size.isEmpty()) {
		size = view->getPreferredSize();
	}
	UDimension viewSize;
	if (scrollableView) {
		viewSize = scrollableView->getPreferredViewportSize();
	} else {
		viewSize = getPreferredSize();
		if (!viewSize.isValid()) {
			viewSize = UDimension(150, 150);
		}
	}
	m_vertical->setRange(0, size.h - viewSize.h);
	m_horizontal->setRange(0, size.w - viewSize.w);
/*
	m_vertical->setVisibleAmount(viewSize.h);
	m_horizontal->setVisibleAmount(viewSize.w);
*/
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
	if (scrollableView) {
		m_vertical->setUnitIncrement(scrollableView->getUnitIncrement(Vertical));
		m_vertical->setBlockIncrement(scrollableView->getBlockIncrement(Vertical));
		m_horizontal->setUnitIncrement(scrollableView->getUnitIncrement(Horizontal));
		m_horizontal->setBlockIncrement(scrollableView->getBlockIncrement(Horizontal));
	}
}
