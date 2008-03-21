/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ucontext.hpp
    begin             : Mon Oct 29 2001
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

#ifndef UCONTEXT_HPP
#define UCONTEXT_HPP

#include "uobject.hpp"

#include "signals/usignal.hpp"

#include "util/urectangle.hpp"
#include <list>

namespace ufo {

class UWidget;
class URootPane;
class UContextGroup;

class UUIManager;
class UGraphics;
class URepaintManager;
class UImage;
class UImageIO;
class UInputMap;

class UEvent;

struct USystemInfo;

/** @short An UFO context is the abstract container for UFO widgets
  *  (think of it as an OpenGL context).
  * @ingroup native
  *
  * The UFO context provides the top level container for UFO widgets
  * (URootPane) and is responsible for event dispatching.
  * <p>
  * An UFO context does not have to have the size of the OpenGL context.
  *
  * @see UXContext
  * @author Johannes Schmidt
  */

class UFO_EXPORT UContext : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UContext)
public: // Public types
	enum {
		ShareNothing = 0,
		ShareGL = 1,
		ShareLAF = 3,
		ShareAll = ShareLAF
	};
public: // Pure virtual functions
	/** Returns the context group this context belongs to.
	  * @see UContextGroup
	  */
	virtual UContextGroup * getContextGroup() const = 0;

	/** Returns the (logical) parent Context. A context can share
	  * its data with a parent context, i.e. the Look And Feel and
	  * (if the contexts belong to different OpenGL Contexts)
	  * OpenGL data like Texture Objects, Display Lists, ..
	  * May return NULL, if no parent was set.
	  */
	virtual UContext * getParent() const = 0;

	/** Returns the UIManager, which controls the Look And Feel
	  * of this context.
	  * @see UUIManager
	  */
	virtual UUIManager * getUIManager() const = 0;

	/** Returns the graphics object for this context. */
	virtual UGraphics * getGraphics() const = 0;

	/** Returns the repaint manager for this context. */
	virtual URepaintManager * getRepaintManager() const = 0;

	/** Stores system specific information on the underlaying platform
	  * in the given struct.
	  * The system info struct is defined in the ufo/ufo_systeminfo.hpp
	  * header and may be overwritten in a binary compatible way by
	  * backends.
	  * @return false if this method is not supported by the backend
	  */
	virtual bool getSystemInfo(USystemInfo * info) const = 0;


	/** Returns what data is shared with the parent UContext.
	  * This may be 0, when this UContext has no parent, or no
	  * data is shared between them.
	  * Else, it is one of the following values
	  * (respectively an OR´d combination):
	  * <ul>
	  * <li>SHARE_LAF (for sharing the Look And Feel)</li>
	  * <li>SHARE_OGL (for sharing OpenGL data)</li>
	  * <li>SHARE_ALL (for sharing all)</li>
	  * </ul>
	  */
	//virtual uint32_t getSharedBits() const = 0;

	/** every context is the owner of a root pane, the top level container.
	  * @see setRootPane
	  */
	virtual URootPane * getRootPane() const = 0;

	/**@see getRootPane
	  */
	virtual void setRootPane(URootPane * paneA) = 0;

	/** Creates an image for this context group using the given image data.
	  * All context from this context group may access and use that image.
	  */
	virtual UImage * createImage(UImageIO * imageIO) = 0;

	/** Creates an image for this context group using the given file name.
	  * All context from this context group may access and use that image.
	  */
	virtual UImage * createImage(const std::string & fileName) = 0;

	/** locks this context for drawing or Widget operations */
	virtual void lock() = 0;
	/** unlocks this context
	  * @see lock
	  */
	virtual void unlock() = 0;


public: // virtual event functions
	/** This functions dispatches events to child widgets of the root pane.
	  * It should be used by an backend to deliver events to the widgets.
	  * It is virtual so that you can implement your own event dispatcher.
	  */
	virtual void dispatchEvent(UEvent * e) = 0;

	/** Sets an event grabber which receives all events and prevents
	  * that any other widget gets those events.
	  * The source widget is always the root pane of this context.
	  * MouseEntered and MouseExited events are not generated for grabbed events.
	  */
	virtual void setEventGrabber(const USlot1<UEvent*> & slot) = 0;
	virtual void releaseEventGrabber() = 0;
	virtual USlot1<UEvent*> * getEventGrabber() const = 0;

	/** Adds an event listener which receives all events. Events are not
	  * retargeted to this listener widget.
	  */
	virtual void connectListener(const USlot1<UEvent*> & slot) = 0;
	virtual void disconnectListener(const USlot1<UEvent*> & slot) = 0;
	virtual std::list<USlot1<UEvent*> > getListeners() const = 0;

	virtual UInputMap * getInputMap() = 0;
	virtual void setInputMap(UInputMap * newMap) = 0;
public: // virtual geometry methods
	/** */
	virtual void setDeviceBounds(const URectangle & rect) = 0;
	virtual URectangle getDeviceBounds() const = 0;


	/** Sets the context bounds within the device.
	  * The corrdinates are in the UFO coordinate space.
	  */
	virtual void setContextBounds(const URectangle & rect) = 0;
	virtual URectangle getContextBounds() const = 0;

public: // Public
	/** Initialize system dependent resources,
	  * creates the look and feel and the root pane.
	  */
	virtual void init() = 0;
	/** Refreshes system dependent resources and emits a refresh signal. */
	virtual void refresh() = 0;
	/** Disposes all system resources which were created by init()
	  * @see init
	  */
	virtual void dispose() = 0;

public: // Public signals
	/** This signal should be emitted by backend implementation whenever
	  * The OpenGL resources (textures, display lists, ..) should be refreshed
	  */
	USignal0 & sigRefresh();
private:
	USignal0 m_sigRefresh;
};

//
// inline implementation
//

inline USignal0 &
UContext::sigRefresh() {
	return m_sigRefresh;
}

} // namespace ufo

#endif // UCONTEXT_HPP
