/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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

//#include "ufo/ui/uuimanager.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UInternalFrame, UWidget)

UInternalFrame::UInternalFrame()
	: m_rootPane(new URootPane())
	, m_title("")
	, m_frameStyle(FrameTitleBar | FrameCloseBox)
{
	setOpaque(false);
	trackPointer(m_rootPane);
}

UInternalFrame::UInternalFrame(const std::string & title)
	: m_rootPane(new URootPane())
	, m_title(title)
	, m_frameStyle(FrameTitleBar | FrameCloseBox)
{
	setOpaque(false);
	trackPointer(m_rootPane);
}
UInternalFrame::UInternalFrame(uint32_t frameStyle)
	: m_rootPane(new URootPane())
	, m_title("")
	, m_frameStyle(FrameStyle(frameStyle))
{
	setOpaque(false);
	trackPointer(m_rootPane);
}
UInternalFrame::UInternalFrame(const std::string & title, uint32_t frameStyle)
	: m_rootPane(new URootPane())
	, m_title(title)
	, m_frameStyle(FrameStyle(frameStyle))
{
	setOpaque(false);
	trackPointer(m_rootPane);
}


void
UInternalFrame::setVisible(bool b) {
	UWidget::setVisible(b);

	if (isVisible()) {
		ULayeredPane * layeredPane = dynamic_cast<ULayeredPane*>(getParent());
		if (layeredPane) {
			layeredPane->moveToFront(this);
		}
	}
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

void
UInternalFrame::maximize() {
}

void
UInternalFrame::restore() {
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
UInternalFrame::setMaximizable(bool b) {
	if (b) {
		m_frameStyle |= FrameMaximizeBox;
	} else {
		m_frameStyle &= ~FrameMaximizeBox;
	}
}

bool
UInternalFrame::isMaximizable() const {
	return m_frameStyle & FrameMaximizeBox;
}

void
UInternalFrame::setMinimizable(bool b) {
	if (b) {
		m_frameStyle |= FrameMinimizeBox;
	} else {
		m_frameStyle &= ~FrameMinimizeBox;
	}
}

bool
UInternalFrame::isMinimizable() const {
	return m_frameStyle & FrameMinimizeBox;
}

void
UInternalFrame::setClosable(bool b) {
	if (b) {
		m_frameStyle |= FrameCloseBox;
	} else {
		m_frameStyle &= FrameCloseBox;
	}
}

bool
UInternalFrame::isClosable() const {
	return m_frameStyle & FrameCloseBox;
}
