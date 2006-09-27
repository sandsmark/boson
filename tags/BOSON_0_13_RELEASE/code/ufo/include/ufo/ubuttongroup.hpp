/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ubuttongroup.hpp
    begin             : Mon Dec 22 2003
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

#ifndef UBUTTONGROUP_HPP
#define UBUTTONGROUP_HPP

#include "uobject.hpp"

#include <vector>

namespace ufo {

class UButton;

/** @short Provides a container for organizing selection states of
  *  toggable button widgets
  * @ingroup widgets
  *
  * A button group is usually used for an exclusive selection of certain
  * buttons, e.g. radio buttons.
  * It is not a visible container for the buttons.
  * <p>
  * If one button within the button group is selected, all other buttons
  * are deselected.
  *
  * @see URadioButton
  * @author Johannes Schmidt
  */
class UFO_EXPORT UButtonGroup : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UButtonGroup)
public:
	UButtonGroup();

	/** Adds a new button to the group. */
	void addButton(UButton * button);
	/** Removes the given button from the group. */
	void removeButton(UButton * button);

	/** Select the given button and deselect all other buttons. */
	void setSelectedButton(UButton * button, bool selected);
	UButton * getSelectedButton() const;

private: // Private attributes
	std::vector<UButton*> m_buttons;
	UButton * m_selectedButton;
};

} // namespace ufo

#endif // UBUTTONGROUP_HPP
