/***************************************************************************
                          ubasicradiobuttonui.hpp  -  description
                             -------------------
    begin                : Sat Jan 10 2004
    copyright            : (C) 2004 by Johannes Schmidt
    email                : schmidtjf at users.sourceforge.net
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

#ifndef UBASICRADIOBUTTONUI_HPP
#define UBASICRADIOBUTTONUI_HPP

#include "ubasicbuttonui.hpp"

namespace ufo {

/**
  *
  * This class is not part of the official UFO API and
  * may be changed without warning.
  *
  *@author Johannes Schmidt
  */

class UFO_EXPORT UBasicRadioButtonUI : public UBasicButtonUI  {
	UFO_DECLARE_DYNAMIC_CLASS(UBasicRadioButtonUI)
public:
	static UBasicRadioButtonUI * createUI(UWidget * w);

	void installUI(UWidget * w);
	void uninstallUI(UWidget * w);

	void paint(UGraphics * g, UWidget * w);

	const std::string & getLafId();

private:  // Private attributes
	/** the shared button ui object */
	static UBasicRadioButtonUI * m_radioButtonUI;

	/** The shared look and feel id */
	static std::string m_lafId;
};

} // namespace ufo

#endif // UBASICRADIOBUTTONUI_HPP
