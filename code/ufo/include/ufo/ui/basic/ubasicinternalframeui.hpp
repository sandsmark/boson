/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/basic/ubasicinternalframeui.hpp
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

#ifndef UBASICINTERNALFRAMEUI_HPP
#define UBASICINTERNALFRAMEUI_HPP

#include "../uinternalframeui.hpp"

#include "../../widgets/uwidget.hpp"

namespace ufo {

class UInternalFrame;
class InternalFrameTitleBar;

/**
  *
  * This class is not part of the official UFO API and
  * may be changed without warning.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBasicInternalFrameUI : public UInternalFrameUI {
	UFO_DECLARE_DYNAMIC_CLASS(UBasicInternalFrameUI)
public:
	UBasicInternalFrameUI(UInternalFrame * frame);

	static UBasicInternalFrameUI * createUI(UWidget * w);

	void installUI(UWidget * w);
	void uninstallUI(UWidget * w);

	const std::string & getLafId();

protected:  // protected methods
	/** creates the title bar, a panel with a title label and
	  * close, resize, .. buttons
	  */
	InternalFrameTitleBar * createTitleBar(UInternalFrame * frame);
	/** adds the title bar to the north of the internalframe layout
	  * (a BorderLayout)
	  */
	void setTitleBar(InternalFrameTitleBar * titleBar);

private:  // private Attributes
	/** The shared look and feel id */
	static std::string m_lafId;

	UInternalFrame * m_frame;
	InternalFrameTitleBar * m_titleBar;
};


//
// class title bar
// FIXME move to another source file?

class UButton;
class UMenu;
class ULabel;

class UFO_EXPORT InternalFrameTitleBar : public UWidget {
public:
	InternalFrameTitleBar(UInternalFrame * frame);

public: // Public slots
	void closeFrame(UActionEvent * e);

	void mouseDragged(UMouseEvent * e);
	void mousePressed(UMouseEvent * e);
protected: // Overrides UWidget
	void addedToHierarchy();
	void removedFromHierarchy();
	void paintWidget(UGraphics * g);

private:
	UInternalFrame * m_frame;

	UMenu * defaultMenu;
	UButton * close;
	ULabel * title;
};

} // namespace ufo

#endif // UBASICINTERNALFRAMEUI_HPP
