/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasicscrollbarui.cpp
    begin             : Mon Jul 22 2002
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

#include "ufo/ui/basic/ubasicscrollbarui.hpp"

//#include "ufo/ufo_gl.hpp"
#include "ufo/utoolkit.hpp"

#include "ufo/ugraphics.hpp"
#include "ufo/ui/uuimanager.hpp"

#include "ufo/widgets/uscrollbar.hpp"
#include "ufo/widgets/ubutton.hpp"

#include "ufo/layouts/uborderlayout.hpp"

#include "ufo/events/umouseevent.hpp"

#include "ufo/image/uxbmicon.hpp"
#include "ufo/util/ucolor.hpp"

#include "arrows.xbm"


namespace ufo {

UFO_IMPLEMENT_ABSTRACT_CLASS(UBasicScrollBarUI, UScrollBarUI)

/** This class represents the knob and the sliding space for a scroll bar.
  */
class TrackWidget : public UWidget {
private:
	UScrollBar * m_scrollBar;
	URectangle m_rect;
	bool m_isDragging;
	UPoint m_mousePress;
public:
	TrackWidget(UScrollBar * scrollBar)
			: m_scrollBar(scrollBar)
			, m_isDragging(false)
			, m_mousePress()
	{
		sigMousePressed().connect(slot(*this, &TrackWidget::mousePressed));
		sigMouseDragged().connect(slot(*this, &TrackWidget::mouseDragged));
		setEventState(UEvent::MouseMoved, true);

		scrollBar->sigValueChanged().connect(slot(*this, &TrackWidget::valueChanged));

		valueChanged(m_scrollBar, m_scrollBar->getValue());
		//setBorder(new ULineBorder(UColor::blue));
	}
	~TrackWidget() {
		sigMousePressed().disconnect(slot(*this, &TrackWidget::mousePressed));
		sigMouseDragged().disconnect(slot(*this, &TrackWidget::mouseDragged));

		m_scrollBar->sigValueChanged().disconnect(slot(*this, &TrackWidget::valueChanged));
	}

