/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uscrollablewidget.hpp
    begin             : Wed Jun 5 2002
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

#ifndef USCROLLABLEWIDGET_HPP
#define USCROLLABLEWIDGET_HPP

#include "uwidget.hpp"

namespace ufo {

/** A scrollable widget.
  * @author Johannes Schmidt
  */

class UFO_EXPORT UScrollableWidget : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UScrollableWidget)
	UFO_UI_CLASS(UWidgetUI)
public: 
	UScrollableWidget();
	
	/** Returns the positive increment for scrolling one "unit"
	  */
	virtual int getUnitIncrement(const URectangle & visibleRectA,
		Orientation orientationA, Direction directionA) const;
	/** A default value for all directions.
	  */
	virtual void setUnitIncrement(int incrementA);

	/** Returns the positive increment for scrolling one block (page up/down)
	  */
	virtual int getBlockIncrement(const URectangle & visibleRectA,
		Orientation orientationA, Direction directionA) const;
	/** A default value for all directions.
	  */
	virtual void setBlockIncrement(int incrementA);


	/** The size of the visible viewport for this scrollable widget.
	  */
	virtual UDimension getPreferredViewportSize() const;
	virtual void setPreferredViewportSize(const UDimension & viewSize);

	/** Paints the given cutting of the scrollable widget.
	  * Should be overriden by subclasses for more optimized clip
	  * paint functions (@see UList).
	  */
	virtual void clipPaint(UGraphics * g, int x, int y, int w, int h);

protected: // Protected attributes
	int m_unitIncrement;
	int m_blockIncrement;
	UDimension m_viewSize;
};

} // namespace ufo

#endif // USCROLLABLEWIDGET_HPP
