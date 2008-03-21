/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#ifndef UFLOWLAYOUT_HPP
#define UFLOWLAYOUT_HPP

#include "ulayoutmanager.hpp"

namespace ufo {

/** @short Layouts all child widgets in one continous flow from left to right,
  *  if necessary in several rows.
  * @author Johannes Schmidt
  */

class UFO_EXPORT UFlowLayout : public ULayoutManager {
	UFO_DECLARE_DYNAMIC_CLASS(UFlowLayout)
public:
	/** Creates a new flow layout with horizontal and vertical gap of 4 and
	  * using the alignment of the container. */
	UFlowLayout();
	/** Creates a new flow layout with the given horizontal and vertical gap
	  * and using the alignment of the container. */
	UFlowLayout(int hgap, int vgap);
	UFlowLayout(int hgap, int vgap, Alignment hAlign, Alignment vAlign);
	virtual ~UFlowLayout();

public: // Implements ULayoutManager
	UDimension getPreferredLayoutSize(const UWidget * parent,
		const UDimension & maxSize) const;

	void layoutContainer(const UWidget * parent);

public: // Public methods
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

private:  // Private methods
	/** Realigns the widgets from rowStart to rowEnd within the container
	  * if the container alignment does not match top left.
	  * Is called whenever a "row" has been filled.
	  * @param container The container which contains the widget to be moved
	  * @param x The horizontal position of the row
	  * @param y The vertical position of the row
	  * @param width The width of the row of child widgets
	  * @param height The height of the row
	  * @param rowStart The starting index of the widgets of the row
	  * @param rowEnd The ending index of the widgets of the row
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
