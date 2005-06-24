/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/basic/ubasicscrollbarui.hpp
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

#ifndef UBASICSCROLLBARUI_HPP
#define UBASICSCROLLBARUI_HPP

#include "../uscrollbarui.hpp"

namespace ufo {

class UScrollBar;
class UEvent;
class UMouseEvent;
class UXBMIcon;

/**
  *
  * This class is not part of the official UFO API and
  * may be changed without warning.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBasicScrollBarUI : public UScrollBarUI  {
	UFO_DECLARE_DYNAMIC_CLASS(UBasicScrollBarUI)
public:
	UBasicScrollBarUI(UScrollBar * scrollBar);

	static UBasicScrollBarUI * createUI(UWidget * w);

	void paint(UGraphics * g, UWidget * w);

	void installUI(UWidget * w);
	void uninstallUI(UWidget * w);

	const std::string & getLafId();

	UDimension getPreferredSize(const UWidget * w);

protected:
	void fireScrollPosUp(UMouseEvent * e);
	void fireScrollPosDown(UMouseEvent * e);

private: // Private attributes
	UScrollBar * m_scrollBar;

	UXBMIcon * m_up;
	UXBMIcon * m_down;
	UXBMIcon * m_left;
	UXBMIcon * m_right;

	Direction m_pushed;

	/** The shared look and feel id */
	static std::string m_lafId;
};

} // namespace ufo

#endif // UBASICSCROLLBARUI_HPP