/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/gl/ugl_builtinfontrenderer.hpp
    begin             : Sat Jun 5 2004
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

#ifndef UGL_BUILTINFONTRENDERER_HPP
#define UGL_BUILTINFONTRENDERER_HPP

#include "../font/ufontrenderer.hpp"

#include <vector>

#include "../font/ufontinfo.hpp"
#include "../font/ufontmetrics.hpp"


namespace ufo {

class UImageIO;

class UFontPlugin;
class UPluginBase;
class UGL_BuiltinFontMetrics;

// damn ms vc6
struct UBuiltinFontStruct {
	const char * m_systemName;
	int m_ascent;
	int m_descent;
	int m_maxAscent;
	int m_maxDescent;
	int m_maxWidth;
	const uint8_t ** m_characters;
};

extern const UBuiltinFontStruct s_font_fixed8x13;
//extern UBuiltinFontStruct s_font_fixed9x15;
extern const UBuiltinFontStruct s_font_helvetica10;
extern const UBuiltinFontStruct s_font_helvetica12;
//extern UBuiltinFontStruct s_font_helvetica18;
extern const UBuiltinFontStruct s_font_timesRoman10;
//extern UBuiltinFontStruct s_font_timesRoman24;


/** @short An OpenGL implementation of a fixed point size font (used as fallback).
  * @ingroup internal
  * @ingroup opengl
  *
  * This font renderer uses built in byte arrays to draw characters.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UGL_BuiltinFontRenderer : public UFontRenderer {
	UFO_DECLARE_ABSTRACT_CLASS(UGL_BuiltinFontRenderer)
private:
	friend class UGL_BuiltinFontMetrics;
public:
	/** Creates one of the four built-in fonts.
	  * @param fontInfo
	  */
	UGL_BuiltinFontRenderer(const UFontInfo & fontInfo);
	~UGL_BuiltinFontRenderer();

public: // Implements UFontRenderer
	int drawString(UGraphics * g, const char * text, unsigned int nChar,
		int xA = 0, int yA = 0);

	void beginDrawing(UGraphics * g);
	void endDrawing(UGraphics * g);

	void refresh();

	const UFontMetrics * getFontMetrics() const;

	UFontInfo getFontInfo() const;

	std::string getSystemName() const;

public: //
	UBuiltinFontStruct getFontStruct() const;

private: // Private methods
	void createTexture(UImageIO * imageIO);
	UImageIO * loadImageFile();
	void genGlyphMetrics(UImageIO * imageIO);

public: // plugin methods
	static UPluginBase * createPlugin();
	static void destroyPlugin(UPluginBase * plugin);

private:  // Private attributes
	UFontInfo m_fontInfo;
	UGL_BuiltinFontMetrics * m_fontMetrics;

	UBuiltinFontStruct m_fontStruct;
	/** Relative origin of the character */
	float m_xorig;
	/** Relative origin of the character */
	float m_yorig;

	float m_raster_position[4];
}; // UGL_BuiltinFontRenderer

} // namespac ufo

#endif // UGL_BUILTINFONTRENDERER_HPP
