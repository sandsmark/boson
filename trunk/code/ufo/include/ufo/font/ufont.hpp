/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/font/ufont.hpp
    begin             : Mon Oct 1 2001
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

#ifndef UFONT_HPP
#define UFONT_HPP

#include "../uobject.hpp"

#include <map>

#include "ufontinfo.hpp"
#include "ufontrenderer.hpp"

namespace ufo {

class UFontMetrics;
class UDisplay;

/** @short This class is a controller class for abstract font renderers.
  * @ingroup appearance
  * @ingroup text
  * @ingroup drawing
  *
  * It uses the active font plugin to create new font renderers according
  * to its attributes.
  * The preferred way to create font with a specific font face is to use one
  * of the predefined font faces.
  * Alternatively, you may use an explicit string literal defining your font
  * face
  * This class may use caching of already existing font renderers.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UFont : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UFont)
public: // Public c'tors
	/** Creates a new font object using the default font renderer. */
	UFont();
	/** Creates a new font and chooses an appropriate font renderer.
	  * The font renderer may be internally cached.
	  * @param family One of the predefined font families
	  * @param pointSize The point size as float value
	  * @param weight The font weight as described by enum Weight
	  * @param style The font style as described by enum Style
	  * @param encoding The character encoding used to interpret the chars
	  */
	UFont(UFontInfo::Family family, float pointSize, int weight = UFontInfo::Normal,
		int style = UFontInfo::Plain, UFontInfo::Encoding encoding = UFontInfo::Encoding_Default);
	/** Creates a new font and chooses an appropriate font renderer.
	  * The font renderer may be internally cached.
	  * @param face A custom font face name
	  * @param pointSize The point size as float value
	  * @param weight The font weight as described by enum Weight
	  * @param encoding The character encoding used to interpret the chars
	  */
	UFont(const std::string & face, float pointSize, int weight = UFontInfo::Normal,
		int style = UFontInfo::Plain, UFontInfo::Encoding encoding = UFontInfo::Encoding_Default);
	/** Creates a new font and chooses an appropriate font renderer.
	  * The font renderer may be internally cached.
	  * @param fontInfo The font info struct describing the desired font
	  */
	UFont(const UFontInfo & fontInfo);
	/** Constructs a font object with the given font renderer.
	  * The font renderer will not be registered at the cache.
	  */
	UFont(UFontRenderer * renderer);

	/** Explicit copy constructor */
	UFont(const UFont & font);
	UFont& operator=(const UFont & font);

	virtual ~UFont();

public: // Public methods
	/** Returns the font renderer used to draw the character strings. */
	UFontRenderer * getRenderer() const;
	/** Returns a font metrics object which describes the font extents. */
	const UFontMetrics * getFontMetrics() const;

	/** Sets the font family.
	  * May create a new font renderer.
	  */
	void setFamiliy(UFontInfo::Family family);

	void setFontFace(const std::string & face);
	/** Returns the font face name for this font
	  * @return The font face for this font.
	  */
	std::string getFontFace() const;

	/** Sets the point size of this font.
	  * May create a new font renderer object
	  */
	void setPointSize(float pointSize);
	/** Returns the point size rounded to integer
	  */
	float getPointSize() const;

	void setWeight(int weight);
	int getWeight() const;

	void setStyle(int flag);
	/** The font style is described by an integer
	  * @see setStyle(int)
	  * @return the current font style
	  */
	int getStyle() const;

	/** Sets the character encoding which should be used to interpret
	  * character strings.
	  * @see Encoding
	  */
	void setEncoding(UFontInfo::Encoding encoding);
	UFontInfo::Encoding getEncoding() const;

public: // Public static methods
	static void clearCache();

protected: // Private methods
	/** Queries the toolkit for the font info struct, searches in the
	  * font cache and creates a new font renderer on no results.
	  */
	void queryAndLoad(const UFontInfo & info);
	/** Disposes the font renderer. */
	void dispose();

private: // Private static attributes
	typedef std::map<UFontInfo, UFontRenderer*> fontCache_t;
	typedef std::map<UDisplay*, fontCache_t> displayCache_t;
	//static fontCache_t sm_fontCache;
	static displayCache_t sm_fontCache;

private: // Private attributes
	/** The currently used font renderer. */
	UFontRenderer * m_renderer;
	/** A cache for the currently used info. */
	UFontInfo m_infoCache;
};

} // namespace ufo

#endif // UFONT_HPP
