/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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

/**
  * @short A layout manager with one main widgets and (at most) 4 minor widgets
  *  in north, east, south or west position.
  * @see UWidget::add
  *
  * The constraints object describes at which position the newly added
  * child widget is located. Use @p UBorderLayout::Center (or NULL) for the
  * centered main widget, @p UBorderLayout::North for the northern widget,
  * @p UBorderLayout::South for the southern widget,
  * @p UBorderLayout::East for the eastern widget,
  * @p UBorderLayout::West for the western widget.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBorderLayout : public ULayoutManager {
	UFO_DECLARE_DYNAMIC_CLASS(UBorderLayout)
public:
	/** Creates a new border layout with a default horizontal and vertical gap
	  * between childs of 2 pixels. */
	UBorderLayout(int hgap = 2, int vgap = 2);
	virtual ~UBorderLayout();

public: // Implements ULayoutManager
	virtual void layoutContainer(const UWidget * parent);
	virtual UDimension getPreferredLayoutSize(const UWidget * parent,
		const UDimension & maxSize) const;

protected:
	/** Determines the child widget of the given container, which should
	  * be placed at the given position.
	  */
	UWidget * getChildWidgetAt(const UWidget * container, const UString * position) const;

public:  // Public attributes
	/** Use this as constraint attribute for the centered child widget.
	  * @see UWidget#add
	  */
	static const UString * Center;
	/** Use this as constraint attribute for the northern child widget.
	  * @see UWidget#add
	  */
	static const UString * North;
	/** Use this as constraint attribute for the southern child widget.
	  * @see UWidget#add
	  */
	static const UString * South;
	/** Use this as constraint attribute for the eastern child widget.
	  * @see UWidget#add
	  */
	static const UString * East;
	/** Use this as constraint attribute for the western child widget.
	  * @see UWidget#add
	  */
	static const UString * West;

private:  // Private attributes
	/** horizontal gap between widgets*/
	int m_hgap;
	/** vertical gap between widgets*/
	int m_vgap;
};

} // namespace ufo

#endif // UBORDERLAYOUT_HPP
