/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uscrollpane.hpp
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

#ifndef USCROLLPANE_HPP
#define USCROLLPANE_HPP

#include "uwidget.hpp"
#include "uscrollablewidget.hpp"
#include "uscrollbar.hpp"

namespace ufo {

class UViewport;

/**
  *@author Johannes Schmidt
  */

class UFO_EXPORT UScrollPane : public UWidget  {
	UFO_DECLARE_DYNAMIC_CLASS(UScrollPane)
	UFO_UI_CLASS(UWidgetUI)
public:
	UScrollPane(UScrollableWidget * viewA = NULL);

	void setScrollable(UScrollableWidget * viewA);
	UScrollableWidget * getScrollable() const;

	/** If b is true the scroll pane checks whether scroll bars are needed. */
	void setAutoAddingScrollBars(bool b);
	bool isAutoAddingScrollBars() const;

protected: // Overrides UWidget
	virtual void addedToHierarchy();
	virtual void processMouseWheelEvent(UMouseWheelEvent * e);

protected: // Protected methods
	void on_scroll(UScrollBar * scrollBarA, int amountA);

	/** Does all initalizing for the previously set scrollable. */
	void installScrollable();

private: // Private attributes
	UViewport * m_viewport;

	UScrollBar * m_horizontal;
	UScrollBar * m_vertical;

	/** Saves the border type of the scrollable widget. */
	BorderType m_oldBorderType;
	bool m_autoAdd;
protected: // Protected classes
	/** A scroll bar implementation which uses the unit and block increment
	  * methods of UScrollableWidget.
	  * @author Johannes Schmidt
	  */
	class ScrollBar : public UScrollBar {
	public:
		ScrollBar(UScrollPane * pane, Orientation orientation)
			: UScrollBar(orientation), m_scrollPane(pane) {}

		int getUnitIncrement(Direction directionA) const;
		int getBlockIncrement(Direction directionA) const;
	private:
		UScrollPane * m_scrollPane;
	};
};

} // namespace ufo

#endif // USCROLLPANE_HPP
