/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/layouts/uborderlayout.hpp
    begin             : Sat Jul 28 2001
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

#ifndef UBORDERLAYOUT_HPP
#define UBORDERLAYOUT_HPP

#include "ulayoutmanager.hpp"

#include "../util/ustring.hpp"

namespace ufo {

/**similar to java(TM) BorderLayout.
  * It has 5 regions: NORTH, SOUTH, EAST, WEST and CENTER.
  * ...
  * you can add the components to the specified region by using
  *<code> parent.add( component, &UBorderLayout.NORTH ); </code>
  * or <code> parent.add( component, new UString("north") ); </code>
  *@author Johannes Schmidt
  */

class UFO_EXPORT UBorderLayout : public ULayoutManager {
	UFO_DECLARE_DYNAMIC_CLASS(UBorderLayout)
public:
	UBorderLayout(int hgap = 2, int vgap = 2);
	virtual ~UBorderLayout();

	virtual void layoutContainer(const UWidget * parent);

	virtual UDimension getPreferredLayoutSize(const UWidget * parent) const;
	virtual UDimension getMinimumLayoutSize(const UWidget * parent) const;
	virtual int getLayoutHeightForWidth(const UWidget * parent, int w) const
	{
		return 0;
	}

protected:
	UWidget * getChildWidgetAt(const UWidget * parent, const UString * position) const;

public:  // Public attributes
	/** Constraints object
	  * @see UWidget#add
	  */
	static const UString * Center;
	static const UString * North;
	static const UString * South;
	static const UString * East;
	static const UString * West;

private:  // Private attributes
	/**  horizontal gap between widgets*/
	int m_hgap;
	/**  vertical gap between widgets*/
	int m_vgap;
};

} // namespace ufo

#endif // UBORDERLAYOUT_HPP
