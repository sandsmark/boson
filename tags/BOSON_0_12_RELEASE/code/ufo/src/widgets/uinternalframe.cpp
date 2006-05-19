/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/uinternalframe.cpp
    begin             : Fri Jun 1 2001
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

#include "ufo/widgets/uinternalframe.hpp"

#include "ufo/widgets/urootpane.hpp"
#include "ufo/widgets/ulayeredpane.hpp"
#include "ufo/widgets/udesktoppane.hpp"

#include "ufo/layouts/uborderlayout.hpp"

#include "ufo/events/umouseevent.hpp"

#include "ufo/ui/ustyle.hpp"
#include "ufo/umodel.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UInternalFrame, UWidget)

UStyle::SubControls
frameStyleToSubControl(int frameStyle) {
	int ret = 0;
	if (frameStyle & FrameTitleBar) {
		ret |= UStyle::SC_TitleBarLabel;
	}
	if (frameStyle & FrameSysMenu) {
		ret |= UStyle::SC_TitleBarSysMenu;
	}
	if (frameStyle & FrameMinimizeBox) {
		ret |= UStyle::SC_TitleBarMinButton;
	}
	if (frameStyle & FrameMaximizeBox) {
		ret |= UStyle::SC_TitleBarMaxButton;
	}
	if (frameStyle & FrameCloseBox) {
		ret |= UStyle::SC_TitleBarCloseButton;
	}

	return UStyle::SubControls(ret);
}

class _TitleBar : public UWidget {
	UFO_STYLE_TYPE(UStyle::CE_TitleBar)
public:
	_TitleBar(UInternalFrame * frame) : m_frame(frame) {
	 	UTitleBarModel * m = new UTitleBarModel();
		m->widgetState = m_model->widgetState;
		m->subControls = frameStyleToSubControl(m_frame->getFrameStyle());
		m->activeSubControls = UStyle::SC_None;
		m->text = m_frame->getTitle();
		m->icon = NULL;
		m->frameState = m_frame->getFrameState();
		m->frameStyle = m_frame->getFrameStyle();
		delete (m_model);
		m_model = m;
	}
protected: // Protected
	virtual void paintWidget(UGraphics * g);
	virtual void processMouseEvent(UMouseEvent * e);
	UDimension getContentsSize(const UDimension & maxSize) const;
private:
	UInternalFrame * m_frame;
};

UInternalFrame::UInternalFrame()
	: m_desktop(NULL)
	, m_rootPane(new URootPane())
	, m_title("")
	, m_frameStyle(FrameTitleBar | FrameCloseBox | FrameResizable)
	, m_frameState(0)
{
	setCssType("internalframe");
	setCssClass("transparent");
	setLayout(new UBorderLayout(0, 0));
	add(new _TitleBar(this), UBorderLayout::North);
	add(m_rootPane);
}

UInternalFrame::UInternalFrame(const std::string & title)
	: m_desktop(NULL)
	, m_rootPane(new URootPane())
	, m_title(title)
	, m_frameStyle(FrameTitleBar | FrameCloseBox | FrameResizable)
	, m_frameState(0)
{
	setCssType("internalframe");
	setCssClass("transparent");
	setLayout(new UBorderLayout(0, 0));
	add(new _TitleBar(this), UBorderLayout::North);
	add(m_rootPane);
}
UInternalFrame::UInternalFrame(uint32_t frameStyle)
	: m_desktop(NULL)
	, m_rootPane(new URootPane())
	, m_title("")
	, m_frameStyle(FrameStyle(frameStyle))
	, m_frameState(0)
{
	setCssType("internalframe");
	setCssClass("transparent");
	setLayout(new UBorderLayout(0, 0));
	add(new _TitleBar(this), UBorderLayout::North);
	add(m_rootPane);
}
UInternalFrame::UInternalFrame(const std::string & title, uint32_t frameStyle)
	: m_desktop(NULL)
	, m_rootPane(new URootPane())
	, m_title(title)
	, m_frameStyle(FrameStyle(frameStyle))
	, m_frameState(0)
{
	setCssType("internalframe");
	setCssClass("transparent");
	setLayout(new UBorderLayout(0, 0));
	add(new _TitleBar(this), UBorderLayout::North);
	add(m_rootPane);
}


