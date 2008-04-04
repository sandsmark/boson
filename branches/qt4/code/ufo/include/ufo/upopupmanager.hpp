/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/upopupmanager.hpp
    begin             : Mon Jun 9 2003
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

#ifndef UPOPUPMANAGER_HPP
#define UPOPUPMANAGER_HPP

#include "uobject.hpp"

namespace ufo {

class UPopup;
class UWidget;

/** @short This class handles requests for popup widget.
  * @ingroup misc
  *
  * The default implementation uses lightweight popups, i.e.
  * UFO widget placed in the popup layer.
  * //The popup manager is responsible for closing the popup on appropriate
  * //user events.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UPopupManager : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UPopupManager)
public:
	static UPopupManager * getPopupManager();
	static void setPopupManager(UPopupManager * popupManager);

	/** Creates a new popup object.
	  * @param owner The widget which requests to open the popup and which
	  *   context is responsible for closing the popup.
	  * @param content The content widget.
	  * @param x The desired x coordinate in the coord system of the owner
	  * @param y The desired y coordinate in the coord system of the owner
	  * @param w The desired width
	  * @param h The desired height
	  */
	virtual UPopup * createPopup(
		UWidget * owner,
		UWidget * content,
		int x, int y, int w, int h);

private: // Private attributes
	static UPopupManager * sm_popupManager;
};

} // namespace ufo

#endif // UPOPUPMANAGER_HPP
