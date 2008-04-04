/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/uviewport.cpp
    begin             : Sat Oct 12 2002
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

#include "ufo/widgets/uviewport.hpp"
#include "ufo/widgets/uscrollablewidget.hpp"

#include "ufo/ugraphics.hpp"

namespace ufo {

// implementation of the viewport layout manager
class UViewportLayout : public ULayoutManager {
public:
	UDimension getPreferredLayoutSize(const UWidget * parent) const {
		const UViewport * viewport = dynamic_cast<const UViewport*>(parent);
		UWidget * w = viewport->getView();
		if (w) {
			if (UScrollableWidget * sw = dynamic_cast<UScrollableWidget*>(w)) {
				return sw->getPreferredViewportSize();
			} else {
				return w->getPreferredSize();
			}
		} else {
			return UDimension();
		}
	}
	UDimension getPreferredLayoutSize(const UWidget * parent, const UDimension & maxSize) const {
		const UViewport * viewport = dynamic_cast<const UViewport*>(parent);
		UWidget * w = viewport->getView();
		if (w) {
			if (UScrollableWidget * sw = dynamic_cast<UScrollableWidget*>(w)) {
				return sw->getPreferredViewportSize();
			} else {
				return w->getPreferredSize(maxSize);
			}
		} else {
			return UDimension();
		}
	}

	UDimension getMinimumLayoutSize(const UWidget * parent) const {
		return getPreferredLayoutSize(parent);
	}

	void layoutContainer(const UWidget * parent) {
		if (parent->getWidgetCount()) {
			// expand to have at least parent size
			UDimension size = parent->getInnerSize();

			UDimension childSize = parent->getWidget(0)->getPreferredSize();
			childSize.expand(size);

			parent->getWidget(0)->setSize(childSize);
		}
	}
};


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UViewport, UWidget)


UViewport::UViewport()
	: UWidget()
	, m_view(NULL)
	, m_scrollableWidget(false)
{
	setLayout(createLayoutManager());
	setClipping(true);
	// FIXME: should this be set via style sheets?
	setCssClass("transparent");
}

void
UViewport::setView(UWidget * w) {
	// remove any other widgets
	removeAll();

	m_view = w;

	if (m_view) {
		// call the
		UWidget::addImpl(m_view, NULL, -1);
	}

	if (dynamic_cast<UScrollableWidget*>(w)) {
		m_scrollableWidget = true;
	} else {
		m_scrollableWidget = false;
	}
}

UWidget *
UViewport::getView() const {
	return m_view;
}

UPoint
UViewport::getViewLocation() const {
	if (m_view) {
		return - m_view->getLocation();
	}
	return UPoint();
}

void
UViewport::setViewLocation(const UPoint & pos) {
	if (m_view) {
		m_view->setLocation(-pos);
	}
}

void
UViewport::setViewBounds(int x, int y, int w, int h) {
	if (m_view) {
		m_view->setLocation(UPoint(-x, -y));

	}

	if (m_scrollableWidget && m_view) {
		(static_cast<UScrollableWidget*>(m_view))->setPreferredViewportSize(UDimension(w, h));
	} else {
		setSize(UDimension(w, h));
	}
}

void
UViewport::scrollRectToVisible(const URectangle & rect) {
	if (!m_view) {
		return;
	}

	//const UPoint & p = m_view->getLocation();

	// x vlaue

	// FIXME !
	// not yet implemented
}

ULayoutManager *
UViewport::createLayoutManager() {
	return new UViewportLayout();
}

void
UViewport::addImpl(UWidget * w, UObject * constraints, int index) {
	if (w) {
		setView(w);
	}
}

void
UViewport::paintChildren(UGraphics * g) {
	/*if (m_view) {
		UGraphics * graphics = getGraphics();
		UPoint pos = pointToRootPoint(getLocation());
		// FIXME !
		// has to be changed when coordinate API is changed
		UInsets insets = getParent()->getInsets();
		pos.x += insets.left;
		pos.y += insets.top;

		graphics->pushClipRect();
		// FIXME
		// works only for viewports without a border
		graphics->setClipRect(URectangle(pos.x, pos.y, getWidth(), getHeight()));

		m_view->paint();

		graphics->popClipRect();
	}*/
	UWidget::paintChildren(g);
}

} // namespace ufo
