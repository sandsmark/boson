/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/upopup.hpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UPOPUP_HPP
#define UPOPUP_HPP

#include "uobject.hpp"
#include "signals/usignal.hpp"

namespace ufo {

/** @short A general abstraction for a widget which pops up.
  * @ingroup misc
  *
  * A popup is created via the popup manager and is initially
  * visible.
  * The life cycle for popups are generally short.
  * After hiding it, you shouldn't access it anymore.
  * UPopup implementations should emit a popup about to close signal.
  *
  * @see UPopupManager
  * @author Johannes Schmidt
  */
class UFO_EXPORT UPopup : public virtual UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UPopup)
public:
	/** Hides the popup and disposes its internal data.
	  * You shouldn't access a popup after hiding it.
	  */
	virtual void hide() = 0;

	/** Sets the size of the popup to the preferred size. */
	virtual void pack() = 0;

	/** Returns the content pane for the user widgets. */
	virtual UWidget * getContentPane() const = 0;

public: // Public signal accessors
	USignal1<UPopup*> & sigPopupAboutToClose();
private: // Private signals
	USignal1<UPopup*> m_sigPopupAboutToClose;
};

//
// inline implementation
//

inline USignal1<UPopup*> &
UPopup::sigPopupAboutToClose() {
	return m_sigPopupAboutToClose;
}

} // namespace ufo

#endif // UPOPUP_HPP
