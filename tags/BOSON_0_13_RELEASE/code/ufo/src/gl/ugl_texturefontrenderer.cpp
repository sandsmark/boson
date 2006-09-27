/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/gl/ugl_texturefontrenderer.cpp
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

#include "ufo/gl/ugl_texturefontrenderer.hpp"

// for texture loading
#include "ufo/utoolkit.hpp"
#include "ufo/udisplay.hpp"
#include "ufo/gl/ugl_driver.hpp"

#include "ufo/util/ufilearchive.hpp"
#include "ufo/image/uimageio.hpp"

// reuse font info methods
#include "ufo/font/utexturefont.hpp"

using namespace ufo;

	// we need to define those classes here to allow
	// type safe overriding of functions
namespace ufo {
class UGL_TextureFontMetrics : public UFontMetrics {
	UFO_DECLARE_ABSTRACT_CLASS(UGL_TextureFontMetrics)
public:
	UGL_TextureFontMetrics(UGL_TextureFontRenderer * fontA)
		: m_renderer(fontA) {}

	UFontRenderer * getFontRenderer() const  { return m_renderer; }
	int getAscent() const;
	int getDescent() const;
	int getLineskip() const { return getHeight() + 2; }
	int getHeight() const { return getAscent() + getDescent(); }

	int getMaxAscent() const;
	int getMaxDescent() const;
	int getMaxCharWidth() const;

	int getUnderlinePosition() const { return 1; }
	int getUnderlineThickness() const { return 1; }


	int getStringWidth(const char * text, unsigned int nChar) const;

	int getCharWidth(const wchar_t chA) const;

	unsigned int viewToModel(const char * text, unsigned int nChar,
		unsigned int w) const;
private:
	UGL_TextureFontRenderer * m_renderer;
}; // UTextureFont::UTextFontMetrics
UFO_IMPLEMENT_ABSTRACT_CLASS(UGL_TextureFontMetrics, UFontMetrics)

struct CharStruct {
	CharStruct()
		: lbearing(0), rbearing(0), width(0), ascent(0), descent(0) {}
	short lbearing;	// origin to left edge of raster
	short rbearing;	// origin to right edge of raster
	short width;		// advance to next char's origin
	short ascent;		// baseline to top edge of raster
	short descent;	// baseline to bottom edge of raster
};

struct UGL_TextureFontData {
	UGL_TextureFontData() : m_chars(), m_maxBounds(), m_textureIndex(0) {}
	CharStruct m_chars[256];
	CharStruct m_maxBounds;
	uint32_t m_textureIndex;
};
/*
class UGL_TextureFontCache : public UObject {
public:
	UGL_TextureFontCache(const std::string & fileNameA);
	~UGL_TextureFontCache();

	uint32_t createTexture(UImageIO * imageIO);
	UImageIO * loadImageFile();

	void refresh();
	void dispose();
public: // Public attributes
	CharStruct m_chars[256];
	CharStruct m_maxBounds;

private: // Private methods
	UImageIO * loadImageFile();
	void genGlyphMetrics(UImageIO * ioA);

public:  // Private attributes
	std::string m_fileName;
	UGL_Image * m_image;

}; // UTextureFont::UTextureFontCache
*/
} // namespace ufo


UFO_IMPLEMENT_ABSTRACT_CLASS(UGL_TextureFontRenderer, UFontRenderer)

//UGL_TextureFontRenderer::CacheType UGL_TextureFontRenderer::m_fontCache;

UGL_TextureFontRenderer::UGL_TextureFontRenderer(const UFontInfo & fontInfo)
	: m_fontInfo(fontInfo)
	, m_fontMetrics(NULL)
	, m_systemName(fontInfo.face)
	, m_data(new UGL_TextureFontData)
	, m_isValid(false)
{
	UImageIO * io = loadImageFile();
	io->reference();

	if (io->getPixels() != NULL) {
		genGlyphMetrics(io);

		if (UToolkit::getToolkit()->getCurrentContext()) {
			createTexture(io);
		}
		m_isValid = true;
	}

	m_fontMetrics = new UGL_TextureFontMetrics(this);
	io->unreference();
}

