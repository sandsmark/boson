/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/gl/ugl_texturefontrenderer.hpp
    begin             : Sat Dec 20 2003
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

#ifndef UGL_TEXTUREFONTRENDERER_HPP
#define UGL_TEXTUREFONTRENDERER_HPP

#include "../font/ufontrenderer.hpp"

#include <vector>

#include "../font/ufontinfo.hpp"
#include "../font/ufontmetrics.hpp"


namespace ufo {

class UGL_TextureFontMetrics;
class UImageIO;
struct UGL_TextureFontData;

class UFontPlugin;
class UPluginBase;

/** @short An OpenGL implementation of a texture font renderer.
  * @ingroup opengl
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UGL_TextureFontRenderer : public UFontRenderer {
	UFO_DECLARE_ABSTRACT_CLASS(UGL_TextureFontRenderer)
private:
	friend class UGL_TextureFontMetrics;
	// some forward declarations
public:
	/** @param Specifies the font familiy (like Courier, Helvetica, ...)
	  * @param pointSize The point size
	  * @param weight The font weight, e.g. UFont::Bold, UFont::Normal, ...
	  * @param style Further style attributes (UFont::Style
	  */
	UGL_TextureFontRenderer(const UFontInfo & fontInfo);

	~UGL_TextureFontRenderer();

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

private: // Private methods
	void createTexture(UImageIO * imageIO);
	UImageIO * loadImageFile();
	void genGlyphMetrics(UImageIO * imageIO);

public: // plugin methods
	static UPluginBase * createPlugin();
	static void destroyPlugin(UPluginBase * plugin);

private:  // Private attributes
	UFontInfo m_fontInfo;
	UGL_TextureFontMetrics * m_fontMetrics;
	/** The system font name */
	std::string m_systemName;

	UGL_TextureFontData * m_data;
	bool m_isValid;
}; // UGL_TextureFontRenderer

} // namespac ufo

#endif // UGL_TEXTUREFONTRENDERER_HPP
