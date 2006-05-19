/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/upalette.hpp
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

#ifndef UPALETTE_HPP
#define UPALETTE_HPP

#include "../uobject.hpp"

#include "ucolor.hpp"

namespace ufo {

/** A palettes represents the color set used to paint a widget.
  * @author Johannes Schmidt
  */
class UFO_EXPORT UPalette : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UPalette)
public: // Public types
	/** Currently not used. */
	enum ColorGroup {
		Active,
		Inactive,
		Disabled,
		NColorGroupType
	};
	/** Identifiers for different colors used in a GUI. */
	enum ColorRole {
		/** The background color for normal widgets. */
		Background,
		/** suitable foreground color for drawing over the widget. */
		Foreground,
		/** Background color of text entries. */
		Base ,
		/** suitable foreground color for text, usually the same as foreground. */
		Text,
		/** Some GUIs need different background colors for buttons,
		  * but usually the same as background. */
		Button,
		/** Foreground color for Button. */
		ButtonText,
		/** A lighter color than Button. */
		Light,
		/** Between Light and Button. */
		MidLight,
		/** A darker color than Button. */
		Dark,
		/** Between Dark and Button. */
		Mid,
		/** Used for selected or current item and text background. */
		Highlight,
		/** Text color used for Highlight. */
		HighlightedText,
		/** Used for unvisited hyper links. */
		Link,
		/** Used for visited hyper links. */
		LinkVisited,
		MaxColorRoles
	};
public:
	UPalette();
	UPalette(
		const UColor & background, const UColor & foreground,
		const UColor & base, const UColor & text,
		const UColor & light, const UColor & midLight,
		const UColor & dark, const UColor & mid,
		const UColor & highlight, const UColor & highlightedText,
		const UColor & button, const UColor & buttonText,
		const UColor & link, const UColor & linkVisited
	);
	UPalette(
		const UColor & background, const UColor & foreground,
		const UColor & base, const UColor & text,
		const UColor & highlight, const UColor & highlightedText
	);
	virtual ~UPalette();


	inline void setColor(ColorRole role, const UColor & color);
	inline const UColor & getColor(ColorRole role) const;

	/** Every color of @p pal which is no default color of the default
	  * constructor overwrites the appropriate value of this palette.
	  * Example: If pal->foreground != UPalette().foreground then
	  * this->foreground becomes pal->foreground.
	  */
	void transcribe(const UPalette & pal);
	/** Updates every default value of this palette with the
	  * given values of @p pal.
	  * Example: If this->foreground == UPalette().foreground then
	  * this->foreground becomes pal->foreground.
	  */
	void update(const UPalette & pal);


	const UColor & background()	const	{ return getColor(Background); }
	const UColor & foreground()	const	{ return getColor(Foreground); }
	const UColor & base()	const	{ return getColor(Base); }
	const UColor & text()	const	{ return getColor(Text); }

	const UColor & button()	const	{ return getColor(Button); }
	const UColor & buttonText()	const	{ return getColor(ButtonText); }

	const UColor & light()	const	{ return getColor(Light); }
	const UColor & midLight()	const	{ return getColor(MidLight); }
	const UColor & dark()	const	{ return getColor(Dark); }
	const UColor & mid()	const	{ return getColor(Mid); }
	const UColor & highlight()	const	{ return getColor(Highlight); }
	const UColor & highlightedText()	const	{ return getColor(HighlightedText); }

	const UColor & link()	const	{ return getColor(Link); }
	const UColor & linkVisited()	const	{ return getColor(LinkVisited); }

	/** @not used. */
	const UColor & getColor(ColorGroup group, ColorRole role) const;
	/** @not used. */
	void setColor(ColorGroup group, ColorRole role, const UColor & color);


public: // Public operators
	bool operator==(const UPalette & pal) const;
	inline bool operator!=(const UPalette & pal) const;

protected: // Overrides UObject
	virtual std::ostream & paramString(std::ostream & os) const;

private:
	UColor m_colors[MaxColorRoles];
};


//
// inline implementation
//

inline void
UPalette::setColor(ColorRole role, const UColor & color) {
	//detach();
	m_colors[role] = color;
}

inline const UColor &
UPalette::getColor(ColorRole role) const {
	return m_colors[role];
}

inline bool
UPalette::operator!=(const UPalette & pal) const {
	return !(operator==(pal));
}

} // namespace ufo

#endif // UPALETTE_HPP
