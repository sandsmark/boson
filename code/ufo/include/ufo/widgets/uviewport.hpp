/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uviewport.hpp
    begin             : Sat Oct 12 2002
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

#ifndef UVIEWPORT_HPP
#define UVIEWPORT_HPP

#include "uwidget.hpp"

namespace ufo {

/** @short This class provides an generic viewport for widgets.
  * @ingroup widgets
  *
  * Only the given visible rect is shown by child widgets of viewports.
  * @author Johannes Schmidt
  */
class UFO_EXPORT UViewport : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UViewport)
public:
	UViewport();

public:
	/** Sets the widget this viewport creates an view on. */
	void setView(UWidget * w);
	UWidget * getView() const;

	UPoint getViewLocation() const;
	void setViewLocation(const UPoint & pos);

	void setViewBounds(int x, int y, int w, int h);

	/** Scrolls the viewport so that the given rectangle becomes visible.
	  * The coordinates are in the coordinate space of the view widget.
	  */
	void scrollRectToVisible(const URectangle & rect);

protected: // Protected methods
	virtual ULayoutManager * createLayoutManager();

protected: // Overrides UWidget
	virtual void addImpl(UWidget * w, UObject * constraints, int index);
	virtual void paintChildren(UGraphics * g);

private: // Private attributes
	UWidget * m_view;
	/** Flags whether m_view is of type UScrollableWidget. */
	bool m_scrollableWidget;
};

} // namespace ufo

#endif // UVIEWPORT_HPP
