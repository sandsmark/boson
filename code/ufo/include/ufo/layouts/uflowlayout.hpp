/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/layouts/uflowlayout.hpp
    begin             : Tue May 29 2001
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

#ifndef UFLOWLAYOUT_HPP
#define UFLOWLAYOUT_HPP

#include "ulayoutmanager.hpp"

namespace ufo {

/**a layout which makes a line of all children widgets each with its preferred size.
  *@author Johannes Schmidt
  */

class UFO_EXPORT UFlowLayout : public ULayoutManager {
	UFO_DECLARE_DYNAMIC_CLASS(UFlowLayout)
public:
	UFlowLayout();
	UFlowLayout(int hgap, int vgap);
	UFlowLayout(int hgap, int vgap, Alignment hAlign, Alignment vAlign);
	virtual ~UFlowLayout();

	UDimension getPreferredLayoutSize(const UWidget * parent) const;
	UDimension getMinimumLayoutSize(const UWidget * parent) const;

	void layoutContainer(const UWidget * parent);

	/** Sets the horizontal alignment of widgets within one row
	  */
	virtual void setHorizontalAlignment(Alignment newHAlign);
	/** Returns the horizontal alignment of widgets within one row
	  */
	virtual Alignment getHorizontalAlignment();
	/** Sets the vertical alignment of widgets within one row
	  */
	virtual void setVerticalAlignment(Alignment newVAlign);
	/** Returns the vertical alignment of widgets within one row
	  */
	virtual Alignment getVerticalAlignment();
	virtual int getLayoutHeightForWidth(const UWidget * parent, int w) const
	{
		return 0;
	}

private:  // Private methods
	/**
	  * If there is space at the end of a row or space above or below
	  * a widget, move the widgets from rowStart to rowEnd in the
	  * parent widget.
	  * @param parent
	  *	the container which contains the widget to be moved
	  * @param x
	  *	the horizontal position of the row
	  * @param y
	  *	the vertical position of the row
	  * @param width
	  *	the width of the space at the end of the row
	  * @param height
	  *	the height of the row
	  * @param rowStart
	  *	the starting index of the widgets of the row
	  * @param rowEnd
	  *	the ending index of the widgets of the row
	  */
	void moveWidgets(const UWidget * parent,
		int x, int y, int width, int height, int rowStart, int rowEnd) const;

protected:  // Protected attributes
	/** the horizontal gap between widgets */
	int m_hgap;
	/** the vetical gap  between widgets */
	int m_vgap;

private:  // Private attributes
	/** horizontal alignment of widgets within one row
	  */
	Alignment m_horizontalAlignment;
	/** vertical alignment of widgets within one row
	  */
	Alignment m_verticalAlignment;
};

} // namespace ufo

#endif // UFLOWLAYOUT_HPP
