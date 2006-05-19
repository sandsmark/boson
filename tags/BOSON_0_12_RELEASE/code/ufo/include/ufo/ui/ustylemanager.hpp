/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/ustylemanager.hpp
    begin             : Mon Feb 28 2005
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

#ifndef USTYLEMANAGER_HPP
#define USTYLEMANAGER_HPP

#include "../uobject.hpp"
#include "../font/ufont.hpp"
#include "../util/upalette.hpp"

namespace ufo {

class UStyle;
class UStyleHints;
class UWidget;

/** @short The style manager provides style and style hints for widgets.
  * @ingroup appearance
  *
  * @see UStyle
  * @see UStyleHints
  * @see UWidget
  * @author Johannes Schmidt
  */
class UFO_EXPORT UStyleManager : public UObject {
public:
	UStyleManager();
	UStyleManager(UStyle * style,
		std::map<std::string, UStyleHints*> hints);
	/** Deletes all style hints objects created by this style manager. */
	virtual ~UStyleManager();

	/** @return The style object used for painting. */
	UStyle * getStyle();
	/** Sets the style object used for painting. */
	void setStyle(UStyle * style);

	/** @return The default palette. */
	UPalette getPalette();
	/** Sets the default palette which is used if no other palette is
	  * specified in the style hint object.
	  */
	void setPalette(const UPalette & palette);

	/** @return The default font. */
	UFont getFont();
	/** Sets the default font which is used if no other font is
	  * specified in the style hint object.
	  */
	void setFont(const UFont & font);

	/** Loads a CSS style sheet file. */
	void loadStyleSheet(const std::string & fileName);

	/** Puts a copy of the given style hints in the cache. */
	void putStyleHints(const std::string & classid, UStyleHints * styleHints);
	void putStyleHints(
		const std::string & type,
		const std::string & classId,
		const std::string & name,
		UStyleHints * styleHints
	);
	/** @return The style hints object for the given css type, css class
	  * and name.
	  */
	UStyleHints * getStyleHints(
		const std::string & type,
		const std::string & classId = "",
		const std::string & name = ""
	);
	std::map<std::string, UStyleHints*> &  getStyleHints() { return m_hints; }

private: // Private attributes
	UStyle * m_style;
	std::map<std::string, UStyleHints*> m_hints;
};

} // namespace ufo

#endif // USTYLEMANAGER_HPP
