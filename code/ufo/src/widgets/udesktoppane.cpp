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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/widgets/udesktoppane.hpp"

#include "ufo/widgets/uinternalframe.hpp"
#include "ufo/widgets/uframedecoration.hpp"
#include "ufo/widgets/umenu.hpp"
#include "ufo/widgets/ubutton.hpp"
#include "ufo/widgets/ulabel.hpp"

#include "ufo/ui/uuimanager.hpp"

#include "ufo/layouts/uborderlayout.hpp"
#include "ufo/layouts/uboxlayout.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontmetrics.hpp"

#include "ufo/ucontext.hpp"
#include "ufo/events/umouseevent.hpp"

#include "ufo/ugraphics.hpp"

using namespace ufo;

class _UFrameDecoration;

class _UTitleWidget : public UWidget {
public:
	_UTitleWidget(_UFrameDecoration * deco) : m_deco(deco) {}
protected:
	virtual void paintWidget(UGraphics * g);
private:
	_UFrameDecoration * m_deco;
};

class _UFrameDecoration : public UFrameDecoration {
public:
	_UFrameDecoration(UDesktopPane * desktop)
		: m_desktop(desktop)
		, m_window(NULL)
		, m_frame(NULL)
		, m_frameStyle(0)
		, m_title("")
		, m_titleBar(NULL)
		, defaultMenu(NULL)
		, close(NULL)
	{}
	virtual void install(UWidget * w);
	virtual void uninstall(UWidget * w);
	virtual UWidget * getFrame() const { return m_frame; }
	virtual std::string getTitle() const {
		return m_window->getString("frame_title");
	}
	virtual uint32_t getFrameStyle() const { return m_frameStyle; }

public: // Public slots
	void mouseDragged(UMouseEvent * e);

	void closeFrame(UActionEvent * e);
	void maximizeFrame(UActionEvent * e);
	void minimizeFrame(UActionEvent * e);
protected: // Overrides UWidget
	void hierarchyChanged(UWidget * w);

private:
	UDesktopPane * m_desktop;
	/** The added window */
	UWidget * m_window;
	/** The frame decoration */
	UWidget * m_frame;
	uint32_t m_frameStyle;
	std::string m_title;

	UWidget * m_titleBar;
	UMenu * defaultMenu;
	UButton * close;
};

// FIXME: is this the right place?
UFO_IMPLEMENT_ABSTRACT_CLASS(UFrameDecoration, UObject)
UFO_IMPLEMENT_DYNAMIC_CLASS(UDesktopPane, ULayeredPane)

UDesktopPane::UDesktopPane() {}
UDesktopPane::~UDesktopPane() {}

void
UDesktopPane::addFrame(UWidget * w) {
	_UFrameDecoration * fdeco = new _UFrameDecoration(this);
	w->put("frame_decoration", fdeco);
	fdeco->install(w);
	fdeco->getFrame()->setBounds(w->getBounds());
	// FIXME: If we do not always add it as first child in
	// the layer, we do not get any events
	add(fdeco->getFrame(), ULayeredPane::FrameLayer, 0);
	//add(w, ULayeredPane::FrameLayer);
}

UWidget*
UDesktopPane::removeFrame(UWidget * w) {
	_UFrameDecoration * fdeco = dynamic_cast<_UFrameDecoration*>(
		w->get("frame_decoration")
	);
	bool removed = false;
	if (fdeco) {
		removed = remove(fdeco->getFrame());
		fdeco->uninstall(w);
		w->put("frame_decoration", NULL);
		// mem management should be done by UWidget
		//delete (fdeco);
	}

	if (removed) {
		return w;
	}
	return NULL;
}

void
UDesktopPane::maximize(UWidget * w) {
	UWidget * frame = getFrame(w);
	if (!frame) { return; }

	if (!isMinimized(w)) {
		URectangleObject * rect = new URectangleObject(frame->getBounds());
		frame->put("frame_restore_bounds", rect);
	}
	frame->put("frame_maximized", new UInteger(1));
	frame->setBounds(0, 0, getWidth(), getHeight());

	frame->put("frame_minimized", NULL);
}

bool
UDesktopPane::isMaximized(UWidget * w) {
	UWidget * frame = getFrame(w);
	if (!frame) { return false; }

	return (frame->get("frame_maximized") != NULL);
}

void
UDesktopPane::minimize(UWidget * w) {
	UWidget * frame = getFrame(w);
	if (!frame) { return; }

	if (!isMaximized(w)) {
		URectangleObject * rect = new URectangleObject(frame->getBounds());
		frame->put("frame_restore_bounds", rect);
	}
	if (false) {//hasTaskBar()) {
	} else {
		frame->put("frame_minimized", new UInteger(1));
		frame->setBounds(0, getHeight() - 20, 100, 20);
	}
	frame->put("frame_maximized", NULL);
}

bool
UDesktopPane::isMinimized(UWidget * w) {
	UWidget * frame = getFrame(w);
	if (!frame) { return false; }

	return (frame->get("frame_minimized") != NULL);
}

