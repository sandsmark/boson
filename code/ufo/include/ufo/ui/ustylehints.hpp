/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/ustylehints.hpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#ifndef USTYLEHINTS_HPP
#define USTYLEHINTS_HPP

#include "../uobject.hpp"

#include "../uicon.hpp"
#include "../udrawable.hpp"

#include "../font/ufont.hpp"

#include "../util/udimension.hpp"
#include "../util/uinsets.hpp"
#include "../util/upalette.hpp"

namespace ufo {

class UBorderModel;

/** @short The style hints contain colors, fonts and other properties which
  *  describe the visual appearance of widgets.
  * @ingroup appearance
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UStyleHints {
public:
	enum StyleHint {
		MinimumSizeHint,
		MaximumSizeHint,
		PreferredSizeHint,
		BorderHint,
		MarginHint,
		HAlignmentHint,
		VAlignmentHint,
		DirectionHint,
		OrientationHint,
		FontHint,
		PaletteHint,
		OpacityHint,
		BackgroundHint,
		IconHint,
		AllHints
	};
public:
	UStyleHints();
	~UStyleHints();
	/** Every hint of @p hints which is no default hint of the default
	  * constructor overwrites the appropriate value of this style hint.
	  * Example: If hints->border != UStyleHint().border then
	  * this->border becomes hints->border.
	  */
	void transcribe(UStyleHints * hints);
	/** Updates every default value of this style hint with the
	  * given values of @p hints.
	  * Example: If this->border == UStyleHint().border then
	  * this->border becomes hints->border.
	  */
	void update(UStyleHints * hints);

	UStyleHints * clone() const;

	friend std::ostream & operator<<(std::ostream & os, const UStyleHints * hints);
public:
	UDimension minimumSize;
	UDimension maximumSize;
	UDimension preferredSize;

	/** the border of this widget */
	UBorderModel * border;
	/** the margin between widget content and border */
	UInsets margin;

	// alignment
	Alignment hAlignment : 4;
	Alignment vAlignment : 4;
	Direction direction : 4;
	Orientation orientation : 4;

	/** the font of this widget */
	UFont font;

	UPalette palette;

	float opacity;

	UDrawable * background;
	UIcon * icon;
};

extern std::ostream &
operator<<(std::ostream & os, const UStyleHints * hints);


} // namespace ufo

#endif // USTYLEHINTS_HPP
