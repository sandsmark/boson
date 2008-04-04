/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/udesktoppane.cpp
    begin             : Mon Dec 27 2004
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

#include "ufo/widgets/udesktoppane.hpp"

#include "ufo/widgets/uinternalframe.hpp"
#include "ufo/widgets/umenu.hpp"
#include "ufo/widgets/ubutton.hpp"
#include "ufo/widgets/ulabel.hpp"
#include "ufo/widgets/udockwidget.hpp"

//#include "ufo/ui/uuimanager.hpp"

#include "ufo/layouts/uborderlayout.hpp"
#include "ufo/layouts/uboxlayout.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontmetrics.hpp"

#include "ufo/ucontext.hpp"
#include "ufo/events/umouseevent.hpp"

#include "ufo/ugraphics.hpp"

#include "ufo/ui/ustyle.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UDesktopPane, ULayeredPane)

UDesktopPane::UDesktopPane()
	: m_topDock(new UWidget())
	, m_leftDock(new UWidget())
	, m_bottomDock(new UWidget())
	, m_rightDock(new UWidget())
{
	m_topDock->setCssClass("transparent");
	m_leftDock->setCssClass("transparent");
	m_bottomDock->setCssClass("transparent");
	m_rightDock->setCssClass("transparent");

	m_topDock->setLayout(new UBoxLayout(Vertical, 0, 0));
	m_bottomDock->setLayout(new UBoxLayout(Vertical, 0, 0));
	m_leftDock->setLayout(new UBoxLayout(Horizontal, 0, 0));
	m_rightDock->setLayout(new UBoxLayout(Horizontal, 0, 0));

	add(m_topDock, RootPaneLayer);
	add(m_bottomDock, RootPaneLayer);
	add(m_leftDock, RootPaneLayer);
	add(m_rightDock, RootPaneLayer);

	m_topDock->setHorizontalAlignment(AlignStretch);
	m_bottomDock->setHorizontalAlignment(AlignStretch);
	m_leftDock->setVerticalAlignment(AlignStretch);
	m_rightDock->setVerticalAlignment(AlignStretch);
}

UDesktopPane::~UDesktopPane() {}

void
UDesktopPane::addFrame(UInternalFrame * frame) {
	add(frame, ULayeredPane::FrameLayer);
	raise(frame);
	frame->m_frameState |= FrameActive;
}

bool
UDesktopPane::removeFrame(UInternalFrame * frame) {
	return remove(frame);
}

void
UDesktopPane::addDockWidget(UDockWidget * w, DockWidgetArea area) {
	switch (area) {
		case TopDockWidgetArea:
			m_topDock->add(w);
			w->setOrientation(Horizontal);
		break;
		case LeftDockWidgetArea:
			m_leftDock->add(w);
			w->setOrientation(Vertical);
		break;
		case BottomDockWidgetArea:
			m_bottomDock->add(w);
			w->setOrientation(Horizontal);
		break;
		case RightDockWidgetArea:
			m_rightDock->add(w);
			w->setOrientation(Vertical);
		break;
		case AllDockWidgetAreas:
		// FIXME: what does this mean?
		// seems to be non-sense here
		break;
	}
}

void
UDesktopPane::removeDockWidget(UDockWidget * w) {
	m_topDock->remove(w);
	m_leftDock->remove(w);
	m_bottomDock->remove(w);
	m_rightDock->remove(w);
}

void
UDesktopPane::maximize(UInternalFrame * frame) {
	int frameState = frame->getFrameState();

	if (!(frameState & FrameMinimized)) {
		frame->m_restoreBounds = frame->getBounds();
	}
	frameState &= ~FrameMinimized;
	frameState |= FrameMaximized;
	frame->setBounds(0, 0, getWidth(), getHeight());
	frame->m_frameState = frameState;
}

bool
UDesktopPane::isMaximized(UInternalFrame * frame) {
	return (frame->m_frameState & FrameMaximized);
}

void
UDesktopPane::minimize(UInternalFrame * frame) {
	int frameState = frame->getFrameState();
	if (!(frameState & FrameMaximized)) {
		frame->m_restoreBounds = frame->getBounds();
	}
	frameState &= ~FrameMaximized;

	if (false) {//hasTaskBar()) {
	} else {
		frameState |= FrameMinimized;
		frame->setBounds(0, getHeight() - 20, 100, 20);
		frame->m_frameState = frameState;
	}
}

bool
UDesktopPane::isMinimized(UInternalFrame * frame) {
	return (frame->m_frameState & FrameMinimized);
}

void
UDesktopPane::restore(UInternalFrame * frame) {
	frame->setBounds(frame->m_restoreBounds);
	int frameState = frame->getFrameState();
	frameState &= ~FrameMaximized;
	frameState &= ~FrameMinimized;
	frame->m_frameState = frameState;
}

