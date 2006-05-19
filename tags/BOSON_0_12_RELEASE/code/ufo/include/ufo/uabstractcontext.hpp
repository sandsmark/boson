/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uabstractcontext.hpp
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

#ifndef UABSTRACTCONTEXT_HPP
#define UABSTRACTCONTEXT_HPP

#include "ucontext.hpp"

#define OLD_EVENT_CODE 0

namespace ufo {

class UEvent;
class UMouseEvent;
class UMouseWheelEvent;
class UKeyEvent;
class UWidgetEvent;

class UWidget;
class UAbstractGraphics;


/** @short Implements some platform independent methods of UContext.
  *  Provided for convenience.
  * @ingroup native
  * @ingroup internal
  *
  * This class is not part of the official UFO API and
  * may be changed without warning.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UAbstractContext : public UContext {
	UFO_DECLARE_ABSTRACT_CLASS(UAbstractContext)
public: // c'tors / d'tors
	/** Creates a new abstract context with parent as its logical parent,
	  * i.e. both contexts share their data.
	  */
	UAbstractContext(UAbstractContext * parent);
	virtual ~UAbstractContext();

public: // Implements UContext
	virtual UContextGroup * getContextGroup() const;
	virtual UContext * getParent() const;
	virtual UUIManager * getUIManager() const;
	virtual UGraphics * getGraphics() const;
	virtual URepaintManager * getRepaintManager() const;

	/** Returns false as default. Should be overriden by backends which
	  * support system info.
	  */
	virtual bool getSystemInfo(USystemInfo * info) const;

	virtual URootPane * getRootPane() const;
	virtual void setRootPane(URootPane * paneA);

	virtual UImage * createImage(UImageIO * imageIO);
	virtual UImage * createImage(const std::string & fileName);


public: // Implements UContext
	virtual void dispatchEvent(UEvent * e);

	virtual void setEventGrabber(const USlot1<UEvent*> & slot);//, UEvent::Type type);
	virtual void releaseEventGrabber();
	virtual USlot1<UEvent*> * getEventGrabber() const;

	virtual void connectListener(const USlot1<UEvent*> & slot);
	virtual void disconnectListener(const USlot1<UEvent*> & slot);
	virtual std::list<USlot1<UEvent*> > getListeners() const;

	virtual UInputMap * getInputMap();
	virtual void setInputMap(UInputMap * newMap);

public: // Implements UContext
	virtual void setDeviceBounds(const URectangle & rect);
	virtual URectangle getDeviceBounds() const;

	virtual void setContextBounds(const URectangle & rect);
	virtual URectangle getContextBounds() const;

	virtual void init();
	virtual void refresh();
	virtual void dispose();

protected: // Private methods
	/** Creates the graphics object for this GL context. */
	virtual UGraphics * createGraphics();
	/** Creates the repaint manager object for this GL context. */
	virtual URepaintManager * createRepaintManager();

	/** Constructs a new mouse event from the given event and modifies it
	  * according to the context properties.
	  * Sends it to an appropriate widget using send.
	  */
	virtual void fireMouseEvent(UMouseEvent * e);
	/** Constructs a new mouse event from the given event and modifies it
	  * according to the context properties.
	  * Sends it to an appropriate widget using send.
	  */
	virtual void fireMouseMotionEvent(UMouseEvent * e);
	/** Constructs a new mouse wheel event from the given event and modifies it
	  * according to the context properties.
	  * Sends it to an appropriate widget using send.
	  */
	virtual void fireMouseWheelEvent(UMouseWheelEvent * e);
	/** Constructs a key mouse event from the given event and modifies it
	  * according to the context properties.
	  * Sends it to an appropriate widget using send.
	  */
	virtual void fireKeyEvent(UKeyEvent * e);

private: // Private functions
	/** Sends the given event to the event grabber or the given receiver.
	  * @return True if the event has benn consumed
	  */
	bool send(UWidget * receiver, UEvent * e);
	void sendToGrabber(UEvent * e);

#if !OLD_EVENT_CODE
	bool sendMouseEventToWidgets(std::list<UWidget*> & widgets, UMouseEvent * e);
	bool sendMouseWheelEventToWidgets(std::list<UWidget*> & widgets, UMouseWheelEvent * e);
	bool sendMouseMotionEventToWidgets(std::list<UWidget*> & widgets, UMouseEvent * e);

	void slotDragWidgetRemoved(UWidgetEvent*);
	void addToDragWidgetsStack(const std::list<UWidget*> & addStack);
	void removeFromDragWidgetsStack(UWidget* w);
	void clearDragWidgetsStack();
#endif


protected: // Protected attributes
	/** */
	UContextGroup * m_contextGroup;
	UUIManager * m_uiManager;
	URootPane * m_rootPane;

private: // Private attributes
	UAbstractContext * m_parent;
	UGraphics * m_graphics;
	URepaintManager * m_repaintManager;
	UInputMap * m_inputMap;

	URectangle m_deviceBounds;
	URectangle m_bounds;

#if OLD_EVENT_CODE
	UWidget * m_dragWidget;
#else
	std::list<UWidget*> m_dragWidgetsStack;
#endif


	USlot1<UEvent*> * m_eventGrabber;
	UPoint m_posBeforeGrabbing;
	UPoint m_posAfterGrabbing;

	typedef std::list<USlot1<UEvent*> > ListenerList;
	typedef std::list<USlot1<UEvent*> >::iterator ListenerIterator;

	ListenerList m_listeners;
};

} // namespace ufo

#endif // UABSTRACTCONTEXT_HPP
