/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/basic/ubasiccomboboxui.hpp
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

#ifndef UBASICCOMBOBOXUI_HPP
#define UBASICCOMBOBOXUI_HPP

#include "../ucomboboxui.hpp"

namespace ufo {

class UComboBox;

/**
  *
  * This class is not part of the official UFO API and
  * may be changed without warning.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBasicComboBoxUI : public UComboBoxUI  {
	UFO_DECLARE_DYNAMIC_CLASS(UBasicComboBoxUI)
public:
	static UBasicComboBoxUI * createUI(UWidget * w);

	void installUI(UWidget * w);
	void uninstallUI(UWidget * w);

	const std::string & getLafId();

private:  // Private attributes
	/** The shared look and feel id */
	static std::string m_lafId;
	/**  */
	static UBasicComboBoxUI * m_defaultComboBoxUI;
};

} // namespace ufo

#endif // UBASICCOMBOBOXUI_HPP
