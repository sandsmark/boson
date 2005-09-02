/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/font/utexturefont.hpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UTEXTUREFONT_HPP
#define UTEXTUREFONT_HPP

#include "ufontrenderer.hpp"

#include "ufontmetrics.hpp"
#include "ufontinfo.hpp"

#include "../util/ugeom.hpp"


namespace ufo {

class UTextureFontMetrics;
//class UTextureFontCache;
struct UTextureFontData;
class UImageIO;

class UFontPlugin;
class UPluginBase;

/** @short A basic (multimedia library independent) implementation of texture fonts.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UTextureFontRenderer : public UFontRenderer {
	UFO_DECLARE_ABSTRACT_CLASS(UTextureFontRenderer)
private:
	friend class UTextureFontMetrics;
	// some forward declarations
public:
	/** @param Specifies the font familiy (like Courier, Helvetica, ...)
	  * @param pointSize The point size
	  * @param weight The font weight, e.g. UFont::Bold, UFont::Normal, ...
	  * @param style Further style attributes (UFont::Style
	  */
	UTextureFontRenderer(const UFontInfo & fontInfo);

	~UTextureFontRenderer();

public: // Public methods
	/** Returns true when this font renderer is ready to draw characters. */
	bool isValid() const;

public: // Implements UFontRenderer
	int drawString(UGraphics * g, const char * text, unsigned int nChar,
		int xA = 0, int yA = 0);

	void beginDrawing(UGraphics * g);
	void endDrawing(UGraphics * g);

	void refresh();

	const UFontMetrics * getFontMetrics() const;

	UFontInfo getFontInfo() const;

	std::string getSystemName() const;

public: // plugin methods
	static UPluginBase * createPlugin();
	static void destroyPlugin(UPluginBase * plugin);

private: // Private methods
	void createTexture(UImageIO * imageIO);
	UImageIO * loadImageFile();
	void genGlyphMetrics(UImageIO * imageIO);

public: // Public static methods
	static UFontInfo queryFont(const UFontInfo & fontInfo);
	/** List all matching fonts. */
	static std::vector<UFontInfo> listFonts(const UFontInfo & fontInfo);
	/** List all available fonts. */
	static std::vector<UFontInfo> listFonts();

private:  // Private attributes
	UFontInfo m_fontInfo;
	UTextureFontMetrics * m_fontMetrics;
	/** The system font name */
	std::string m_systemName;

	UTextureFontData * m_data;
	bool m_isValid;
}; // UTextureFontRenderer


} // namespace ufo

#endif // UTEXTUREFONT_HPP
