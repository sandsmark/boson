/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/uabstractcontext.cpp
    begin             : Fri Aug 16 2002
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

#include "ufo/uabstractcontext.hpp"

//#include "ufo/uabstractgraphics.hpp"
#include "ufo/gl/ugl_graphics.hpp"
#include "ufo/gl/ugl_image.hpp"
#include "ufo/urepaintmanager.hpp"

#include "ufo/events/uevent.hpp"
#include "ufo/events/umouseevent.hpp"
#include "ufo/events/umousewheelevent.hpp"
#include "ufo/events/ukeyevent.hpp"

#include "ufo/widgets/uwidget.hpp"
#include "ufo/widgets/urootpane.hpp"

#include "ufo/ui/uuimanager.hpp"

#include "ufo/utoolkit.hpp"
#include "ufo/ucontextgroup.hpp"

// for key bindings
#include "ufo/uinputmap.hpp"
#include "ufo/ukeystroke.hpp"
#include "ufo/events/uactionevent.hpp"


using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UAbstractContext, UObject)

UAbstractContext::UAbstractContext(UAbstractContext * parent)
	: m_contextGroup(NULL)
	, m_uiManager(NULL)
	, m_rootPane(NULL)
	, m_parent(parent)
	, m_graphics(NULL)
	, m_repaintManager(NULL)
	, m_inputMap(new UInputMap())
	, m_deviceBounds()
	, m_bounds()
	, m_dragWidget(NULL)
	, m_dragWidgets()
	, m_eventGrabber(NULL)
	, m_listeners()
{
	if (m_parent) {
		m_contextGroup = m_parent->m_contextGroup;
	} else {
		m_contextGroup = new UContextGroup();
	}
}

UAbstractContext::~UAbstractContext() {
	releaseEventGrabber();

	std::vector<UContext*> m_contexts = m_contextGroup->getContexts();
	if (m_contexts.size() == 0) {
		delete (m_contextGroup);
	}
}

UContextGroup *
UAbstractContext::getContextGroup() const {
	return m_contextGroup;
}

UContext *
UAbstractContext::getParent() const {
	return m_parent;
}

UUIManager *
UAbstractContext::getUIManager() const {
	return m_uiManager;
}


UGraphics *
UAbstractContext::getGraphics() const {
	return m_graphics;
}

URepaintManager *
UAbstractContext::getRepaintManager() const {
	return m_repaintManager;
}

bool
UAbstractContext::getSystemInfo(USystemInfo * info) const {
	return false;
}


URootPane *
UAbstractContext::getRootPane() const {
	return m_rootPane;
}

void
UAbstractContext::setRootPane(URootPane * paneA) {
	if (paneA && paneA != m_rootPane) {
		swapPointers(m_rootPane, paneA);
		m_rootPane = paneA;
		m_rootPane->invalidateTree();
		m_rootPane->setContext(this);
		m_rootPane->addedToHierarchy();
		m_rootPane->setBounds(0, 0, m_bounds.w, m_bounds.h);
		m_rootPane->setVisible(true);
		m_rootPane->requestFocus();
	}
}

UImage *
UAbstractContext::createImage(UImageIO * imageIO) {
	return new UGL_Image(imageIO);
}

UImage *
UAbstractContext::createImage(const std::string & fileName) {
	return new UGL_Image(fileName);
}

//
// event methods
//

void
UAbstractContext::dispatchEvent(UEvent * e) {
	if (e) {
		e->reference();
		//if (m_eventGrabber) {
		//	sendToGrabber(e);
		//} else {
		switch (e->getType()) {
			case UEvent::MousePressed:
			case UEvent::MouseReleased:
			case UEvent::MouseClicked:
				fireMouseEvent(dynamic_cast<UMouseEvent *>(e));
			break;
			case UEvent::MouseMoved:
			case UEvent::MouseDragged:
			case UEvent::MouseEntered:
			case UEvent::MouseExited:
				fireMouseMotionEvent(dynamic_cast<UMouseEvent *>(e));
			break;
			case UEvent::MouseWheel:
				fireMouseWheelEvent(dynamic_cast<UMouseWheelEvent *>(e));
			break;
			case UEvent::KeyPressed:
			case UEvent::KeyReleased:
			case UEvent::KeyTyped:
				fireKeyEvent(dynamic_cast<UKeyEvent *>(e));
			break;
			default:
			break;
		//}
		}
		e->unreference();
	}
}

void
UAbstractContext::setEventGrabber(const USlot1<UEvent*> & slot) {
	m_eventGrabber = new USlot1<UEvent*>(slot);
}

