/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/uwidgetevent.hpp
    begin             : Wed May 9 2001
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

#ifndef UWIDGETEVENT_HPP
#define UWIDGETEVENT_HPP

#include "uevent.hpp"

namespace ufo {

class UWidget;

/** @short An event for widget state changes.
  * @ingroup events
  *
  * It is fired when a widget was moved, resized, added,
  * is about to be removed or its z order changed.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UWidgetEvent : public UEvent {
	UFO_DECLARE_DYNAMIC_CLASS(UWidgetEvent)
public:
	UWidgetEvent(UWidget * sourceA, Type typeA);

	UWidget * getWidget() const;
};

} // namespace ufo

#endif // UWIDGETEVENT_HPP
