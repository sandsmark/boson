/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/ucolorgroup.hpp
    begin             : Fri Jun 11 2004
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

#ifndef UCOLORGROUP_HPP
#define UCOLORGROUP_HPP

#include "../uobject.hpp"
#include "ucolor.hpp"

namespace ufo {

/** A color group represents a color set which is used to paint a certain
  * state of a widget, e.g. an active widget compared to a disabled one. 
  * Modelled after the color scheme spec of freedesktop.org
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UColorGroup : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UColorGroup)
public: // Public types
	enum ColorRole {
		Base = 0, // Desktop background color
		BaseFore, // suitable foreground color for drawing over the Desktop background.
		Background, // background color
		Foreground, // suitable foreground color for drawing over the widget
		Text, // used for text on the widget
		Light, // a light background color 
		Dark, // a dark background color
		//
		MidLight, // a decent active widget background
		Highlight, // highlighted background
		HighlightedText, // highlighted text
		MaxColorRoles
	};
public: // constructors
	UColorGroup();
	UColorGroup(const UColor & prim1, const UColor & prim2, const UColor & prim3,
	const UColor & sec1, const UColor & sec2, const UColor & sec3);
	UColorGroup(
		const UColor & base, const UColor & baseFore,
		const UColor & background, const UColor & foreground,
		const UColor & text,
		const UColor & light, const UColor & dark,
		const UColor & midLight, const UColor & highlight,
		const UColor & highlightedText
	);
	virtual ~UColorGroup();

public:
	void setColor(ColorRole role, const UColor & color);
	const UColor & getColor(ColorRole role) const;

	//
	const UColor & base()	const	{ return getColor(Base); }
	const UColor & baseFore()	const	{ return getColor(BaseFore); }
	const UColor & background()	const	{ return getColor(Background); }
	const UColor & foreground()	const	{ return getColor(Foreground); }
	const UColor & text()	const	{ return getColor(Text); }
	
	const UColor & light()	const	{ return getColor(Light); }
	const UColor & dark()	const	{ return getColor(Dark); }
	const UColor & midLight()	const	{ return getColor(MidLight); }
	const UColor & highlight()	const	{ return getColor(Highlight); }
	const UColor & highlightedText()	const	{ return getColor(HighlightedText); }

public: // Public operators
	bool operator==(const UColorGroup & group) const;
	bool operator!=(const UColorGroup & group) const;

protected: // Overrides UObject
	virtual std::ostream & paramString(std::ostream & os) const;

private: // Private methods
	//void detach();

private:
	UColor m_colors[MaxColorRoles]; // USharedPtr<UColor*> m_colors;// 
};

//
// inline implementation
//

inline void 
UColorGroup::setColor(ColorRole role, const UColor & color) {
	//detach();
	m_colors[role] = color;
}

inline const UColor &
UColorGroup::getColor(ColorRole role) const {
	return m_colors[role];
}

inline bool
UColorGroup::operator!=(const UColorGroup & group) const {
	return !(operator==(group));
}

} // namespace ufo

#endif // UCOLORGROUP_HPP