void
UAbstractContext::releaseEventGrabber() {
	if (m_eventGrabber) {
		delete (m_eventGrabber);
		m_eventGrabber = NULL;
		fireMouseMotionEvent(
			new UMouseEvent(getRootPane(), UEvent::MouseMoved,
				UMod::NoModifier,
				m_posAfterGrabbing,
				m_posAfterGrabbing - m_posBeforeGrabbing,
				UMod::NoButton, 0
			)
		);
	}
}

USlot1<UEvent*> *
UAbstractContext::getEventGrabber() const {
	return m_eventGrabber;
}

void
UAbstractContext::connectListener(const USlot1<UEvent*> & slot) {
	m_listeners.push_back(slot);
}

void
UAbstractContext::disconnectListener(const USlot1<UEvent*> & slot) {
	m_listeners.remove(slot);
}

std::list<USlot1<UEvent*> >
UAbstractContext::getListeners() const {
	return m_listeners;
}

UInputMap *
UAbstractContext::getInputMap() {
	return m_inputMap;
}

void
UAbstractContext::setInputMap(UInputMap * newMap) {
	m_inputMap = newMap;
}

// geometry methods


void
UAbstractContext::setDeviceBounds(const URectangle & rect) {
	m_deviceBounds = rect;
}

URectangle
UAbstractContext::getDeviceBounds() const {
	return m_deviceBounds;
}


#include "ufo/ufo_debug.hpp"

void
UAbstractContext::setContextBounds(const URectangle & rect) {
	m_bounds = rect;
	if (m_rootPane) {
		m_rootPane->setBounds(0, 0, m_bounds.w, m_bounds.h);
	}
}

URectangle
UAbstractContext::getContextBounds() const {
	return m_bounds;
}


void
UAbstractContext::init() {
	m_graphics = createGraphics();
	trackPointer(m_graphics);

	m_repaintManager = createRepaintManager();
	trackPointer(m_repaintManager);

	m_graphics->resetDeviceAttributes();
	m_graphics->resetDeviceViewMatrix();
	/*
	if (getSharedBits() & ShareLAF) {
		if (UContext * context = getParent()) {
			// re-use the already existant ui manager object of the parent
			m_uiManager = context->getUIManager();
		}
	} else {
		// create a new ui manager and create an appropriate look and feel
		m_uiManager = new UUIManager();
		m_uiManager->setLookAndFeel(UToolkit::getToolkit()->createLookAndFeel());
	}*/
	// the new default is to always share th ui manager
	if (UContext * context = getParent()) {
		// re-use the already existant ui manager object of the parent
		m_uiManager = context->getUIManager();
	} else {
		m_uiManager = new UUIManager();
		m_uiManager->setLookAndFeel(UToolkit::getToolkit()->createLookAndFeel());
	}

	trackPointer(m_uiManager);

	setRootPane(new URootPane());
}

void
UAbstractContext::refresh() {
	// refresh ui manager (only when created self)
	UContext * context = getParent();
	if (!context) {// || !(getSharedBits() & ShareLAF)) {
		getUIManager()->refresh();
	}
	// FIXME!: this should be done automatically by refresh signals
	// refresh ui attributes (like icons which have to be recreated)
	//if (m_rootPane) {
	//	m_rootPane->invalidateTree(UWidget::ValidationUIAttributes);
	//}
	// emit a refresh signal
	sigRefresh()();
}

void
UAbstractContext::dispose() {
	// recursiv destruction of all created memebers
	releasePointer(m_rootPane);
	m_rootPane = NULL;

	releasePointer(m_uiManager);
	m_uiManager = NULL;

	releasePointer(m_repaintManager);
	m_repaintManager = NULL;

	releasePointer(m_graphics);
	m_graphics = NULL;
}


//
// protected methods
//

UGraphics *
UAbstractContext::createGraphics() {
	//return new UAbstractGraphics(this);
	return new UGL_Graphics(this);
}

URepaintManager *
UAbstractContext::createRepaintManager() {
	return new URepaintManager();
}

// AB: this code exists only for reference. you can see what i changed with it,
// but it should not be used anymore.
#define OLD_DRAG_WIDGET 0