UGL_TextureFontRenderer::~UGL_TextureFontRenderer() {
	delete (m_fontMetrics);
	delete (m_data);
}

bool
UGL_TextureFontRenderer::isValid() const {
	return m_isValid;
}

int
UGL_TextureFontRenderer::drawString(UGraphics * g, const char * text, unsigned int nChar,
		int xA, int yA) {
	if (m_data->m_textureIndex == 0) {
		refresh();
	}
	if (m_data->m_textureIndex == 0) {
		// FIXME: warning: couldn't load texture
		return 0;
	}
	beginDrawing(g);
	register float texAdvance = 1.0f / 16.0f;

	register float x = xA;
	float y = yA;
	for (unsigned int i = 0; i < nChar; ++i) {
		CharStruct cStruct = m_data->m_chars[uint8_t(text[i])];

		float texx = (uint8_t(text[i]) % 16) / 16.f;
		float texy = (uint8_t(text[i]) / 16) / 16.f;
		float left = x - cStruct.lbearing;// * m_multiplier;

		ugl_driver->glTexCoord2f(texx, texy);
		ugl_driver->glVertex2f(left, y);

		ugl_driver->glTexCoord2f(texx, texy + texAdvance);
		ugl_driver->glVertex2f(left, y + 16);

		ugl_driver->glTexCoord2f(texx + texAdvance, texy + texAdvance);
		ugl_driver->glVertex2f(left + 16, y + 16);

		ugl_driver->glTexCoord2f(texx + texAdvance, texy);
		ugl_driver->glVertex2f(left + 16, y);

		x += cStruct.width;// * m_multiplier;
	}

	endDrawing(g);
	// FIXME
	return int(x - xA + 0.5f);
}

void
UGL_TextureFontRenderer::beginDrawing(UGraphics * g) {
	ugl_driver->glEnable(GL_TEXTURE_2D);

	ugl_driver->glEnable(GL_BLEND);
	ugl_driver->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //

	ugl_driver->glBindTexture(GL_TEXTURE_2D, m_data->m_textureIndex);

	ugl_driver->glBegin(GL_QUADS);
}

void
UGL_TextureFontRenderer::endDrawing(UGraphics * g) {
	ugl_driver->glEnd();
	ugl_driver->glDisable(GL_TEXTURE_2D);
	ugl_driver->glDisable(GL_BLEND);
}

const UFontMetrics *
UGL_TextureFontRenderer::getFontMetrics() const {
	return m_fontMetrics;
}

UFontInfo
UGL_TextureFontRenderer::getFontInfo() const {
	return m_fontInfo;
}

std::string
UGL_TextureFontRenderer::getSystemName() const {
	return m_systemName;
}

void
UGL_TextureFontRenderer::refresh() {
	setDisplay(UDisplay::getDefault());
	//m_fontCache[m_systemName]->refresh();
	UImageIO * io = loadImageFile();
	io->reference();
	createTexture(io);
	io->unreference();
	m_isValid = true;
}

UImageIO *
UGL_TextureFontRenderer::loadImageFile() {
	UImageIO * ioL = new UImageIO();

	UFontInfo queryInfo = UTextureFontRenderer::queryFont(m_fontInfo);

	std::string fileName;
	// determine file name
	if (queryInfo.face.empty()) {
		queryInfo.face = "Arial";
	}
	fileName = queryInfo.face;

	if (queryInfo.weight > UFontInfo::Normal) {
		fileName.append("_Bold");
	}
	if (queryInfo.style & UFontInfo::Italic) {
		fileName.append("_Italic");
	}

	std::string pointSize = UString::toString(queryInfo.pointSize);
	fileName.append("_");
	fileName.append(pointSize);
	// we are using tga images for font files
	fileName.append(".tga");

	// load from font directory
	UFileArchive * archive = new UFileArchive(UToolkit::getToolkit()->getFontDir());
	archive->reference();
	if (archive->existsInArchive(fileName)) {
		ioL->load(archive->getAbsolutePath(fileName));
	} else {
		// discard
		fileName = queryInfo.face;
		fileName.append(pointSize);
		fileName.append(".tga");
		if (archive->existsInArchive(fileName)) {
			ioL->load(archive->getAbsolutePath(fileName));
		} else {
			uError() << "No valid image file for texture font renderer.\n"
			<< "Sorry, couldn't find any font files for UGL_TextureFontRenderer.\n"
			<< "Check your media data and your font path.\n";
			/*
			// Courier.pnm should always exist
			fileName = "Courier.pnm";
			if (archive->existsInArchive(fileName)) {
				ioL->load(archive->getAbsolutePath(fileName));
			} else {
				// last resort
				ioL->loadFromArchive(fileName);
			}
			*/
		}
	}
	m_systemName = fileName;
	m_fontInfo = queryInfo;

	archive->unreference();

	// FIXME
	// add a warning on failure

	return ioL;
}

