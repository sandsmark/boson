/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasicinternalframeui.cpp
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

#include "ufo/ui/basic/ubasicinternalframeui.hpp"

//#include "ufo/ufo_gl.hpp"

#include "ufo/ucontext.hpp"
#include "ufo/ugraphics.hpp"

#include "ufo/ui/uuimanager.hpp"

#include "ufo/util/ucolor.hpp"
#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontmetrics.hpp"

#include "ufo/widgets/urootpane.hpp"
#include "ufo/widgets/ulayeredpane.hpp"
#include "ufo/widgets/uinternalframe.hpp"
#include "ufo/widgets/ubutton.hpp"

#include "ufo/layouts/uborderlayout.hpp"

// internal frame bar
#include "ufo/widgets/ubutton.hpp"
#include "ufo/widgets/ulabel.hpp"
#include "ufo/widgets/umenu.hpp"
#include "ufo/widgets/umenuitem.hpp"

#include "ufo/layouts/uborderlayout.hpp"
#include "ufo/layouts/uboxlayout.hpp"
#include "ufo/layouts/uflowlayout.hpp"

#include "ufo/events/uactionevent.hpp"
#include "ufo/events/umouseevent.hpp"


using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UBasicInternalFrameUI, UInternalFrameUI)

//

std::string UBasicInternalFrameUI::m_lafId("UInternalFrame");

UBasicInternalFrameUI::UBasicInternalFrameUI(UInternalFrame * frame)
	: m_frame(frame)
	, m_titleBar(NULL)
{}

UBasicInternalFrameUI *
UBasicInternalFrameUI::createUI(UWidget * w) {
	return new UBasicInternalFrameUI(dynamic_cast<UInternalFrame *>(w));
}

void
UBasicInternalFrameUI::installUI(UWidget * w) {
	UWidgetUI::installUI(w);
	//frame = (UInternalFrame *) w;
	//if ( !frame.equals(frame) ) return:

	m_frame->setLayout(new UBorderLayout(0, 0));
/*

	InternalFrameTitleBar * title = createTitleBar(m_frame);

	// dragging support
	title->sigMousePressed().connect(slot(*title, &InternalFrameTitleBar::mousePressed));
	title->sigMouseDragged().connect(slot(*title, &InternalFrameTitleBar::mouseDragged));

	setTitleBar(title);

	// we need validating title bar because we need the valid size to
	// validate the internal frame
	// remember that this functions is called during the validation
	// of the internal frame
	//title->validate();
	title->setPalette(w->getPalette());
*/
	m_frame->add(m_frame->getRootPane(), UBorderLayout::Center);
}

void
UBasicInternalFrameUI::uninstallUI(UWidget * w) {
	if (!w->equals(m_frame)) {
		// throw an exception ??
		return ;
	}
	UWidgetUI::uninstallUI(w);/*

	m_titleBar->sigMousePressed().disconnect(slot(*m_titleBar, &InternalFrameTitleBar::mousePressed));
	m_titleBar->sigMouseDragged().disconnect(slot(*m_titleBar, &InternalFrameTitleBar::mouseDragged));

	setTitleBar(NULL);*/
	m_frame->remove(m_frame->getRootPane());
}

const std::string &
UBasicInternalFrameUI::getLafId() {
	return m_lafId;
}


void
UBasicInternalFrameUI::setTitleBar(InternalFrameTitleBar * titleBar) {
	if (m_titleBar) {
		m_frame->remove(m_titleBar);
		m_titleBar = NULL;
	}
	if (titleBar) {
		m_titleBar = titleBar;
		m_frame->add(m_titleBar, UBorderLayout::North);
	}
}

InternalFrameTitleBar *
UBasicInternalFrameUI::createTitleBar(UInternalFrame * frame) {
	return new InternalFrameTitleBar(frame);
}



//
// class InternalFrameTitleBar
//

InternalFrameTitleBar::InternalFrameTitleBar(UInternalFrame * frame)
		: m_frame(frame)
{}

void
InternalFrameTitleBar::closeFrame(UActionEvent * e) {
	e->consume();
	//if (e->getActionCommand() == "hide internal frame") {
		m_frame->setVisible(false);
	//}
}

