/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/basic/ubasicseparatorui.hpp
    begin             : Mon Jul 22 2002
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

#ifndef UBASICSEPARATORUI_HPP
#define UBASICSEPARATORUI_HPP

#include "../useparatorui.hpp"


namespace ufo {

/**
  *
  * This class is not part of the official UFO API and
  * may be changed without warning.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBasicSeparatorUI : public USeparatorUI {
	UFO_DECLARE_DYNAMIC_CLASS(UBasicSeparatorUI)
public:
	static UBasicSeparatorUI * createUI(UWidget * w);

	void paint(UGraphics * g, UWidget * w);

	const std::string & getLafId();

	UDimension getPreferredSize(const UWidget * w);

private:  // Private attributes
	/** the shared widget ui object */
	static UBasicSeparatorUI * m_separatorUI;
	/** The shared look and feel id */
	static std::string m_lafId;
};

} // namespace ufo

#endif // UBASICSEPARATORUI_HPP