void
UAbstractContext::fireMouseEvent(UMouseEvent * e) {
	URootPane * root = getRootPane();

	UFO_ASSERT(root)
	if (!root) {
		uError() << "UAbstractContext: can´t process " << e << "\n"
		<< "REASON: this context has no root pane" << "\n";
		return;
	}

	UPoint pos = e->getLocation();
	// translate with context location in device
	pos.x -= m_bounds.x;
	pos.y -= m_bounds.y;

#if OLD_DRAG_WIDGET
	UWidget * w = NULL;
#endif
	std::vector<UWidget*> visibleWidgets;

	// if we have been dragging, send the release event to the drag widget.
#if OLD_DRAG_WIDGET
	if (m_dragWidget) {
		w = m_dragWidget;
	} else {
		w = root->getVisibleWidgetAt(pos);
	}
#endif
	if (m_dragWidgets.size() > 0) {
		visibleWidgets = m_dragWidgets;
	} else {
		visibleWidgets = root->getVisibleWidgetsAt(pos);
	}

	if (e->getType() == UEvent::MousePressed) {
#if OLD_DRAG_WIDGET
		m_dragWidget = w;
#endif
		m_dragWidgets = visibleWidgets;
	} else if (e->getType() == UEvent::MouseReleased && !e->hasMouseModifiers()) {
		// all mouse buttons have been released, dragging is over
#if OLD_DRAG_WIDGET
		m_dragWidget = NULL;
#endif
		m_dragWidgets.clear();
	}

#if OLD_DRAG_WIDGET
	UPoint wRoot = (w) ? w->getRootLocation() : UPoint();

	// lazily create a new mouse event
	UMouseEvent * newE = new UMouseEvent(
		w,
		e->getType(),
		e->getModifiers(),
		pos - wRoot,
		e->getRelMovement(),
		pos,
		e->getButton(),
		e->getClickCount()
	);

	if (send(w, newE)) {
		e->consume();
	}
#endif
	for (std::vector<UWidget*>::iterator it = visibleWidgets.begin();
			it != visibleWidgets.end();
			++it) {

		UWidget * w = *it;
		UPoint wRoot = w->getRootLocation();

		// lazily create a new mouse event
		UMouseEvent * newE = new UMouseEvent(
			w,
			e->getType(),
			e->getModifiers(),
			pos - wRoot,
			e->getRelMovement(),
			pos,
			e->getButton(),
			e->getClickCount()
		);

		if (send(w, newE)) {
			e->consume();
		}
	}
	for (std::vector<UWidget*>::iterator it = visibleWidgets.begin();
			it != visibleWidgets.end();
			++it) {
		// flush the saved event
		(*it)->dispatchEvent(NULL);
	}
}

void
UAbstractContext::fireMouseWheelEvent(UMouseWheelEvent * e) {
	URootPane * root = getRootPane();

	if (!root) {
		uError()
		<< this << "  can´t process " << e << "\n"
		<< "REASON: this context has no root pane" << "\n";
		return;
	}

	UPoint pos = e->getLocation();
	// translate with context location in device
	pos.x -= m_bounds.x;
	pos.y -= m_bounds.y;

	UWidget * w = root->getVisibleWidgetAt(pos);
	UPoint wRoot = (w) ? w->getRootLocation() : UPoint();

	// lazily create a new mouse event
	UMouseWheelEvent * newE = new UMouseWheelEvent(
		w,
		e->getType(),
		e->getModifiers(),
		pos - wRoot,
		pos,
		e->getDelta(),
		e->getWheel()
	);

	if (send(w, newE)) {
		e->consume();
	}
	// flush the event
	if (w) {
		w->dispatchEvent(NULL);
	}
}