void
UGL_TextureFontRenderer::createTexture(UImageIO * io) {
	io->reference();

	UDimension size = io->getSize();
	// set standard format
	GLenum imageFormat = GL_RGBA;
	// shouldn't happen, but doesn't hurt
	if (io->getImageComponents() == 1) {
		imageFormat = GL_LUMINANCE;
	} else if (io->getImageComponents() == 2) {
		imageFormat = GL_LUMINANCE_ALPHA;
	} else if(io->getImageComponents() == 3) {
		imageFormat = GL_RGB;
	}
	// save texture memory
	GLenum internalFormat = GL_INTENSITY;

	ugl_driver->glGenTextures(1, &m_data->m_textureIndex);
	ugl_driver->glBindTexture(GL_TEXTURE_2D, m_data->m_textureIndex);

	ugl_driver->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// repeat texture if tex is bigger ?
	ugl_driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	ugl_driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// scale only with nearest
	ugl_driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	ugl_driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// ??
	ugl_driver->glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	ugl_driver->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.w, size.h,
		  0, imageFormat, GL_UNSIGNED_BYTE, io->getPixels());


	io->unreference();
}

static int
searchLeft(unsigned char * data, int bpp) {
	for (int i = 0; i < 16; ++i) {
		if (data[i * bpp] != 0) {
			return i;
		}
	}
	return 16;
}
static int
searchRight(unsigned char * data, int bpp) {
	for (int i = 15; i >=0; --i) {
		if (data[i * bpp] != 0) {
			return i;
		}
	}
	return 0;
}

static void
correctGlyph(UImageIO * ioA, int indexA, CharStruct * charStruct) {
	// both, width and height have to be 256
	int x_index = (indexA % 16) * 16;
	int y_index = (indexA / 16) * 16;

	unsigned char * pixels = ioA->getPixels();
	int bpp = ioA->getImageComponents();
	bool topDone = false;

	int topLines = 0;
	int bottomLines = 0;

	int leftMost = 16;
	int rightMost = 0;

	// get bounding box

	for (int y = y_index; y < y_index + 16; y++) {
		int left = searchLeft(pixels + y * 256 * bpp + x_index * bpp, bpp);
		leftMost = std::min(leftMost, left);
		if (left != 16) {
			int right = searchRight(pixels + y * 256 * bpp + x_index * bpp, bpp);
			rightMost = std::max(rightMost, right);
			topDone = true;
		} else {
			if (!topDone) {
				topLines++;
			} else {
				bottomLines++;
			}
		}
	}

	if (leftMost == 16) {
		// white space
		leftMost = 0;
		rightMost = 3;
	}
	if (rightMost == 0) {
		rightMost = 15;
	}
	charStruct->lbearing = leftMost;
	charStruct->rbearing = rightMost;
	charStruct->width = rightMost - leftMost + 2;
	// FIXME
	// we are using 4 as baseline
	charStruct->ascent = 12 - topLines; // - bottomLines;
	charStruct->descent = 4 - bottomLines;
}

