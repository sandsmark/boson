/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ux/uxframe.hpp
    begin             : Wed Jul 28 2004
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

#ifndef UXFRAME_HPP
#define UXFRAME_HPP

#include "../uobject.hpp"

#include "../util/urectangle.hpp"
#include "../signals/ufo_signals.hpp"

namespace ufo {

class UXContext;
class UXDisplay;
class URootPane;
class UWidget;
class UVideoDevice;

/** @short A platform-independent representation of system windows.
  * @ingroup native
  *
  * Use a UXDisplay object to create frames.
  * Note: You can use LibUFO without using frames.
  * See test/sdl.cpp and test/glut.cpp
  *
  * @see UXDisplay
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UXFrame : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UXFrame)
	friend class UXDisplay;
protected:
	UXFrame(UVideoDevice * device);
public:
	virtual ~UXFrame();

public: // video device specific
	/** Swaps the GL buffers of this frame. */
	void swapBuffers();
	/** Makes the underlying GL context the current context. */
	void makeContextCurrent();

	UVideoDevice * getVideoDevice() const;

public:
	UXContext * getContext() const;

	URootPane * getRootPane() const;
	UWidget * getContentPane() const;

	/** Repaints the whole frame as long as a repaint was requested by
	  * one of the widgets.
	  *
	  * @param force If true, forces a repaint
	  */
	virtual void repaint(bool force = false);

public:
	void setVisible(bool vis);
	bool isVisible();

	void setDepth(int depth);
	int getDepth() const;

	//
	// various window manager attributes
	//

	/** Sets the frame title. */
	void setTitle(std::string title);
	/** Returns current frame title. */
	std::string getTitle();

	void setFullScreened(bool b);
	bool isFullScreened() const;

	/** Sets frame decoration.
	  * @see FrameStyle
	  */
	void setFrameStyle(uint32_t frameStyle);
	FrameStyle getFrameStyle() const;

	void setResizable(bool b);
	bool isResizable() const;

	/** Sets the initial state of the frame.
	  * This method has no effect on visible frames.
	  * @see FrameState
	  */
	void setInitialFrameState(uint32_t initialState);
	/** Returns the current frame state, or - if not visible - the initial
	  * frame state.
	  */
	FrameState getFrameState() const;

	//
	// geometry
	//

	/** sets the frame size to the preferred size.
	  * computes the preferred size of the root pane and resizes the frame
	  * to the correct size.
	  */
	void pack();

	void setSize(int w, int h);
	void setSize(const UDimension & d);
	/** returns the size of the frame */
	UDimension getSize() const;

	void setLocation(int x, int y);
	void setLocation(const UPoint & d);
	/** Returns the current location of the frame. Currently not supported.
	  */
	UPoint getLocation() const;

	void setBounds(int x, int y, int w, int h);
	void setBounds(const URectangle & rect);
	/** returns the bounds of the frame */
	URectangle getBounds() const;

	/** Dumps the frame content to the given file.
	  * The file must have an extension supported by UImageIO.
	  * @see UImageIO
	  */
	void makeScreenShot(std::string fileNameA);

public: // Public signals
	USignal1<UXFrame*> & sigMoved();
	USignal1<UXFrame*> & sigResized();

protected: //
	void openContext();
	void closeContext();

private: // Private slots
	void deviceMoved(UVideoDevice * videoDevice);
	void deviceResized(UVideoDevice * videoDevice);

private:   // Private attributes
	UVideoDevice * m_videoDevice;
	UXContext * m_context;

	bool m_isVisible;
private: // Private signals
	USignal1<UXFrame*> m_sigMoved;
	USignal1<UXFrame*> m_sigResized;
};

//
// inline implementation
//

inline USignal1<UXFrame*> &
UXFrame::sigMoved() {
	return m_sigMoved;
}

inline USignal1<UXFrame*> &
UXFrame::sigResized() {
	return m_sigResized;
}

} // namespace ufo

#endif // UXFRAME_HPP
