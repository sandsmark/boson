/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ugl_builtinfontrenderer.cpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/gl/ugl_builtinfontrenderer.hpp"

#include "ufo/gl/ugl_driver.hpp"
#include "ufo/uplugin.hpp"

using namespace ufo;

// these vars come from ugl_font_data.cpp

// we need to define those classes here to allow
// type safe overriding of functions
namespace ufo {
class UGL_BuiltinFontMetrics : public UFontMetrics {
	UFO_DECLARE_ABSTRACT_CLASS(UGL_BuiltinFontMetrics)
public:
	UGL_BuiltinFontMetrics(UGL_BuiltinFontRenderer * renderer)
		: m_renderer(renderer) {}

	UFontRenderer * getFontRenderer() const  { return m_renderer; }
	int getAscent() const;
	int getDescent() const;
	int getLineskip() const { return getHeight() + 2; }
	int getHeight() const { return getAscent() + getDescent(); }

	int getMaxAscent() const;
	int getMaxDescent() const;
	int getMaxCharWidth() const;

	int getUnderlinePosition() const { return 2; }
	int getUnderlineThickness() const { return 1; }


	int getStringWidth(const char * text, unsigned int nChar) const;

	int getCharWidth(const wchar_t chA) const;

	unsigned int viewToModel(const char * text, unsigned int nChar,
		unsigned int w) const;
private:
	UGL_BuiltinFontRenderer * m_renderer;
}; // UTextureFont::UTextFontMetrics
}
UFO_IMPLEMENT_ABSTRACT_CLASS(UGL_BuiltinFontMetrics, UFontMetrics)

UFO_IMPLEMENT_ABSTRACT_CLASS(UGL_BuiltinFontRenderer, UFontRenderer)

//extern const uint8_t ** Helvetica12_Character_Map;

UGL_BuiltinFontRenderer::UGL_BuiltinFontRenderer(const UFontInfo & fontInfo)
	: m_fontInfo(fontInfo)
	, m_fontMetrics(new UGL_BuiltinFontMetrics(this))
	, m_fontStruct()
	, m_xorig(-1.0f)
	, m_yorig(3.0f)
{
	m_fontInfo.style = UFontInfo::Plain;
	m_fontInfo.encoding = UFontInfo::Encoding_ISO8859_1;
	m_fontInfo.face = "";

	if (fontInfo.family == UFontInfo::DefaultFamily ||
			fontInfo.family == UFontInfo::SansSerif ||
			fontInfo.family == UFontInfo::Decorative) {
		m_fontInfo.family = UFontInfo::SansSerif;
		if (fontInfo.pointSize <= 11.f && fontInfo.pointSize > 0) {
			m_fontStruct = s_font_helvetica10;
			m_fontInfo.pointSize = 10;
		} else {
			m_fontStruct = s_font_helvetica12;
			m_fontInfo.pointSize = 12;
		}
	} else if (fontInfo.family == UFontInfo::Serif) {
		m_fontInfo.family = UFontInfo::Serif;
		m_fontInfo.pointSize = 10;
		m_fontStruct = s_font_timesRoman10;
	} else {
		// mono spaced
		m_fontInfo.family = UFontInfo::MonoSpaced;
		m_fontInfo.pointSize = 8;
		m_fontStruct = s_font_fixed8x13;
	}
}


UGL_BuiltinFontRenderer::~UGL_BuiltinFontRenderer() {
}

int
UGL_BuiltinFontRenderer::drawString(UGraphics * g, const char * text,
		unsigned int nChar,  int xA, int yA) {
	beginDrawing(g);

	int advance = 0;
	int height = m_fontStruct.m_ascent + m_fontStruct.m_descent;
	ugl_driver->glRasterPos2i(xA, yA + height);// + m_height - 4 - 1);

	// Step through the string, drawing each character.
	for(unsigned int c = 0; c < nChar; c++) {
		/*if(text[c] == '\n' ) {
			raster_position[ 1 ] -= ( float )font->Height;
			glRasterPos4fv( raster_position );
		} else */ // Not an EOL, draw the bitmap character
		{
			const GLubyte* face = m_fontStruct.m_characters[uint8_t(text[c])];

			ugl_driver->glBitmap(
				face[0], height,    // Bitmap's width and height
				0, 0,//m_xorig, m_yorig,     // The origin in the font glyph
				(float)(face[0]), 0.0,// The raster advance; inc. x,y
				(face + 1)            // The packed bitmap data...
			);
			advance += face[0];
		}
	}
	endDrawing(g);
	// FIXME
	return advance;
}

void
UGL_BuiltinFontRenderer::beginDrawing(UGraphics * g) {
	ugl_driver->glGetFloatv(GL_CURRENT_RASTER_POSITION, m_raster_position);
	ugl_driver->glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
	ugl_driver->glPixelStorei(GL_UNPACK_SWAP_BYTES,  GL_FALSE);
	ugl_driver->glPixelStorei(GL_UNPACK_LSB_FIRST,   GL_FALSE);
	ugl_driver->glPixelStorei(GL_UNPACK_ROW_LENGTH,  0       );
	ugl_driver->glPixelStorei(GL_UNPACK_SKIP_ROWS,   0       );
	ugl_driver->glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0       );
	ugl_driver->glPixelStorei(GL_UNPACK_ALIGNMENT,   1       );
}

