/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasicsliderui.cpp
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

#include "ufo/ui/basic/ubasicsliderui.hpp"

#include "ufo/ui/uuimanager.hpp"
#include "ufo/ui/ustyle.hpp"
#include "ufo/uicon.hpp"
#include "ufo/ugraphics.hpp"
#include "ufo/ucontext.hpp"

#include "ufo/widgets/uslider.hpp"
#include "ufo/font/ufont.hpp"

#include "ufo/events/umouseevent.hpp"
#include "ufo/layouts/uborderlayout.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicSliderUI, USliderUI)


std::string UBasicSliderUI::m_lafId("USlider");

UBasicSliderUI::UBasicSliderUI(USlider * slider)
	: m_slider(slider)
	, m_rect()
	, m_isDragging(false)
{}

UBasicSliderUI *
UBasicSliderUI::createUI(UWidget * w) {
	return new UBasicSliderUI(dynamic_cast<USlider*>(w));
}

void
UBasicSliderUI::paint(UGraphics * g, UWidget * w) {
	UWidgetUI::paint(g, w);

	USlider * slider;
	const UInsets & insets = w->getInsets();

	slider = dynamic_cast<USlider *>(w);

	UStyle * style = w->getUIManager()->getStyle();

	UPoint left(5, w->getHeight() / 2);
	UPoint right(w->getWidth() - 5, left.y);
	g->setColor(w->getColorGroup().baseFore());
	g->drawLine(left, right);

	style->paintScrollTrack(g, slider, m_rect, m_slider->isActive(), m_isDragging);
}

void
UBasicSliderUI::installUI(UWidget * w) {
	USliderUI::installUI(w);
/*
	w->setLayout(new UBorderLayout(0, 0));

	TrackWidget * button = new TrackWidget(m_slider);
	button->setPalette(w->getPalette());
	//button->setColor(w->getUIManager()->getColor("UScrollBar.track"));
	w->add(button, new UString("center"));
*/
	if (w->getSize().isEmpty()) {
		w->setSize(w->getPreferredSize());
	}
	m_slider->sigMousePressed().connect(slot(*this, &UBasicSliderUI::mousePressed));
	m_slider->sigMouseDragged().connect(slot(*this, &UBasicSliderUI::mouseDragged));
	w->setEventState(UEvent::MouseMoved, true);

	m_slider->sigValueChanged().connect(slot(*this, &UBasicSliderUI::valueChanged));

	valueChanged(m_slider, m_slider->getValue());
}

void
UBasicSliderUI::uninstallUI(UWidget * w) {
	USliderUI::uninstallUI(w);

	w->setLayout(NULL);
	//w->removeAll();
}

const std::string &
UBasicSliderUI::getLafId() {
	return m_lafId;
}



UDimension
UBasicSliderUI::getPreferredSize(const UWidget * w) {
	const USlider * slider = dynamic_cast<const USlider *>(w);

	if (m_slider->getOrientation() == Vertical) {
		return UDimension(10, 110);
	} else {
		return UDimension(110, 10);
	}
}


void
UBasicSliderUI::valueChanged(USlider * slider, int newValue) {
	int width = 10;//m_slider->getVisibleAmount();
	int max = m_slider->getMaximum();

	float widthDelta = float(m_slider->getWidth() - width) / max;
	float heightDelta = float(m_slider->getHeight() - width) / max;

	if (m_slider->getOrientation() == Vertical) {
		m_rect.setBounds(0, int(newValue * heightDelta),
			width, width);
	} else {
		m_rect.setBounds(int(newValue * widthDelta), 0,
			width, width);
	}
}

void
UBasicSliderUI::mousePressed(UMouseEvent * e) {
	e->consume();
	UPoint pos = e->getLocation();
	if (m_rect.contains(pos)) {
		// dragging
		//m_slider->getContext()->setEventGrabber(slot(*this, &UBasicSliderUI::eventGrabber));
		m_isDragging = true;
		m_mousePress = m_rect.getLocation() - e->getLocation();
	} else {
		m_isDragging = false;
		// block scrolling
		int value = m_slider->getValue();
		int delta = 0;
		if (m_slider->getOrientation() == Vertical) {
			if (pos.y <= m_rect.y) {
				delta = - m_slider->getBlockIncrement();//Up)
			} else {
				delta = m_slider->getBlockIncrement();//Down)
			}
		} else {
			if (pos.x <= m_rect.x) {
				delta = - m_slider->getBlockIncrement();//Up)
			} else {
				delta = m_slider->getBlockIncrement();//Down)
			}
		}
		if (delta) {
			m_slider->setValue(value + delta);
		}
	}
}

void
UBasicSliderUI::mouseDragged(UMouseEvent * e) {
	// clip
	if (!m_isDragging){
		return;
	}
	e->consume();
	UPoint pos(m_rect.getLocation() - e->getLocation());
	UPoint rel(m_mousePress - pos);

	float delta;
	if (m_slider->getOrientation() == Vertical) {
		delta = (rel.y *
			float(m_slider->getMaximum()) / (m_slider->getHeight() - m_rect.h));
	} else {
		delta = (rel.x *
			float(m_slider->getMaximum()) / (m_slider->getWidth() - m_rect.w));
	}
	int idelta = 0;
	// rounding
	if (delta > 0) {
		idelta = int(delta + 0.5f);
	} else {
		idelta = int(delta - 0.5f);
	}
	m_slider->setValue(m_slider->getValue() + idelta);
}
