/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/ulistboxui.hpp
    begin             : Wed Jun 19 2002
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

#ifndef ULISTBOXUI_HPP
#define ULISTBOXUI_HPP

#include "uwidgetui.hpp"

#include "../util/upoint.hpp"

namespace ufo {

class UListBox;

/**
  *@author Johannes Schmidt
  */

class UFO_EXPORT UListBoxUI : public UWidgetUI  {
	UFO_DECLARE_ABSTRACT_CLASS(UListBoxUI)

public:/*
	void installUI(UWidget * w);
	void uninstallUI(UWidget * w);
*/
	virtual UPoint indexToLocation(const UListBox * listA, unsigned int indexA) = 0;
	virtual int locationToIndex(const UListBox * listA, const UPoint & locationA) = 0;

	//virtual void paint(UList * list, int x, int y, int w, int h) = 0;
/*
private: // Private classes
	class MouseSelectionListener : public UMouseListener {
	public:
		void mousePressed(UMouseEvent * e);
		void mouseReleased(UMouseEvent * e);
		void mouseClicked(UMouseEvent * e);
	};
*/
private: // Private attributes
	//static MouseSelectionListener * m_mouseSelectionListener;
};

} // namespace ufo

#endif // ULISTBOXUI_HPP
