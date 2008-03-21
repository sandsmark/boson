/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uframedecoration.hpp
    begin             : Thu Jan 6 2005
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

#ifndef UFRAMEDECORATION_HPP
#define UFRAMEDECORATION_HPP

#include "../uobject.hpp"

namespace ufo {

/** @short Installs a surrounding frame and a title bar with event listeners
  *  if wanted
  *
  * This class is used by UDesktopPane
  * @see UDesktopPane
  * @author Johannes Schmidt
  */
class UFO_EXPORT UFrameDecoration : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UFrameDecoration)
public:
	/** Installs the frame decoration to a window widget, usually a
	  * UInternalFrame. May differ from the widget returned by @p getFrame()
	  * @see UInternalFrame
	  * @see
	  */
	virtual void install(UWidget * window) = 0;
	/** Installs the frame decoration to a window widget, usually an
	  * internal frame.
	  */
	virtual void uninstall(UWidget * window) = 0;
	/** @return The frame widget (usually UInternalFrame).
	  */
	virtual UWidget * getFrame() const = 0;
};

} // namespace ufo

#endif // UFRAMEDECORATION_HPP