void
UDesktopPane::restore(UWidget * w) {
	UWidget * frame = getFrame(w);
	if (!frame) { return; }
	URectangleObject * rect = dynamic_cast<URectangleObject*>(
		frame->get("frame_restore_bounds")
	);
	if (rect) {
		frame->put("frame_restore_bounds", NULL);
		frame->setBounds(*rect);
	}

	frame->put("frame_maximized", NULL);
	frame->put("frame_minimized", NULL);
}

void
UDesktopPane::close(UWidget * w) {
	removeFrame(w);
}

void
UDesktopPane::setTitle(UWidget * w, const std::string & title) {
	w->put("frame_title", title);
}

std::string
UDesktopPane::getTitle(UWidget * w) {
	return w->getString("frame_title");
}

UWidget*
UDesktopPane::getFrame(UWidget * w) const {
	_UFrameDecoration * fdeco = dynamic_cast<_UFrameDecoration*>(
		w->get("frame_decoration")
	);
	UWidget * ret = fdeco->getFrame();
	// msvc 6
	if (ret && static_cast<const UWidget*>(ret->getParent()) == this) {
		return ret;
	}
	return NULL;
}

void
UDesktopPane::addedToHierarchy() {
	UWidget::addedToHierarchy();
	getContext()->connectListener(slot(
		*this, &UDesktopPane::eventListener)
	);
}

void
UDesktopPane::removedFromHierarchy() {
	UWidget::removedFromHierarchy();
	getContext()->disconnectListener(slot(
		*this, &UDesktopPane::eventListener)
	);
}

void
_UTitleWidget::paintWidget(UGraphics * g) {
	UWidget * frame = m_deco->getFrame();
	URectangle bounds = /*frame->*/getInnerBounds();

	ULayeredPane * layeredPane = dynamic_cast<ULayeredPane*>(frame->getParent());

	UPalette::ColorGroupType type;
	if (layeredPane->getPosition(frame) == 0) {
		type = UPalette::Active;
	} else {
		type = UPalette::Inactive;
	}

	const UColorGroup & colorGroup =
		frame->getWidget(0)->getPalette().getColorGroup(type);

	// draw background
	g->setColor(colorGroup.background());
	g->fillRect(bounds.x, bounds.y, bounds.w, bounds.h);

	// draw a small border line
	g->setColor(colorGroup.background().darker());
	g->drawLine(
		bounds.x, bounds.y + bounds.h - 1,
		bounds.x + bounds.w - 1, bounds.y + bounds.h - 1
	);

	// draw title
	if (m_deco->getFrameStyle() & FrameTitleBar) {
		g->setColor(colorGroup.foreground());
		g->drawString(m_deco->getTitle(), bounds.x + 20, 1);
	}
}

void
_UFrameDecoration::install(UWidget * w) {
	/*if (UInternalFrame * iframe = dynamic_cast<UInternalFrame*>(w)) {
		title = iframe->getTitle();
		frameStyle = iframe->getFrameStyle();
		m_frame = iframe;
	} else {
		m_frame = new UWidget();
		m_frame->setLayout(new UBorderLayout());
		m_frame->add(w);
		m_frameStyle = FrameResizable | FrameTitleBar | FrameCloseBox;
		m_title = "test";
	}*/
	m_window = w;
	if (UInternalFrame * iframe = dynamic_cast<UInternalFrame*>(w)) {
		m_frame = iframe;
		w->put("frame_title", iframe->getTitle());
		m_frameStyle = iframe->getFrameStyle();
		//m_frame->setBorder(iframe->getBorder());
		//iframe->setBorder(NoBorder);
	} else {
		m_frame = new UWidget();
		w->put("frame_title", "internal frame");
		m_frameStyle = FrameResizable | FrameTitleBar | FrameCloseBox;
		m_frame->setBorder(LineBorder);
		m_frame->setLayout(new UBorderLayout());
		m_frame->add(w);
	}

	m_titleBar = new _UTitleWidget(this);//new UWidget();
	m_titleBar->setLayout(new UBorderLayout());
	m_frame->add(m_titleBar, UBorderLayout::North);

	m_frame->sigWidgetAdded().connect(
		slot(*this, &_UFrameDecoration::hierarchyChanged)
	);
	m_frame->sigWidgetRemoved().connect(
		slot(*this, &_UFrameDecoration::hierarchyChanged)
	);
}