void
UGL_TextureFontRenderer::genGlyphMetrics(UImageIO * ioA) {
	for (int i=0;i<256;i++) {
		correctGlyph(ioA, i, &m_data->m_chars[i]);//&m_width[i], &m_lshift[i]);
		// construct max bounds
		// FIXME this is damn slow
		if (m_data->m_chars[i].ascent > m_data->m_maxBounds.ascent) {
			m_data->m_maxBounds.ascent = m_data->m_chars[i].ascent;
		}
		if (m_data->m_chars[i].descent > m_data->m_maxBounds.descent) {
			m_data->m_maxBounds.descent = m_data->m_chars[i].descent;
		}
		if (m_data->m_chars[i].width > m_data->m_maxBounds.width) {
			m_data->m_maxBounds.width = m_data->m_chars[i].width;
		}
		if (m_data->m_chars[i].lbearing > m_data->m_maxBounds.lbearing) {
			m_data->m_maxBounds.lbearing = m_data->m_chars[i].lbearing;
		}
		if (m_data->m_chars[i].rbearing > m_data->m_maxBounds.rbearing) {
			m_data->m_maxBounds.rbearing = m_data->m_chars[i].rbearing;
		}
	}
}


int
UGL_TextureFontMetrics::getAscent() const {
	return int(m_renderer->m_data->m_maxBounds.ascent + 0.5f);
}

int
UGL_TextureFontMetrics::getDescent() const {
	return int(m_renderer->m_data->m_maxBounds.descent + 0.5f);
}

int
UGL_TextureFontMetrics::getMaxAscent() const {
	return int(m_renderer->m_data->m_maxBounds.ascent + 0.5f);
}
int
UGL_TextureFontMetrics::getMaxDescent() const {
	return int(m_renderer->m_data->m_maxBounds.descent + 0.5f);
}
int
UGL_TextureFontMetrics::getMaxCharWidth() const {
	return int(m_renderer->m_data->m_maxBounds.width + 0.5f);
}

int
UGL_TextureFontMetrics::getStringWidth(const char * text, unsigned int nChar) const {
	float ret = 0;
	for (unsigned int i = 0; i < nChar; ++i) {
		if (!iscntrl(text[i]))
		ret += m_renderer->m_data->m_chars[uint8_t(text[i])].width;
	}
	return int(ret + 0.5f);
}

int
UGL_TextureFontMetrics::getCharWidth(const wchar_t chA) const {
	unsigned char ch = chA;
	float ret = m_renderer->m_data->m_chars[ch].width;
	return int(ret + 0.5f);
}

unsigned int
UGL_TextureFontMetrics::viewToModel(const char * text, unsigned int nChar,
		unsigned int w) const {
	float advance = 0.0f;
	unsigned int index = 0;

	for (;index < nChar; ++index) {
		advance += m_renderer->m_data->m_chars[uint8_t(text[index])].width;
		if (advance > w) {
			if (index > 0) {
				index--;
			}
			break;
		}
	}
	return index;
}

class UGL_TextureFontPlugin : public UFontPlugin {
	UFO_DECLARE_DYNAMIC_CLASS(UGL_TextureFontPlugin)
public:
	virtual UFontRenderer * createFontRenderer(const UFontInfo & fontInfo) {
		UGL_TextureFontRenderer * ret = new UGL_TextureFontRenderer(fontInfo);

		if (ret->isValid()) {
			trackPointer(ret);
			return ret;
		} else {
			delete ret;
			return NULL;
		}
	}

	virtual UFontInfo queryFont(const UFontInfo & fontInfo) {
		return UTextureFontRenderer::queryFont(fontInfo);
	}
	virtual std::vector<UFontInfo> listFonts(const UFontInfo & fontInfo) {
		return UTextureFontRenderer::listFonts(fontInfo);
	}
	virtual std::vector<UFontInfo> listFonts() {
		return UTextureFontRenderer::listFonts();
	}

};
UFO_IMPLEMENT_DYNAMIC_CLASS(UGL_TextureFontPlugin, UFontPlugin)


UPluginBase *
UGL_TextureFontRenderer::createPlugin() {
	return new UGL_TextureFontPlugin();
}

void
UGL_TextureFontRenderer::destroyPlugin(UPluginBase * plugin) {
	if (dynamic_cast<UGL_TextureFontPlugin*>(plugin)) {
		delete (plugin);
	}
}
