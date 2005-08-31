/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uinternalframe.hpp
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

#ifndef UINTERNALFRAME_HPP
#define UINTERNALFRAME_HPP

#include "uwidget.hpp"


namespace ufo {

class URootPane;
class ULayeredPane;
class UDesktopPane;

/** @short An internal frame, contains a root pane and optional a menubar
  * @ingroup widgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UInternalFrame : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UInternalFrame)
	UFO_UI_CLASS(UInternalFrameUI)
	UFO_STYLE_TYPE(UStyle::CE_InternalFrame)
public:
	/** Creates a closable internal frame. */
	UInternalFrame();
	/** Creates a closable internal frame with the given title.
	  * @param title The frame caption
	  */
	UInternalFrame(const std::string & title);
	/** Creates an internal frame with the given frame style.
	  * @param frameStyle The frame style (closable, resizable, ...)
	  * @see FrameStyle
	  */
	UInternalFrame(uint32_t frameStyle);
	/** Creates an internal frame with the given title and given frame style.
	  * @param title The frame caption
	  * @see FrameStyle
	  */
	UInternalFrame(const std::string & title, uint32_t frameStyle);

	friend class UDesktopPane;
public: // Public methods
	/** @return The root pane object for this frame
	  */
	virtual URootPane * getRootPane() const;
	/** @return The content pane object for this frame
	  */
	virtual UWidget * getContentPane() const;
	/** @return the layered pane object for this frame
	  */
	virtual ULayeredPane * getLayeredPane() const;

	virtual void setTitle(const std::string & title);
	virtual std::string getTitle();

	/** sets the frame size to the preferred size.
	* computes the preferred size of the root pane and resizes the frame
	* to the correct size.
	*/
	virtual void pack();

	//
	// Attributes
	//

	FrameStyle getFrameStyle() const;
	void setFrameStyle(int frameStyle);

	FrameState getFrameState() const;
	/** Tries to change the frame state. You should check with
	  * getFrameState whether it was succesful.
	  */
	void setFrameState(int frameState);

	/** Maximizes this internal frame to parents root pane size.
	  */
	void maximize();
	bool isMaximized() const;

	void minimize();
	bool isMinimized() const;

	/** Restores this internal frame to its size before
	  * maximizing or minimizing or does nothing.
	  */
	void restore();

	/** Sets whether this frame is resizable.
	  * Default is false. Not yet implemented
	  */
	virtual void setResizable(bool b);
	bool isResizable() const;

public: // overrides UWidget
	/** Returns true if this frame is the top most frame. */
	virtual bool isActive() const;
	virtual UDimension getContentsSize(const UDimension & maxSize) const;
	virtual void processMouseEvent(UMouseEvent * e);
	virtual void processWidgetEvent(UWidgetEvent * e);
	virtual void processStateChangeEvent(uint32_t state);

protected: // Protected methods
	UDesktopPane * getDesktopPane();

protected:  // Protected attributes
	UDesktopPane * m_desktop;
	/** the root pane object */
	URootPane * m_rootPane;
	std::string m_title;

	uint32_t m_frameStyle;
	uint32_t m_frameState;
	URectangle m_restoreBounds;
};

} // namespace ufo

#endif // UINTERNALFRAME_HPP
