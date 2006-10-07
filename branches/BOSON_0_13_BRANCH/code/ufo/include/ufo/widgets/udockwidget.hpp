/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/udockwidget.hpp
    begin             : Tue Apr 5 2005
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

#ifndef UDOCKWIDGET_HPP
#define UDOCKWIDGET_HPP

#include "uwidget.hpp"

namespace ufo {

/** @short Base class for dockable widget
  * @ingroup widgets
  *
  * A dock widget might be added to the desktop pane in one dock area.
  * Not yet implemented is dragging support to drag a dock widget from
  * one area to another area.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UDockWidget : public UWidget {
	UFO_DECLARE_CLASS(UDockWidget)
public:
	UDockWidget();

	void setAllowedAreas(DockWidgetArea areas);
	DockWidgetArea getAllowedAreas() const;

	/** not yet implemented. */
	bool isFloating() const;
	/** not yet implemented. */
	void setFloating(bool b);
private:
	DockWidgetArea m_allowedAreas;
	bool m_isFloating;
};

} // namespace ufo

#endif // UDOCKWIDGET_HPP