void
_UFrameDecoration::hierarchyChanged(UWidget * w) {
	if (w->isInValidHierarchy()) {
		//m_frame->setBorder(LineBorder);
		m_titleBar->sigMouseDragged().connect(slot(
			*this, &_UFrameDecoration::mouseDragged)
		);
		m_titleBar->setPalette(w->getPalette());

		if (m_frameStyle & FrameNoBorder) {
			return;
		}

		UUIManager * manager = w->getUIManager();

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

		if (m_frameStyle & FrameSysMenu) {
			defaultMenu = new UMenu(dynamic_cast<UIcon*>(
				manager->get("UInternalFrame.icon")
			));

			UMenuItem * closeItem = new UMenuItem("Close");
			defaultMenu->add(closeItem);
			closeItem->sigButtonClicked().connect(
				slot(*this, &_UFrameDecoration::closeFrame)
			);

			defaultMenu->setOpaque(false);
			defaultMenu->setBorderPainted(false);
			defaultMenu->setActionCommand("open internal frame menu");
			westPane->add(defaultMenu);
		}

		// set top right button bar

		//UBorder * border = new ULineBorder();
		// minimize button
		if (m_frameStyle & FrameMinimizeBox) {
			UButton * minmizeBox = new UButton(
				dynamic_cast<UIcon*>(manager->get("UInternalFrame.minimizeIcon"))
			);
			minmizeBox->sigButtonClicked().connect(
				slot(*this, &_UFrameDecoration::minimizeFrame)
			);
			minmizeBox->setBorder(LineBorder);//border);
			minmizeBox->setActionCommand("minimize internal frame");
			eastPane->add(minmizeBox);
		}

		// maximize button
		if (m_frameStyle & FrameMaximizeBox) {
			UButton * maxmizeBox = new UButton(
				dynamic_cast<UIcon*>(manager->get("UInternalFrame.maximizeIcon"))
			);
			maxmizeBox->sigButtonClicked().connect(
				slot(*this, &_UFrameDecoration::maximizeFrame)
			);
			maxmizeBox->setBorder(LineBorder);//border);
			maxmizeBox->setActionCommand("maximize internal frame");
			eastPane->add(maxmizeBox);
		}

		// close button
		if (m_frameStyle & FrameCloseBox) {
			close = new UButton(dynamic_cast<UIcon*>(
				manager->get("UInternalFrame.closeIcon")
			));

			close->sigButtonClicked().connect(
				slot(*this, &_UFrameDecoration::closeFrame)
			);
			close->setBorder(LineBorder);//border);
			close->setActionCommand("hide internal frame");
			eastPane->add(close);//, InternalFrameTitleBarLayout::WEST);
		}
		// assure size
		if (m_frameStyle & FrameTitleBar) {
			UDimension size;
			size.w = w->getFont()->getFontMetrics()->getStringWidth(m_title);
			size.h = w->getFont()->getFontMetrics()->getHeight();
			centerPane->setPreferredSize(size);
		}
		centerPane->setEventState(UEvent::MouseMoved, false);
		centerPane->setEventState(UEvent::MousePressed, false);

		m_titleBar->add(westPane, UBorderLayout::West);
		//m_titleBar->add(centerPane, UBorderLayout::Center);
		m_titleBar->add(eastPane, UBorderLayout::East);
	} else {
		m_titleBar->sigMouseDragged().disconnect(slot(
			*this, &_UFrameDecoration::mouseDragged)
		);
		m_titleBar->removeAll();
	}
}

void
_UFrameDecoration::uninstall(UWidget * w) {
	m_window = NULL;
	m_frame->removeAll();
}

void
_UFrameDecoration::closeFrame(UActionEvent * e) {
	// FIXME: Can't immediately close as we are within an action listener
	// need async closing
	//m_desktop->close(m_window);
	m_frame->setVisible(false);
}

void
_UFrameDecoration::maximizeFrame(UActionEvent * e) {
	if (m_desktop->isMaximized(m_window)) {
		m_desktop->restore(m_window);
	} else {
		m_desktop->maximize(m_window);
	}
}
void
_UFrameDecoration::minimizeFrame(UActionEvent * e) {
	if (m_desktop->isMinimized(m_window)) {
		m_desktop->restore(m_window);
	} else {
		m_desktop->minimize(m_window);
	}
}

void
_UFrameDecoration::mouseDragged(UMouseEvent * e) {
	if (!m_desktop->isMaximized(m_window)) {
		UPoint pos = m_frame->getRootLocation();
		pos += e->getRelMovement();
		m_frame->setLocation(pos);
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
			UWidget * frame = NULL;
			int begin = getLayerBegin(ULayeredPane::FrameLayer->toInt());
			int end = getLayerEnd(ULayeredPane::FrameLayer->toInt());
			for (int i = begin; i < end; ++i) {
				if (getWidget(i)->isVisible()) {
					if (getWidget(i)->getRootBounds().contains(me->getRootLocation())) {
						frame = getWidget(i);
						// First widget is top most widget
						break;
					}
				}
			}
			if (frame) {
				moveToFront(frame);
			}
			/*
			URectangle rootBounds = m_frame->getRootBounds();
			if (me && rootBounds.contains(me->getRootLocation())) {
				(dynamic_cast<UDesktopPane*>(m_frame->getParent()))->
					moveToFront(m_frame);
				m_frame->repaint();
			}*/
		}
		break;
	}
}