void
UGL_BuiltinFontRenderer::endDrawing(UGraphics * g) {
    ugl_driver->glPopClientAttrib();
}

const UFontMetrics *
UGL_BuiltinFontRenderer::getFontMetrics() const {
	return m_fontMetrics;
}

UFontInfo
UGL_BuiltinFontRenderer::getFontInfo() const {
	return m_fontInfo;
}

std::string
UGL_BuiltinFontRenderer::getSystemName() const {
	return "builtin_font";
}

void
UGL_BuiltinFontRenderer::refresh() {
}

int
UGL_BuiltinFontMetrics::getAscent() const {
	return m_renderer->m_fontStruct.m_ascent;//m_height - 4;
}

int
UGL_BuiltinFontMetrics::getDescent() const {
	return m_renderer->m_fontStruct.m_descent;//4;
}

int
UGL_BuiltinFontMetrics::getMaxAscent() const {
	return m_renderer->m_fontStruct.m_maxAscent;
}
int
UGL_BuiltinFontMetrics::getMaxDescent() const {
	return m_renderer->m_fontStruct.m_maxDescent;
}
int
UGL_BuiltinFontMetrics::getMaxCharWidth() const {
	return m_renderer->m_fontStruct.m_maxWidth;
}

int
UGL_BuiltinFontMetrics::getStringWidth(const char * text, unsigned int nChar) const {
	int advance = 0;
	for(unsigned int c = 0; c < nChar; c++) {
		/*if(text[c] == '\n' ) {
			raster_position[ 1 ] -= ( float )font->Height;
			glRasterPos4fv( raster_position );
		} else */ // Not an EOL, draw the bitmap character
		{
			advance += (m_renderer->m_fontStruct.m_characters[uint8_t(text[c])])[0];
		}
	}
	return advance;
}

int
UGL_BuiltinFontMetrics::getCharWidth(const wchar_t chA) const {
	return (m_renderer->m_fontStruct.m_characters[uint8_t(chA)])[0];
}

unsigned int
UGL_BuiltinFontMetrics::viewToModel(const char * text, unsigned int nChar,
		unsigned int w) const {
	float advance = 0.0f;
	unsigned int index = 0;

	for (;index < nChar; ++index) {
		advance += (m_renderer->m_fontStruct.m_characters[uint8_t(text[index])])[0];
		if (advance > w) {
			index--;
			break;
		}
	}
	return index;
}

class UGL_BuiltinFontPlugin : public UFontPlugin {
	UFO_DECLARE_DYNAMIC_CLASS(UGL_BuiltinFontPlugin)
public:
	virtual UFontRenderer * createFontRenderer(const UFontInfo & fontInfo) {
		UGL_BuiltinFontRenderer * ret = new UGL_BuiltinFontRenderer(fontInfo);

		trackPointer(ret);
		return ret;
	}

	virtual UFontInfo queryFont(const UFontInfo & fontInfo) {
		UFontInfo ret;
		ret.face = "";
		if (fontInfo.family == UFontInfo::DefaultFamily ||
				fontInfo.family == UFontInfo::SansSerif ||
				fontInfo.family == UFontInfo::Decorative) {
			ret.family = UFontInfo::SansSerif;
			if (fontInfo.pointSize <= 11.f) {
				ret.pointSize = 10;
			} else {
				ret.pointSize = 12;
			}
		} else if (fontInfo.family == UFontInfo::Serif) {
			ret.family = UFontInfo::Serif;
			ret.pointSize = 10;
		} else {
			// mono spaced
			ret.family = UFontInfo::MonoSpaced;
			ret.pointSize = 8;
		}
		ret.style = UFontInfo::Plain;
		ret.encoding = UFontInfo::Encoding_ISO8859_1;
		return ret;
	}
	virtual std::vector<UFontInfo> listFonts(const UFontInfo & fontInfo) {
		UFontInfo info = queryFont(fontInfo);
		std::vector<UFontInfo> ret;
		ret.push_back(info);
		return ret;
	}
	virtual std::vector<UFontInfo> listFonts() {
		std::vector<UFontInfo> ret;
		ret.push_back(UFontInfo(UFontInfo::SansSerif, 10, UFontInfo::Normal));
		ret.push_back(UFontInfo(UFontInfo::SansSerif, 12, UFontInfo::Normal));
		ret.push_back(UFontInfo(UFontInfo::Serif, 10, UFontInfo::Normal));
		ret.push_back(UFontInfo(UFontInfo::MonoSpaced, 8, UFontInfo::Normal));
		return ret;
	}

};
UFO_IMPLEMENT_DYNAMIC_CLASS(UGL_BuiltinFontPlugin, UFontPlugin)


UPluginBase *
UGL_BuiltinFontRenderer::createPlugin() {
	return new UGL_BuiltinFontPlugin();
}

void
UGL_BuiltinFontRenderer::destroyPlugin(UPluginBase * plugin) {
	if (dynamic_cast<UGL_BuiltinFontPlugin*>(plugin)) {
		delete (plugin);
	}
}