bool
UInternalFrame::isActive() const {
	ULayeredPane * layeredPane = dynamic_cast<ULayeredPane*>(getParent());
	if (layeredPane) {
		return (layeredPane->getPosition(this) == 0);
	}
	return false;
}

//
// public methods
//

URootPane *
UInternalFrame::getRootPane() const {
	return m_rootPane;
}

UWidget *
UInternalFrame::getContentPane() const {
	return m_rootPane->getContentPane();
}

ULayeredPane *
UInternalFrame::getLayeredPane() const {
	return m_rootPane->getLayeredPane();
}

void
UInternalFrame::setTitle(const std::string & title) {
	m_title = title;
}
std::string
UInternalFrame::getTitle() {
	return m_title;
}

void
UInternalFrame::pack() {
	setSize(getPreferredSize());
}

FrameStyle
UInternalFrame::getFrameStyle() const {
	return FrameStyle(m_frameStyle);
}

void
UInternalFrame::setFrameStyle(int frameStyle) {
	m_frameStyle = frameStyle;
}

FrameState
UInternalFrame::getFrameState() const {
	return FrameState(m_frameState);
}

void
UInternalFrame::setFrameState(int frameState) {
	m_frameState = FrameState(frameState);
}

void
UInternalFrame::maximize() {
	if (m_desktop) {
		m_desktop->maximize(this);
	}
}

bool
UInternalFrame::isMaximized() const {
	return (m_frameState & FrameMaximized);
}

void
UInternalFrame::minimize() {
	if (m_desktop) {
		m_desktop->minimize(this);
	}
}

bool
UInternalFrame::isMinimized() const {
	return (m_frameState & FrameMinimized);
}

void
UInternalFrame::restore() {
	if (m_desktop) {
		m_desktop->restore(this);
	}
}

void
UInternalFrame::setResizable(bool b) {
	if (b) {
		m_frameStyle |= FrameResizable;
	} else {
		m_frameStyle &= ~FrameResizable;
	}
}

bool
UInternalFrame::isResizable() const {
	return m_frameStyle & FrameResizable;
}

void
UInternalFrame::processMouseEvent(UMouseEvent * e) {
	if (!isResizable() || e->getSource() != this) {
		UWidget::processMouseEvent(e);
		return;
	}
	static int horizontal = 0;
	static int vertical = 0;
	switch (e->getType()) {
		case UEvent::MousePressed: {
			e->consume();
			UPoint pos(e->getLocation());
			if (pos.x < 5) {
				horizontal = Left;
			}
			if (pos.x > getWidth() - 5) {
				horizontal = Right;
			}
			if (pos.y < 5) {
				vertical = Up;
			}
			if (pos.y > getHeight() - 5) {
				vertical = Down;
			}
		}
		break;
		case UEvent::MouseDragged: {
			e->consume();
			URectangle newBounds(getBounds());
			if (vertical == Up) {
				newBounds.y += e->getRelMovement().y;
				newBounds.h -= e->getRelMovement().y;
			} else if (vertical == Down) {
				newBounds.h += e->getRelMovement().y;
			}
			if (horizontal == Left) {
				newBounds.x += e->getRelMovement().x;
				newBounds.w -= e->getRelMovement().x;
			} else if (horizontal == Right) {
				newBounds.w += e->getRelMovement().x;
			}
			if (newBounds != getBounds()) {
				setBounds(newBounds);
			}
		}
		break;
		case UEvent::MouseReleased:
			e->consume();
			horizontal = 0;
			vertical = 0;
		break;
		default:
		break;
	}
	UWidget::processMouseEvent(e);
}

void
UInternalFrame::processWidgetEvent(UWidgetEvent * e) {
	// FIXME: should we use signals & slots?
	switch (e->getType()) {
		case UEvent::WidgetZOrderChanged:
			if (m_desktop && m_desktop->isActive(this)) {
				m_frameState |= FrameActive;
			} else {
				m_frameState &= ~FrameActive;
			}
		break;
		case UEvent::WidgetAdded:
			m_desktop = dynamic_cast<UDesktopPane*>(getParent());
		break;
		case UEvent::WidgetRemoved:
			m_desktop = NULL;
		break;
		default:
		break;
	}
	UWidget::processWidgetEvent(e);
}