void
UAbstractContext::fireMouseMotionEvent(UMouseEvent * e) {
	URootPane * root = getRootPane();

	if (!root) {
		uError()
		<< this << "  can´t process " << e << "\n"
		<< "REASON: this context has no root pane" << "\n";
		return;
	}
	bool consumed = false;

	UPoint pos = e->getLocation();
	// translate with context location in device
	pos.x -= m_bounds.x;
	pos.y -= m_bounds.y;

	if (!m_eventGrabber) {
		// for event grabbing
		m_posBeforeGrabbing = pos;
	} else {
		// for event grabbing releases
		m_posAfterGrabbing = pos;
	}

	// FIXME: should mouse drag events override all?
	// first send the drag event, then look for widgets under mouse
	// the drag event might move some widgets (e.g. internal frames).
#if OLD_DRAG_WIDGET
	if (m_dragWidget) {
		// lazily create a new mouse event
		UMouseEvent * newE = new UMouseEvent(
			m_dragWidget,
			UEvent::MouseDragged,
			e->getModifiers(),
			pos - m_dragWidget->getRootLocation(),
			e->getRelMovement(),
			pos,
			e->getButton(),
			e->getClickCount()
		);

		if (send(m_dragWidget, newE)) {
			consumed = true;
		}

		// flush the event
		m_dragWidget->dispatchEvent(NULL);
	}
#endif
	if (m_dragWidgets.size() > 0) {
		for (std::vector<UWidget*>::iterator it = m_dragWidgets.begin();
				it != m_dragWidgets.end();
				++it) {
			// lazily create a new mouse event
			UMouseEvent * newE = new UMouseEvent(
				(*it),
				UEvent::MouseDragged,
				e->getModifiers(),
				pos - (*it)->getRootLocation(),
				e->getRelMovement(),
				pos,
				e->getButton(),
				e->getClickCount()
			);

			if (send(*it, newE)) {
				consumed = true;
			}
		}
		// flush the event
		for (std::vector<UWidget*>::iterator it = m_dragWidgets.begin();
				it != m_dragWidgets.end();
				++it) {
			(*it)->dispatchEvent(NULL);
		}
	}

	UWidget * w;
	UWidget * wbef;

	w = root->getVisibleWidgetAt(pos);
	wbef = root->getVisibleWidgetAt(pos.x - e->getXRel(), pos.y - e->getYRel());

	UPoint wRoot = (w) ? w->getRootLocation() : UPoint();
	UPoint wbefRoot = (wbef) ? wbef->getRootLocation() : UPoint();

	// mouse motion event
#if OLD_DRAG_WIDGET
	if (!m_dragWidget && w == wbef) {
#else
	if (m_dragWidgets.size() == 0 && w == wbef) {
#endif
		// lazily create a new mouse event
		UMouseEvent * newE = new UMouseEvent(
			w,
			e->getType(),
			e->getModifiers(),
			pos - wRoot,
			e->getRelMovement(),
			pos,
			e->getButton(),
			e->getClickCount()
		);

		if (send(w, newE)) {
			consumed = true;
		}

		// flush the event
		if (w) {
			w->dispatchEvent(NULL);
		}
	}
	// always send mouse enter and exit events
	if (w != wbef) {
		UMouseEvent * newE;

		// mouse exit event
		newE = new UMouseEvent(
			wbef,
			UEvent::MouseExited,
			e->getModifiers(),
			pos - wbefRoot,
			e->getRelMovement(),
			pos,
			e->getButton(),
			e->getClickCount()
		);

		if (send(wbef, newE)) {
			consumed = true;
		}

		// flush the event
		if (wbef) {
			wbef->dispatchEvent(NULL);
		}

		// mouse enter event
		newE = new UMouseEvent(
			w,
			UEvent::MouseEntered,
			e->getModifiers(),
			pos - wRoot,
			e->getRelMovement(),
			pos,
			e->getButton(),
			e->getClickCount()
		);

		if (send(w, newE)) {
			consumed = true;
		}
		// flush the event
		if (w) {
			w->dispatchEvent(NULL);
		}
	}
	if (consumed) {
		e->consume();
	}
}

void
UAbstractContext::fireKeyEvent(UKeyEvent * e) {
	URootPane * root = getRootPane();

	if (!root) {
		uError()
		<< this << "  can´t process " << e << "\n"
		<< "REASON: this context has no root pane" << "\n";
		return;
	}
	// check key bindings
	UKeyStroke stroke(e);
	USlot1<UActionEvent*> * slot = m_inputMap->get(stroke);
	if (slot) {
		UActionEvent * ae = new UActionEvent(this, UEvent::Action,
			e->getModifiers(), stroke.toString());
		ae->reference();
		(*slot)(ae);
		// FIXME: should context bindings be consumable?
		// if (e->isConsumend()) {
		//	e->unreference();
		//	return;
		//}
		ae->unreference();
	}

	UWidget * w = root->getFocusedWidget();

	// lazily create a new key event
	UKeyEvent * newE = new UKeyEvent(
		w,
		e->getType(),
		e->getModifiers(),
		e->getKeyCode(),
		e->getKeyChar()
	);

	if (send(w, newE)) {
		e->consume();
	}

	// flush the event
	if (w) {
		w->dispatchEvent(NULL);
	}
}

//
// private methods
//

bool
UAbstractContext::send(UWidget * receiver, UEvent * e) {
	e->reference();

	if (m_eventGrabber) {
		(*m_eventGrabber)(e);
	} else {

	for (ListenerIterator next_iter, iter = m_listeners.begin();
			iter != m_listeners.end();
			iter = next_iter) {
		next_iter = iter;
		++next_iter;
		// FIXME !
		// now what? what shall be the source?
		//e->setSource(*iter);
		//(*iter)->dispatchEvent(e);
		(*iter)(e);
	}


	e->setSource(receiver);
	if (receiver) {
		receiver->dispatchEvent(e);
	}

	}
	if (e->isConsumed()) {
		e->unreference();
		return true;
	} else {
		e->unreference();
		return false;
	}
}
