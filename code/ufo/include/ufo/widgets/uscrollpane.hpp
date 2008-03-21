/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#ifndef USCROLLPANE_HPP
#define USCROLLPANE_HPP

#include "uwidget.hpp"
#include "uscrollablewidget.hpp"
#include "uscrollbar.hpp"

namespace ufo {

class UViewport;

/** @short A scroll pane may be used to display a smaller detail of
  *  a large widget on a relative
  * @ingroup widgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UScrollPane : public UWidget  {
	UFO_DECLARE_DYNAMIC_CLASS(UScrollPane)
public:
	UScrollPane(UScrollableWidget * viewA = NULL);

	void setScrollable(UScrollableWidget * viewA);
	UScrollableWidget * getScrollable() const;

	/** If b is true the scroll pane checks whether scroll bars are needed. */
	void setAutoAddingScrollBars(bool b);
	bool isAutoAddingScrollBars() const;

protected: // Overrides UWidget
	virtual void processMouseWheelEvent(UMouseWheelEvent * e);
	virtual void processWidgetEvent(UWidgetEvent * e);

protected: // Protected methods
	void on_scroll(UAbstractSlider * slider);

	/** Does all initalizing for the previously set scrollable. */
	void installScrollable();

private: // Private attributes
	UViewport * m_viewport;

	UScrollBar * m_horizontal;
	UScrollBar * m_vertical;

	/** Saves the border type of the scrollable widget. */
	BorderType m_oldBorderType;
	bool m_autoAdd;
};

} // namespace ufo

#endif // USCROLLPANE_HPP