void
UInternalFrame::processStateChangeEvent(uint32_t state) {
	if (state & WidgetVisible) {
		if (isVisible()) {
			if (m_frameState & FrameModal) {
				UWidget::getRootPane(true)->setModalWidget(this);
			}
		} else if (m_frameState & FrameModal) {
			UWidget::getRootPane(true)->setModalWidget(NULL);
		}
	}
	UWidget::processStateChangeEvent(state);
}

UDimension
UInternalFrame::getContentsSize(const UDimension & maxSize) const {
	return UWidget::getContentsSize(maxSize);
}

void
_TitleBar::paintWidget(UGraphics * g) {
	UTitleBarModel * model = static_cast<UTitleBarModel*>(m_model);
	model->text = m_frame->getTitle();
	model->frameState = m_frame->getFrameState();
	model->frameStyle = m_frame->getFrameStyle();
	model->subControls = frameStyleToSubControl(m_frame->getFrameStyle());

	getStyle()->paintComponent(g, UStyle::CE_TitleBar, getInnerBounds(), getStyleHints(), model, this);
}

void
_TitleBar::processMouseEvent(UMouseEvent * e) {
	UTitleBarModel * model = static_cast<UTitleBarModel*>(m_model);
	model->text = m_frame->getTitle();
	model->frameState = m_frame->getFrameState();
	model->frameStyle = m_frame->getFrameStyle();
	model->subControls = frameStyleToSubControl(m_frame->getFrameStyle());

	UStyle::SubControls subctrl = getStyle()->getSubControlAt(
		UStyle::CE_TitleBar, getSize(), getStyleHints(),
		model, e->getLocation()
	);
	UDesktopPane * desktop = dynamic_cast<UDesktopPane*>(m_frame->getParent());
	switch (e->getType()) {
		case UEvent::MousePressed: {
			model->activeSubControls = UStyle::SC_None;
			switch (subctrl) {
				case UStyle::SC_TitleBarSysMenu:
				break;
				case UStyle::SC_TitleBarMinButton:
					e->consume();
					model->activeSubControls = UStyle::SC_TitleBarMinButton;
				break;
				case UStyle::SC_TitleBarMaxButton:
					e->consume();
					model->activeSubControls = UStyle::SC_TitleBarMaxButton;
				break;
				case UStyle::SC_TitleBarCloseButton:
					e->consume();
					model->activeSubControls = UStyle::SC_TitleBarCloseButton;
				break;
				case UStyle::SC_TitleBarLabel:
				default:
				break;
			}
		}
		break;
		case UEvent::MouseReleased: {
			switch (subctrl) {
				case UStyle::SC_TitleBarCloseButton:
					if (model->activeSubControls & UStyle::SC_TitleBarCloseButton) {
						if (desktop) {
							desktop->lower(m_frame);
						}
						m_frame->setVisible(false);
					}
				break;
				case UStyle::SC_TitleBarMaxButton:
					if (model->activeSubControls & UStyle::SC_TitleBarMaxButton) {
						if (desktop) {
							if (m_frame->isMaximized()) {
								desktop->restore(m_frame);
							} else {
								desktop->maximize(m_frame);
							}
						}
					}
				break;
				case UStyle::SC_TitleBarMinButton:
					if (model->activeSubControls & UStyle::SC_TitleBarMinButton) {
						if (desktop) {
							if (m_frame->isMinimized()) {
								desktop->restore(m_frame);
							} else {
								desktop->minimize(m_frame);
							}
						}
					}
				break;
				default:
				break;
			}
			repaint();
			model->activeSubControls = UStyle::SC_None;
		}
		break;
		case UEvent::MouseDragged: {
			if (model->activeSubControls == UStyle::SC_None) {
				e->consume();
				if (!m_frame->isMaximized()) {
					UPoint pos = m_frame->getLocation();
					pos += e->getRelMovement();
					m_frame->setLocation(pos);
				}
			}
		}
		break;
		default:
		break;
	}
	UWidget::processMouseEvent(e);
}

UDimension
_TitleBar::getContentsSize(const UDimension & maxSize) const {
	return UDimension(50, 20);
}