void
UDesktopPane::raise(UInternalFrame * frame) {
	if (getPosition(frame) == 0) {
		return;
	}
	bool topmost = true;
	// check for stays on top
	// FIXME: checks only for the top most
	unsigned int first = getLayerBegin(getLayer(frame));
	if (first >= 0 && first < getWidgetCount()) {
		if (UInternalFrame * tempFrame = dynamic_cast<UInternalFrame*>(getWidget(first))) {
			if (tempFrame->getFrameState() & FrameModal ||
					tempFrame->getFrameState() & FrameStaysOnTop) {
				topmost = false;
			}
		}
	}
	if (topmost) {
		moveToFront(frame);
	} else {
		setPosition(frame, 1);
	}
}

void
UDesktopPane::lower(UInternalFrame * frame) {
	moveToBack(frame);
}

bool
UDesktopPane::isActive(UInternalFrame * frame) {
	if (getIndexOf(frame) == getLayerBegin(ULayeredPane::FrameLayer->toInt())) {
		return true;
	}
	return false;
}

void
UDesktopPane::dragDockWidget(UDockWidget * w, const UPoint & pos) {
}

void
UDesktopPane::dropDockWidget(UDockWidget * w, const UPoint & pos) {
	if (pos.y < 20) {
		moveDockWidget(w, TopDockWidgetArea);
	} else if (pos.y > getHeight() - 20) {
		moveDockWidget(w, BottomDockWidgetArea);
	} else if (pos.x < 20) {
		moveDockWidget(w, LeftDockWidgetArea);
	} else if (pos.x > getWidth() - 20) {
		moveDockWidget(w, RightDockWidgetArea);
	}
}

void
UDesktopPane::close(UInternalFrame * frame) {
	removeFrame(frame);
}

void
UDesktopPane::setTitle(UInternalFrame * frame, const std::string & title) {
	frame->m_title = title;
	//w->put("frame_title", title);
}

std::string
UDesktopPane::getTitle(UInternalFrame * frame) {
	return frame->m_title;
	//return w->getString("frame_title");
}

UInsets
UDesktopPane::getContentsInsets() const {
	(const_cast<UDesktopPane*>(this))->validateSelf();
	UInsets ret;
	ret.top = m_topDock->getHeight();
	ret.left = m_leftDock->getWidth();
	ret.bottom = m_bottomDock->getHeight();
	ret.right = m_rightDock->getWidth();
	return ret;
}

void
UDesktopPane::moveDockWidget(UDockWidget * frame, DockWidgetArea newArea) {
}

void
UDesktopPane::validateSelf() {
	UWidget::validateSelf();

	URectangle rect = getInnerBounds();

	UDimension topDim = m_topDock->getPreferredSize(rect.getSize());
	UDimension leftDim = m_leftDock->getPreferredSize(rect.getSize());
	UDimension bottomDim = m_bottomDock->getPreferredSize(rect.getSize());
	UDimension rightDim = m_rightDock->getPreferredSize(rect.getSize());

	m_topDock->setBounds(0, 0, rect.w, topDim.h);
	m_leftDock->setBounds(0, rect.y + topDim.h, leftDim.w, rect.h - topDim.h - bottomDim.h);
	m_bottomDock->setBounds(0, rect.y + rect.h - bottomDim.h, rect.w, bottomDim.h);
	m_rightDock->setBounds(rect.x + rect.w - rightDim.w, rect.y + topDim.h, rightDim.w, rect.h - topDim.h - bottomDim.h);
}

void
UDesktopPane::processWidgetEvent(UWidgetEvent * e) {
	switch (e->getType()) {
		case UEvent::WidgetAdded:
			getContext()->connectListener(slot(
				*this, &UDesktopPane::eventListener)
			);
		break;
		case UEvent::WidgetRemoved:
			getContext()->disconnectListener(slot(
				*this, &UDesktopPane::eventListener)
			);
		break;
		default:
		break;
	}
}

void
UDesktopPane::eventListener(UEvent * e) {
	switch (e->getType()) {
		case UEvent::MousePressed:
		{
			UMouseEvent * me = dynamic_cast<UMouseEvent*>(e);
			if (!me) { return; }
			// check all frames if they contain the mouse event
			UInternalFrame * frame = NULL;
			int begin = getLayerBegin(ULayeredPane::FrameLayer->toInt());
			int end = getLayerEnd(ULayeredPane::FrameLayer->toInt());
			for (int i = begin; i < end; ++i) {
				if (getWidget(i)->isVisible()) {
					if (getWidget(i)->getRootBounds().contains(me->getRootLocation())) {
						frame = dynamic_cast<UInternalFrame*>(getWidget(i));
						// First widget is top most widget
						break;
					}
				}
			}
			if (frame) {
				raise(frame);
			}
		}
		break;
		default:
		break;
	}
}

UDimension
UDesktopPane::getContentsSize(const UDimension & maxSize) const {
	UDimension topDim = m_topDock->getPreferredSize(maxSize);
	UDimension leftDim = m_leftDock->getPreferredSize(maxSize);
	UDimension bottomDim = m_bottomDock->getPreferredSize(maxSize);
	UDimension rightDim = m_rightDock->getPreferredSize(maxSize);

	return UDimension(
		std::max(std::max(leftDim.w, rightDim.w), std::max(topDim.w, bottomDim.w)),
		std::max(leftDim.h, rightDim.h) +  std::max(topDim.h, bottomDim.h)
	);
}