void
InternalFrameTitleBar::mousePressed(UMouseEvent * e) {
	e->consume();
	getRootPane(true)->getLayeredPane()->moveToFront(getParent());
	getParent()->repaint();
}
void
InternalFrameTitleBar::mouseDragged(UMouseEvent * e) {
	e->consume();
	//if (e->getType() == UEvent::MouseDragged) {
		UPoint pos = m_frame->getRootLocation();
		m_frame->setLocation(pos.x + e->getXRel(), pos.y + e->getYRel());
	//}
}

void
InternalFrameTitleBar::addedToHierarchy() {
	UWidget::addedToHierarchy();
	removeAll();
	//setLayout(createLayout());
	setLayout(new UBorderLayout());
	// check for the frame style and decide which buttons are needed
	int frameStyle = m_frame->getFrameStyle();
	if (frameStyle & FrameNoBorder) {
		return;
	}

	UUIManager * manager = getUIManager();

	UBoxLayout * layout = new UBoxLayout();

	UWidget * westPane = new UWidget();
	westPane->setLayout(layout);
	westPane->setOpaque(false);
	UWidget * centerPane = new UWidget();
	centerPane->setLayout(layout);
	centerPane->setOpaque(false);
	UWidget * eastPane = new UWidget();
	eastPane->setLayout(layout);
	eastPane->setOpaque(false);

	// set default frame menu

	if (frameStyle & FrameSysMenu) {
		defaultMenu = new UMenu(dynamic_cast<UIcon*>(manager->get("UInternalFrame.icon")));

		UMenuItem * closeItem = new UMenuItem("Close");
		defaultMenu->add(closeItem);
		closeItem->sigButtonClicked().connect(
			slot(*this, &InternalFrameTitleBar::closeFrame)
		);

		defaultMenu->setOpaque(false);
		defaultMenu->setBorderPainted(false);
		defaultMenu->setActionCommand("open internal frame menu");
		westPane->add(defaultMenu);
	}

	// set top right button bar

	//UBorder * border = new ULineBorder();
	// minimize button
	if (frameStyle & FrameMinimizeBox) {
		UButton * minmizeBox = new UButton(
			dynamic_cast<UIcon*>(manager->get("UInternalFrame.minimizeIcon"))
		);
		minmizeBox->setBorder(LineBorder);//border);
		minmizeBox->setActionCommand("minimize internal frame");
		eastPane->add(minmizeBox);
	}

	// maximize button
	if (frameStyle & FrameMaximizeBox) {
		UButton * maxmizeBox = new UButton(
			dynamic_cast<UIcon*>(manager->get("UInternalFrame.maximizeIcon"))
		);
		maxmizeBox->setBorder(LineBorder);//border);
		maxmizeBox->setActionCommand("maximize internal frame");
		eastPane->add(maxmizeBox);
	}

	// close button
	if (frameStyle & FrameCloseBox) {
		close = new UButton(dynamic_cast<UIcon*>(manager->get("UInternalFrame.closeIcon")));

		close->sigButtonClicked().connect(
			slot(*this, &InternalFrameTitleBar::closeFrame)
		);
		close->setBorder(LineBorder);//border);
		close->setActionCommand("hide internal frame");
		eastPane->add(close);//, InternalFrameTitleBarLayout::WEST);
	}
	// assure size
	if (frameStyle & FrameTitleBar) {
		UDimension size;
		size.w = getFont()->getFontMetrics()->getStringWidth(m_frame->getTitle());
		size.h = getFont()->getFontMetrics()->getHeight();
		centerPane->setPreferredSize(size);
	}

	add(westPane, UBorderLayout::West);
	add(centerPane, UBorderLayout::Center);
	add(eastPane, UBorderLayout::East);
}

void
InternalFrameTitleBar::removedFromHierarchy() {
	removeAll();
}

void
InternalFrameTitleBar::paintWidget(UGraphics * g) {
	URectangle bounds = getInnerBounds();

	// draw background
	g->setColor(m_frame->getColorGroup().background());
	g->fillRect(bounds.x, bounds.y, bounds.w, bounds.h);

	// draw a small border line
	g->setColor(getColorGroup().background().darker());
	g->drawLine(
		bounds.x, bounds.y + bounds.h - 1,
		bounds.x + bounds.w - 1, bounds.y + bounds.h - 1
	);

	// draw title
	if (m_frame->getFrameStyle() & FrameTitleBar) {
		g->setColor(m_frame->getColorGroup().foreground());
		g->drawString(m_frame->getTitle(), bounds.x + 20, 1);
	}
}