	void paintWidget(UGraphics * g) {
		UWidget::paintWidget(g);

		//glColor3fv(m_foreground->getFloat());
		//glRecti(m_rect.x, m_rect.y, m_rect.x + m_rect.w, m_rect.y + m_rect.h);
		//g->setColor(getColorGroup().highlight());
		g->setColor(getColorGroup().highlight());
		//g->setColor(m_foreground);
		g->fillRect(m_rect);
		g->setColor(getColorGroup().highlight().darker());
		g->drawRect(m_rect);
	}
	void setBounds(int x, int y, int w, int h) {
		UWidget::setBounds(x, y, w, h);
		valueChanged(m_scrollBar, m_scrollBar->getValue());
	}
	void valueChanged(UScrollBar * scroll, int value) {
		int max = m_scrollBar->getMaximum();
		int amount = m_scrollBar->getVisibleAmount();

		float amountDelta = float(amount) / max;
		float widthDelta = float(getWidth()) / max;
		float heightDelta = float(getHeight()) / max;

		if (amountDelta > 1.0f) {
			amountDelta = 1.0f;
			widthDelta = 0.0f;
			heightDelta = 0.0f;
		}

		if (m_scrollBar->getOrientation() == Vertical) {
			m_rect.setBounds(0, int(value * heightDelta),
				getWidth() - 1, int(getHeight() * amountDelta));
		} else {
			m_rect.setBounds(int(value * widthDelta), 0,
				int(getWidth() * amountDelta), getHeight() - 1);
		}
	}
	// block scrolling
	void mousePressed(UMouseEvent * e) {
		e->consume();
		UPoint pos = e->getLocation();
		if (m_rect.contains(pos)) {
			//getContext()->setEventGrabber(m_eventGrabberSlot);
			m_isDragging = true;
			m_mousePress = m_rect.getLocation() - pos;
		} else {
			m_isDragging = false;
			// block scrolling
			int value = m_scrollBar->getValue();
			int delta = 0;
			if (m_scrollBar->getOrientation() == Vertical) {
				if (pos.y <= m_rect.y) {
					delta = m_scrollBar->getBlockIncrement(Up);
				} else {
					delta = m_scrollBar->getBlockIncrement(Down);
				}
			} else {
				if (pos.x <= m_rect.x) {
					delta =  m_scrollBar->getBlockIncrement(Up);
				} else {
					delta =  m_scrollBar->getBlockIncrement(Down);
				}
			}
			if (delta) {
				m_scrollBar->setValue(value + delta);
			}
		}
	}
	void mouseDragged(UMouseEvent * e) {
		if (!m_isDragging){
			return;
		}
		e->consume();
		UPoint pos(m_rect.getLocation() - e->getLocation());
		UPoint rel(m_mousePress - pos);

		float delta;
		if (m_scrollBar->getOrientation() == Vertical) {
			delta = (rel.y *
				float(m_scrollBar->getMaximum()) / (m_scrollBar->getHeight() - m_rect.h));
		} else {
			delta = (rel.x *
				float(m_scrollBar->getMaximum()) / (m_scrollBar->getWidth() - m_rect.w));
		}
		int idelta = 0;
		// rounding
		if (delta > 0) {
			idelta = int(delta + 0.5f);
		} else {
			idelta = int(delta - 0.5f);
		}
		m_scrollBar->setValue(m_scrollBar->getValue() + idelta);
	}
	/*
	// using the track button to scroll
	void trackDraggedSlot(UEvent * e) {
		// huh, this code is rather ugly
		if (e->getType() == UEvent::MouseDragged) {
			UMouseEvent * mouseEvent = dynamic_cast<UMouseEvent*>(e);
			int delta;
			if (m_scrollBar->getOrientation() == Vertical) {
				delta = int(mouseEvent->getRelMovement().y *
					float(m_scrollBar->getMaximum()) / getHeight());
			} else {
				delta = int(mouseEvent->getRelMovement().x *
					float(m_scrollBar->getMaximum()) / getWidth());
			}

			// compute new scroll value
			int value = m_overDrag + m_scrollBar->getValue() + delta;

			// clamp and check if we should set
			// FIXME hmm, this extent thingie is not really wanted (?)
			if (value > m_scrollBar->getMaximum() - m_scrollBar->getVisibleAmount()) {
				m_overDrag = value - m_scrollBar->getMaximum() + m_scrollBar->getVisibleAmount();
			} else if (value < m_scrollBar->getMinimum()) {
				m_overDrag = value - m_scrollBar->getMinimum();
			} else {
				m_overDrag = 0;
			}
			m_scrollBar->setValue(value);
		} else if (e->getType() == UEvent::MouseReleased) {
			// release event grabbing
			getContext()->releaseEventGrabber();
			m_overDrag = 0;
		}
	}*/
};

//UBasicScrollBarUI * UBasicScrollBarUI::m_scrollBarUI =
//	new UBasicScrollBarUI();

std::string UBasicScrollBarUI::m_lafId("UScrollBar");

UBasicScrollBarUI::UBasicScrollBarUI(UScrollBar * scrollBar)
	: m_scrollBar(scrollBar)
	, m_up(new UXBMIcon(arrow_up_bits, arrow_up_width, arrow_up_height))
	, m_down(new UXBMIcon(arrow_down_bits, arrow_down_width, arrow_down_height))
	, m_left(new UXBMIcon(arrow_left_bits, arrow_left_width, arrow_left_height))
	, m_right(new UXBMIcon(arrow_right_bits, arrow_right_width, arrow_right_height))
{}

UBasicScrollBarUI *
UBasicScrollBarUI::createUI(UWidget * w) {
	return new UBasicScrollBarUI(dynamic_cast<UScrollBar*>(w));
}

void
UBasicScrollBarUI::paint(UGraphics * g, UWidget * w) {
	//UWidgetUI::paint(w);
/*
	UScrollBar * scroll = dynamic_cast<UScrollBar*>(w);
	if (!scroll) {
		return;
	}
	UDimension size = w->getInnerSize();

	//Direction dir = scroll->getOrientation();

	if (scroll->getOrientation() == Vertical) {
		if (m_pushed == Up) {
			glColor3fv(UColor::black->getFloat());
			glRecti(0, 0, m_up->getIconWidth(), m_up->getIconHeight());

			glColor3fv(UColor::white->getFloat());
			m_up->paintIcon(w, 0, 0);

			glColor3fv(UColor::black->getFloat());
			m_down->paintIcon(w, 0, size.h - m_down->getIconHeight());
		} else if (m_pushed == Down) {
			glColor3fv(UColor::black->getFloat());
			m_up->paintIcon(w, 0, 0);

			glRecti(0, size.h - m_down->getIconHeight(),
				m_down->getIconWidth(), size.h);

			glColor3fv(UColor::white->getFloat());
			m_down->paintIcon(w, 0, size.h - m_down->getIconHeight());
		} else {
			glColor3fv(UColor::black->getFloat());
			m_up->paintIcon(w, 0, 0);

			m_down->paintIcon(w, 0, size.h - m_down->getIconHeight());
		}

	}*/
}


const std::string &
UBasicScrollBarUI::getLafId() {
	return m_lafId;
}

void
UBasicScrollBarUI::installUI(UWidget * w) {
	UScrollBarUI::installUI(w);

	UScrollBar * scroll = dynamic_cast<UScrollBar*>(w);

	if (scroll->getOrientation() == Vertical) {
		scroll->setLayout(new UBorderLayout(0, 0));

		UButton * north = new UButton(m_up);
		north->sigMousePressed().connect(slot(*this, &UBasicScrollBarUI::fireScrollPosUp));
		north->setBorderPainted(false);
		north->setFocusable(false);
		w->add(north, new UString("north"));

		UButton * south = new UButton(m_down);
		south->sigMousePressed().connect(slot(*this, &UBasicScrollBarUI::fireScrollPosDown));
		south->setBorderPainted(false);
		south->setFocusable(false);
		w->add(south, new UString("south"));

		TrackWidget * button = new TrackWidget(scroll);
		button->setPalette(w->getPalette());
		//button->setColor(w->getUIManager()->getColor("UScrollBar.track"));
		w->add(button, new UString("center"));
	} else {
		scroll->setLayout(new UBorderLayout(0, 0));

		UButton * north = new UButton(m_left);
		north->sigMousePressed().connect(slot(*this, &UBasicScrollBarUI::fireScrollPosUp));
		north->setBorderPainted(false);
		north->setFocusable(false);
		w->add(north, new UString("west"));

		UButton * south = new UButton(m_right);
		south->sigMousePressed().connect(slot(*this, &UBasicScrollBarUI::fireScrollPosDown));
		south->setBorderPainted(false);
		south->setFocusable(false);
		w->add(south, new UString("east"));

		TrackWidget * button = new TrackWidget(scroll);
		button->setPalette(w->getPalette());
		//button->setColor(w->getUIManager()->getColor("UScrollBar.track"));
		w->add(button, new UString("center"));
	}

}

void
UBasicScrollBarUI::uninstallUI(UWidget * w) {
	UScrollBarUI::uninstallUI(w);

	w->setLayout(NULL);
	w->removeAll();
}


UDimension
UBasicScrollBarUI::getPreferredSize(const UWidget * w) {
	UInsets in = w->getInsets();
	UDimension dim;
	dim.w = m_up->getIconWidth() + in.getHorizontal();
	dim.h = m_up->getIconHeight() + in.getVertical();
	return dim;//new UDimension(200, 200);
}


void
UBasicScrollBarUI::fireScrollPosUp(UMouseEvent * e) {
	e->consume();
	m_scrollBar->setValue(m_scrollBar->getValue() + m_scrollBar->getUnitIncrement(Up));
}

void
UBasicScrollBarUI::fireScrollPosDown(UMouseEvent * e) {
	e->consume();
	m_scrollBar->setValue(m_scrollBar->getValue() + m_scrollBar->getUnitIncrement(Down));
}

} // namespace ufo
