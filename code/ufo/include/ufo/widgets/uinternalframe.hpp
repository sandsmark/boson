/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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

// we need this for proper getUI() overriding
//#include "../ui/uinternalframeui.hpp"

namespace ufo {

class URootPane;
class ULayeredPane;

/** an internal frame, contains a root pane and optional a menubar
  * @author Johannes Schmidt
  */

class UFO_EXPORT UInternalFrame : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UInternalFrame)
	UFO_UI_CLASS(UInternalFrameUI)
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

public: // overrides UWidget
	/** Brings internal frame to front */
	virtual void setVisible(bool b);
	/** Returns true if this frame it the top most frame. */
	virtual bool isActive() const;

public: // Public methods

	/** returns the root pane object for this frame
	  */
	virtual URootPane * getRootPane() const;
	/** returns the root pane object for this frame
	  */
	virtual UWidget * getContentPane() const;
	/** returns the root pane object for this frame
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

	/** Maximizes this internal frame to parents root pane size.
	  * Not yet implemented.
	  */
	void maximize();

	/** Not yet implemented. */
	void restore();

	/** Sets whether this frame is resizable.
	  * Default is false.
	  * Not yet implemented.
	  */
	virtual void setResizable(bool b);
	bool isResizable() const;

	/** Sets whether this frame is maximizable.
	  * Default is false.
	  * Not yet implemented.
	  */
	virtual void setMaximizable(bool b);
	bool isMaximizable() const;

	/** Sets whether this frame is maximizable.
	  * Default is false.
	  * Not yet implemented.
	  */
	virtual void setMinimizable(bool b);
	bool isMinimizable() const;

	/** Sets whether this frame is closable.
	  * Default is false.
	  * Not yet implemented.
	  */
	virtual void setClosable(bool b);
	bool isClosable() const;

protected:  // Protected attributes
	/** the root pane object
	  * @see URootPane
	  */
	URootPane * m_rootPane;
	std::string m_title;

	int m_frameStyle;
};

} // namespace ufo

#endif // UINTERNALFRAME_HPP
