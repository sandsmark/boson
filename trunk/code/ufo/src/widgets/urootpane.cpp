/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/urootpane.cpp
    begin             : Fri May 18 2001
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

#include "ufo/widgets/urootpane.hpp"

#include <algorithm>

#include "ufo/layouts/uboxlayout.hpp"
#include "ufo/widgets/umenubar.hpp"

#include "ufo/widgets/uinternalframe.hpp"
#include "ufo/widgets/ulayeredpane.hpp"
#include "ufo/widgets/udesktoppane.hpp"
#include "ufo/widgets/udockwidget.hpp"

#include "ufo/utoolkit.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(URootPane, UWidget)

namespace ufo {
class URootLayout : public ULayoutManager {
public:
	URootLayout(URootPane * rootPane);
public: // Implements ULayoutManager
	virtual UDimension
	getPreferredLayoutSize(const UWidget * parent, const UDimension & maxSize) const;

	virtual void layoutContainer(const UWidget * parent);
private:  //Private attributes
	URootPane * m_rootPane;
};
}

URootPane::URootPane()
	: UWidget()
	, m_menuBar(NULL)
	, m_contentPane(NULL)
	, m_desktopPane(NULL)
	, m_modalWidget(NULL)
{
	m_context = UToolkit::getToolkit()->getCurrentContext();

	setLayeredPane(createLayeredPane());

	setContentPane(createContentPane());
	setLayout(new URootLayout(this));

	setVisible(true);
}


void
URootPane::setMenuBar(UMenuBar * menuBar) {
	// FIXME: take a closer look on that
	if (m_menuBar && m_menuBar->getParent() &&
		m_menuBar->getParent()->getParent() == m_desktopPane) {
		m_desktopPane->removeDockWidget(m_menuBar);
	}
	m_menuBar = menuBar;

	if (m_menuBar) {
		m_desktopPane->addDockWidget(m_menuBar, TopDockWidgetArea);//, ULayeredPane::RootPaneLayer, 0);
	}
}

UMenuBar *
URootPane::getMenuBar( ) {
	return m_menuBar;
}

UWidget *
URootPane::createContentPane() const {
	UWidget * contentPane = new UWidget();
	contentPane->setCssClass("transparent");
	return contentPane;
}

UWidget *
URootPane::getContentPane() const {
	return m_contentPane;
}

void
URootPane::setContentPane(UWidget * contentPane) {
	if (contentPane) {
		if (m_contentPane && m_contentPane->getParent() == m_desktopPane)
			m_desktopPane->remove(m_contentPane);
		m_contentPane = contentPane;

		m_desktopPane->add(m_contentPane, ULayeredPane::RootPaneLayer);
	}
}


ULayeredPane *
URootPane::createLayeredPane() const {
	ULayeredPane * layeredPane = new UDesktopPane();//new ULayeredPane();
	layeredPane->setOpaque(false);
	return layeredPane;
}

void
URootPane::setLayeredPane(ULayeredPane * layeredPane) {
	if (layeredPane) {
		if (m_desktopPane && m_desktopPane->getParent() == this) {
			remove(m_desktopPane);
		}
		m_desktopPane = dynamic_cast<UDesktopPane*>(layeredPane);

		add(m_desktopPane);
	}
}

ULayeredPane *
URootPane::getLayeredPane() const {
	return m_desktopPane;
}

void
URootPane::addFrame(UInternalFrame * frame) {
	//m_desktopPane->add(frame, ULayeredPane::FrameLayer);
	(dynamic_cast<UDesktopPane*>(m_desktopPane))->addFrame(frame);
}

UInternalFrame *
URootPane::removeFrame(UInternalFrame * frame) {
	//if (m_desktopPane->remove(frame)) {
	if ((dynamic_cast<UDesktopPane*>(m_desktopPane))->removeFrame(frame)) {
		return frame;
	} else {
		return NULL;
	}
}


void
URootPane::moveToFront(UInternalFrame * frame) {
	m_desktopPane->moveToFront(frame);
}

void
URootPane::moveToBack(UInternalFrame * frame) {
	m_desktopPane->moveToBack(frame);
}

void
URootPane::addDockWidget(UDockWidget * w, DockWidgetArea area) {
	m_desktopPane->addDockWidget(w, area);
}

void
URootPane::removeDockWidget(UDockWidget * w) {
	m_desktopPane->removeDockWidget(w);
}


void
URootPane::setModalWidget(UWidget * w) {
	m_modalWidget = w;
}

UWidget *
URootPane::getModalWidget() const {
	return m_modalWidget;
}

//
// Overrides UWidget
//

URootPane *
URootPane::getRootPane(bool topmost) {
	if (topmost) {
		if (getParent()) {
			return getParent()->getRootPane(true);
		}
	}
	return this;
}

void
URootPane::addedToHierarchy() {
	UWidget::addedToHierarchy();
}

UWidget *
URootPane::getVisibleWidgetAt(const UPoint & p) const {
	if (m_modalWidget) {
		return m_modalWidget->getVisibleWidgetAt(p - m_modalWidget->getRootLocation());
	}
	return UWidget::getVisibleWidgetAt(p);
}

//
// class URootPane::URootLayout
//

URootLayout::URootLayout(URootPane * rootPane) :
m_rootPane(rootPane) {}

UDimension
URootLayout::getPreferredLayoutSize(const UWidget * container,
		const UDimension & maxSize) const {
	UDimension contentDim;
	UDimension menuDim;
/*
	if (UMenuBar * mbar = m_rootPane->getMenuBar()) {
		menuDim = mbar->getPreferredSize(maxSize);
	}
	if (UWidget * tDock = m_rootPane->m_topDock) {
		menuDim += tDock->getPreferredSize(maxSize);
	}
	if (UWidget * content = m_rootPane->getContentPane()) {
		contentDim = content->getPreferredSize(maxSize - menuDim);
	}

	return UDimension(
		std::max(contentDim.w, menuDim.w),
		contentDim.h + menuDim.h
	);
	*/
	menuDim = m_rootPane->m_desktopPane->getPreferredSize();
	contentDim = m_rootPane->getContentPane()->getPreferredSize(maxSize - menuDim);

	return menuDim + contentDim;
}

void
URootLayout::layoutContainer(const UWidget * parent) {
	URectangle rect = parent->getInnerBounds();
	//int top = 0;

	m_rootPane->m_desktopPane->setBounds(rect);

	m_rootPane->getContentPane()->setBounds(rect - m_rootPane->m_desktopPane->getContentsInsets());
/*
	if (ULayeredPane * lPane = m_rootPane->getLayeredPane()) {
		lPane->setBounds(0, 0, dim.w, dim.h);
	}

	if (UMenuBar * mbar = m_rootPane->getMenuBar()) {
		const UDimension & menuDim = mbar->getPreferredSize(parent->getSize());
		mbar->setBounds(0, top, dim.w, menuDim.h);
		top += menuDim.h;
	}
	if (UWidget * tDock = m_rootPane->m_topDock) {
		UDimension menuDim = tDock->getPreferredSize(parent->getSize());
		tDock->setBounds(0, top, dim.w, menuDim.h);
		top += menuDim.h;
	}
	if (UWidget * content = m_rootPane->getContentPane()) {
		content->setBounds(0, top, dim.w, dim.h - top);
	}*/
}
