/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/font/utexturefont.cpp
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

#include "ufo/font/utexturefont.hpp"

//#include "ufo/text/uattribute.hpp"

// for texture loading
#include "ufo/utoolkit.hpp"
#include "ufo/udisplay.hpp"
#include "ufo/ugraphics.hpp"
//#include "ufo/utexture.hpp"
//#include "ufo/ufo_gl.hpp"

#include "ufo/util/ufilearchive.hpp"
#include "ufo/image/uimageio.hpp"
#include "ufo/util/ucolor.hpp"

using namespace ufo;

	// we need to define those classes here to allow
	// type safe overriding of functions
namespace ufo {
class UTextureFontMetrics : public UFontMetrics {
	UFO_DECLARE_ABSTRACT_CLASS(UTextureFontMetrics)
public:
	UTextureFontMetrics(UTextureFontRenderer * fontA)
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
	UTextureFontRenderer * m_renderer;
	int m_height, m_kernAdvance;
}; // UTextureFont::UTextFontMetrics
UFO_IMPLEMENT_ABSTRACT_CLASS(UTextureFontMetrics, UFontMetrics)


struct CharStruct {
	CharStruct()
		: lbearing(0), rbearing(0), width(0), ascent(0), descent(0) {}
	short lbearing;	// origin to left edge of raster
	short rbearing;	// origin to right edge of raster
	short width;		// advance to next char's origin
	short ascent;		// baseline to top edge of raster
	short descent;	// baseline to bottom edge of raster
};

struct UTextureFontData {
	CharStruct m_chars[256];
	CharStruct m_maxBounds;
	UImage * m_image;
};
/*
class UTextureFontCache : public UObject {
public:
	UTextureFontCache(const std::string & fileNameA);
	~UTextureFontCache();

	UImage * getTexture() const;

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
	UImage * m_image;

}; // UTextureFont::UTextureFontCache
*/
} // namespace ufo

#include "ufo/font/ufont.hpp"


UFO_IMPLEMENT_ABSTRACT_CLASS(UTextureFontRenderer, UFontRenderer)

//UTextureFontRenderer::CacheType UTextureFontRenderer::m_fontCache;

UTextureFontRenderer::UTextureFontRenderer(const UFontInfo & fontInfo)
	: m_fontInfo(fontInfo)
	, m_systemName(fontInfo.face)
	, m_data(new UTextureFontData)
	, m_isValid(false)
{
	m_data->m_image = NULL;
	UImageIO * io = loadImageFile();
	io->reference();

	if (io->getPixels() != NULL) {
		genGlyphMetrics(io);

		if (UToolkit::getToolkit()->getCurrentContext()) {
			createTexture(io);
		}
		m_isValid = true;
	}

	m_fontMetrics = new UTextureFontMetrics(this);
	io->unreference();
}

UTextureFontRenderer::~UTextureFontRenderer() {
	delete (m_data);
}

bool
UTextureFontRenderer::isValid() const {
	return m_isValid;
}

int
UTextureFontRenderer::drawString(UGraphics * g, const char * text, unsigned int nChar,
		int xA, int yA) {
	if (m_data->m_image == 0) {
		refresh();
	}
	if (m_data->m_image == 0) {
		// FIXME: warning: couldn't load texture
		return 0;
	}
	beginDrawing(g);
	register float x = xA;
	float y = yA;
	for (unsigned int i = 0; i < nChar; ++i) {

		CharStruct cStruct = m_data->m_chars[uint8_t(text[i])];

		float texx = (uint8_t(text[i]) % 16) * 16.f;
		float texy = (uint8_t(text[i]) / 16) * 16.f;
		float left = x - cStruct.lbearing;// * m_multiplier;

		g->drawSubImage(m_data->m_image,
			int(texx), int(texy), 16, 16,
			int(left), int(y), 16, 16);//m_fontInfo.pointSize, m_fontInfo.pointSize);

		x += cStruct.width;// * m_multiplier;
	}
	// FIXME
	return int(x - xA + 0.5f);
}

void
UTextureFontRenderer::beginDrawing(UGraphics * g) {
}

void
UTextureFontRenderer::endDrawing(UGraphics * g) {
}

const UFontMetrics *
UTextureFontRenderer::getFontMetrics() const {
	return m_fontMetrics;
}

UFontInfo
UTextureFontRenderer::getFontInfo() const {
	return m_fontInfo;
}

std::string
UTextureFontRenderer::getSystemName() const {
	return m_systemName;
}

void
UTextureFontRenderer::refresh() {
//	m_fontCache[m_systemName]->refresh();
	setDisplay(UDisplay::getDefault());
	//m_fontCache[m_systemName]->refresh();
	UImageIO * io = loadImageFile();
	io->reference();
	createTexture(io);
	io->unreference();
	m_isValid = true;
}


UImageIO *
UTextureFontRenderer::loadImageFile() {
	UImageIO * ioL = new UImageIO();

	UFontInfo queryInfo = queryFont(m_fontInfo);

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
			<< "Sorry, couldn't find any font files for UTextureFontRenderer.\n"
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

#include "ufo/gl/ugl_image.hpp"
void
UTextureFontRenderer::createTexture(UImageIO * io) {
	io->reference();

	//UToolkit::getToolkit()->getCurrentContext()->createImage(io);
	m_data->m_image = UDisplay::getDefault()->createImage(io);

	if (UGL_Image * im = dynamic_cast<UGL_Image*>(m_data->m_image)) {
		im->setQuality(1);
	}

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
UTextureFontRenderer::genGlyphMetrics(UImageIO * ioA) {
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

//
// public static methods
//

UFontInfo
UTextureFontRenderer::queryFont(const UFontInfo & fontInfo) {
	UFontInfo searchInfo = fontInfo;

	std::vector<UFontInfo> infos;
	infos = listFonts(searchInfo);

	if (infos.size() == 0) {
		searchInfo.style = UFontInfo::AnyStyle;
		infos = listFonts(searchInfo);
	}
	if (infos.size() == 0) {
		searchInfo.weight = UFontInfo::AnyWeight;
		infos = listFonts(searchInfo);
	}
	if (infos.size() == 0) {
		searchInfo.pointSize = 0;
		infos = listFonts(searchInfo);
	}
	if (infos.size() == 0) {
		// FIXME if no font is available throw warning, exception?
		//std::cerr << "couldn't find any matching font file\n";
		return UFontInfo();
	}

	return infos[0];
}

std::vector<UFontInfo>
UTextureFontRenderer::listFonts(const UFontInfo & fontInfo) {
	std::vector<UFontInfo> ret;

	std::vector<std::string> files =
		UFileArchive::readDir(UToolkit::getToolkit()->getFontDir());

	for (std::vector<std::string>::const_iterator iter = files.begin();
			iter != files.end();
			++iter) {
		bool add = true;
		std::string::size_type tga_index = (*iter).find(".tga");
		if (tga_index != std::string::npos) {
			std::string::size_type index = 0;
			UFontInfo info("", 14, UFontInfo::Normal);

			// check font face
			info.face = (*iter).substr(0, (*iter).find("_"));
			if ((fontInfo.face != info.face) &&
					fontInfo.face != "") {
				add = false;
			}
			while ((index != std::string::npos) && add) {
				index = (*iter).find("_", index + 1);
				if ((*iter)[index + 1] == 'B') {
					// weight
					info.weight = UFontInfo::Bold;
					if ((fontInfo.weight != UFontInfo::Bold) &&
							fontInfo.weight != UFontInfo::AnyWeight) {
						add = false;
					}
				} else if ((*iter)[index + 1] == 'I') {
					// style
					info.style &= UFontInfo::Italic;
					if ((fontInfo.style & UFontInfo::Italic) == 0 &&
							fontInfo.style != UFontInfo::AnyStyle) {
						add = false;
					}
				} else if ((*iter)[index + 1] >= '0' && (*iter)[index + 1] <= '9') {
					// point size
					int ptSize = (*iter)[index + 1] - '0';
					if ((*iter)[index + 2] >= '0' && (*iter)[index + 1] <= '9') {
						ptSize *= 10;
						ptSize += (*iter)[index + 2] - '0';
					}
					info.pointSize = ptSize;
					if ((fontInfo.pointSize != ptSize) &&
							fontInfo.pointSize != 0) {
						add = false;
					}
				}
			}
			if (add) {
				ret.push_back(info);
			}
		}
	}

	return ret;
}

std::vector<UFontInfo>
UTextureFontRenderer::listFonts() {
	std::vector<UFontInfo> ret;

	std::vector<std::string> files =
		UFileArchive::readDir(UToolkit::getToolkit()->getFontDir());

	for (std::vector<std::string>::const_iterator iter = files.begin();
			iter != files.end();
			++iter) {
		std::string::size_type tga_index = (*iter).find(".tga");
		if (tga_index != std::string::npos) {
			std::string::size_type index = 0;
			UFontInfo info("", 14, UFontInfo::Normal);
			info.face = (*iter).substr(0, (*iter).find("_"));
			while (index != std::string::npos) {
				index = (*iter).find("_", index + 1);
				if ((*iter)[index + 1] == 'B') {
					info.weight = UFontInfo::Bold;
				} else if ((*iter)[index + 1] == 'I') {
					info.style &= UFontInfo::Italic;
				} else if ((*iter)[index + 1] >= 48 && (*iter)[index + 1] < 58){
					// read point size
					int ptSize = (*iter)[index + 1] - 48;
					if ((*iter)[index + 2] >= 48 && (*iter)[index + 1] < 58) {
						ptSize *= 10;
						ptSize += (*iter)[index + 2] - 48;
					}
					info.pointSize = ptSize;
				}
			}
			ret.push_back(info);
		}
	}

	return ret;
}

//
// texture font metrics
//

int
UTextureFontMetrics::getAscent() const {
	return int(m_renderer->m_data->m_maxBounds.ascent /** m_renderer->m_multiplier*/ + 0.5f);
}

int
UTextureFontMetrics::getDescent() const {
	return int(m_renderer->m_data->m_maxBounds.descent /** m_renderer->m_multiplier*/ + 0.5f);
}

//int
//UTextureFontMetrics::getHeight() const { return m_height + getLineskip(); }

int
UTextureFontMetrics::getMaxAscent() const {
	return int(m_renderer->m_data->m_maxBounds.ascent /** m_renderer->m_multiplier */+ 0.5f);
}
int
UTextureFontMetrics::getMaxDescent() const {
	return int(m_renderer->m_data->m_maxBounds.descent /** m_renderer->m_multiplier*/ + 0.5f);
}
int
UTextureFontMetrics::getMaxCharWidth() const {
	return int(m_renderer->m_data->m_maxBounds.width /** m_renderer->m_multiplier*/ + 0.5f);
}

int
UTextureFontMetrics::getStringWidth(const char * text, unsigned int nChar) const {
	float ret = 0;
	for (unsigned int i = 0; i < nChar; ++i) {
		if (!iscntrl(text[i]))
		ret += m_renderer->m_data->m_chars[uint8_t(text[i])].width;//+ * m_renderer->m_multiplier;
	}
	return int(ret + 0.5f);
}

int
UTextureFontMetrics::getCharWidth(const wchar_t chA) const {
	unsigned char ch = chA;
	float ret = m_renderer->m_data->m_chars[ch].width;// * m_renderer->m_multiplier;
	return int(ret + 0.5f);
}

unsigned int
UTextureFontMetrics::viewToModel(const char * text, unsigned int nChar,
		unsigned int w) const {
	float advance = 0.0f;
	unsigned int index = 0;

	for (;index < nChar; ++index) {
		advance += m_renderer->m_data->m_chars[uint8_t(text[index])].width;// *
//				m_renderer->m_multiplier;

		if (advance > w) {
			if (index > 0) {
				index--;
			}
			break;
		}
	}
	return index;
}

class UTextureFontPlugin : public UFontPlugin {
	UFO_DECLARE_DYNAMIC_CLASS(UTextureFontPlugin)
public:
	virtual UFontRenderer * createFontRenderer(const UFontInfo & fontInfo) {
		UTextureFontRenderer * ret = new UTextureFontRenderer(fontInfo);

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
UFO_IMPLEMENT_DYNAMIC_CLASS(UTextureFontPlugin, UFontPlugin)


UPluginBase *
UTextureFontRenderer::createPlugin() {
	return new UTextureFontPlugin();
}

void
UTextureFontRenderer::destroyPlugin(UPluginBase * plugin) {
	if (dynamic_cast<UTextureFontPlugin*>(plugin)) {
		delete (plugin);
	}
}


/*
void
UTextureFont::refresh() {
	UTextureFontCache * cache = m_fontCache[m_fontName];
	if (! cache) {
		std::cerr << "couldn't find texture cache for texture font " << this
		<< "  no refreshing possible\n";
	} else {
		cache->refresh();
	}
}

///
/// implements UFont
///

int
UTextureFont::drawString(const std::string & textA, int xA, int yA) const {
	if (textA.empty()) {
		return 0;
	}

	std::string::const_iterator iter = textA.begin();
	std::string::const_iterator end = textA.end();

	UPoint pos(xA, yA);

	UGlyph * start = getGlyph(*iter);
	start->startDrawing();

	// draw the first character
	pos->x += glyph->draw(pos);
	iter++;

	for (;iter != end;iter++) {
		UGlyph * glyph = getGlyph(*iter);
		// add the kern advance between this and the last character
		pos->x += getKernAdvance(*(iter-1), *iter);

		pos->x += glyph->draw(pos);
}

	// kern advance is always the same

	for (;iter != end;iter++) {
		UGlyph * glyph = getGlyph(*iter);
		pos.x += glyph->draw(pos);
		pos.x += m_kernAdvance;
	}
	start->endDrawing();

	return pos.x - xA;
}

#ifdef UFO_USE_WSTRING
int
UTextureFont::drawString(const std::wstring & textA, int xA, int yA) const {
	// the texture font supports only ASCII
	// TODO convert wstring to std::string and use
	// drawString(const std::string & textA, int xA, int yA)

	if (textA.empty()) {
		return 0;
	}

	std::wstring::const_iterator iter = textA.begin();
	std::wstring::const_iterator end = textA.end();

	UPoint pos(xA, yA);

	UGlyph * start = getGlyph(*iter);
	start->startDrawing();

	for (;iter != end;iter++) {
		UGlyph * glyph = getGlyph(*iter);
		pos.x += glyph->draw(pos);
		pos.x += m_kernAdvance;
	}
	start->endDrawing();

	return pos.x - xA;
}
#endif // UFO_USE_WSTRING

std::vector<std::string>
UTextureFont::getFileExtensions() const {
	return UImageIO::getAvailableLoadingExtensions();
}


//UTextureFont::UTextureGlyph *
UGlyph *
UTextureFont::getGlyph(wchar_t chA) const {
	//return m_glyphContainer->getGlyph(chA);

	unsigned char ch = 0xff & chA;
	return m_glyphList[ch];
}


int
UTextureFont::getKernAdvance(wchar_t left, wchar_t right) const {
	return m_kernAdvance;
}

int
UTextureFont::getKernAdvance(UGlyph * left, UGlyph * right) const {
	return m_kernAdvance;
}

UFont *
UTextureFont::deriveFont(int styleA, int ptSizeA) const {
	//if (styleA != m_style && ptSizeA != m_ptSize) {
	return new UTextureFont(m_family, UFont::Bold, ptSizeA, styleA);
	//}
	//return this;
}


///
/// protected methods
///


UTextureFont::UTextureGlyph **
UTextureFont::makeGlyphList(UTextureFont::UTextureFontCache * cache) {
#ifdef DEBUG
	std::cerr << "make glyph list for file " << std::endl;
#endif
	// this glyph list is returned
	typedef UTextureGlyph * UTextureGlyphPtr;
	UTextureGlyphPtr * glyphListL = new UTextureGlyphPtr[256];

	// pre-calculate coordinates
	GLfloat incX = 1 / 16.f;
	GLfloat incY = 1 / 16.f;
	GLfloat fontPoints[256][2];

	int i = 0;

	for (GLfloat y = 0; y <= 1 - incY; y += incY) {
		for (GLfloat x = 0; x <= 1 - incX; x += incX, i++) {
			fontPoints[i][0] = x;
			fontPoints[i][1] = y;
		}
	}

	int widthL;
	int lshiftL;

	for (i = 0;i < 256; i++) {
		cache->getGlyphMetrics(i, &widthL, &lshiftL);

		// correct width and left shift to current font size
		widthL = (widthL * m_ptSize) / 16;
		lshiftL = (lshiftL * m_ptSize) / 16;

		glyphListL[i] = new UTextureGlyph(
			cache,
			i,  // character
			fontPoints[i][0],
			fontPoints[i][1],
			widthL,
			lshiftL,
			m_ptSize
		);
	}

	return glyphListL;
}
*/

///
/// private classes
///


///
/// UTextureFont::UTextureFontMetrics
///

/*
int
UTextureFont::UTextureFontMetrics::getStringWidth(const std::string & textA)
const {
	return getStringWidth(textA, 0, textA.size());
}

#ifdef UFO_USE_WSTRING
int
UTextureFont::UTextureFontMetrics::getStringWidth(const std::wstring & textA)
const {
	return getStringWidth(textA, 0, textA.size());
}
#endif // UFO_USE_WSTRING


int
UTextureFont::UTextureFontMetrics::getStringWidth(const std::string & textA,
	unsigned int offsetA, unsigned int lengthA) const {

	if (textA.empty() || lengthA == 0) {
		return 0;
	}

	std::string::const_iterator iter = textA.begin() + offsetA;
	std::string::const_iterator end = textA.end();
	std::string::const_iterator len = textA.begin() + offsetA + lengthA;

	int advance = 0;

	for (; iter < len && iter != end; iter++) {
		advance += m_font->getGlyph(*iter)->getAdvance();
		advance += m_kernAdvance;
	}
	// do not leave space at the end

	// FIXME:
	// hmm, width seems to be too short(?)
	//advance -=m_kernAdvance;

	return advance;
}

#ifdef UFO_USE_WSTRING
int
UTextureFont::UTextureFontMetrics::getStringWidth(const std::wstring & textA,
	unsigned int offsetA, unsigned int lengthA) const {
	// the texture font supports only ASCII
	// TODO convert wstring to std::string and use
	// getStringWidth(const std::string textA)

	if (textA.empty() || lengthA == 0) {
		return 0;
	}

	std::wstring::const_iterator iter = textA.begin() + offsetA;
	std::wstring::const_iterator end = textA.end();
	std::wstring::const_iterator len = textA.begin() + offsetA + lengthA;

	int advance = 0;

	for (; iter < len && iter != end; iter++) {
		advance += m_font->getGlyph(*iter)->getAdvance();
		advance += m_kernAdvance;
	}
	// do not leave space at the end

	// FIXME:
	// hmm, width seems to be too short(?)
	//advance -=m_kernAdvance;

	return advance;
}
#endif // UFO_USE_WSTRING

int
UTextureFont::UTextureFontMetrics::getCharWidth(const wchar_t chA) const {
	return m_font->getGlyph(chA)->getAdvance();
}


unsigned int
UTextureFont::UTextureFontMetrics::getIndex(
		const char * text, unsigned int nChar, unsigned int w) const {
	unsigned int advance = 0;
	unsigned int index = 0;

	for (;index < nChar; ++index) {
		advance += m_font->getGlyph(text[index])->getAdvance();
		advance += m_kernAdvance;

		if (advance > w) {
			iter--;
			break;
		}
	}
	if (iter == end) {
		return textA.size() - 1;
	} else {
		return iter - textA.begin();
	}
}

*/

///
/// UTextureFont::UTextureGlyph
///
/*

//int UTextureFont::UTextureGlyph::m_ptSize = 16;

UTextureFont::UTextureGlyph::
UTextureGlyph(UTextureFontCache * cacheA, wchar_t chA,
		float texxA,
		float texyA,
		int widthA, int lshiftA,
		int pointSizeA)
	: m_cache(cacheA)
	, m_ch(chA)
	, texx(texxA)
	, texy(texyA)
	, m_width(widthA)
	, m_lshift(lshiftA)
	, m_ptSize(pointSizeA)
{
}

wchar_t
UTextureFont::UTextureGlyph::getCharacter() const {
	return m_ch;
}

int
UTextureFont::UTextureGlyph::draw(const UPoint & pos) {
	glTexCoord2f(texx, texy);
	glVertex2f(pos.x - m_lshift, pos.y);

	glTexCoord2f(texx, texy + 1/16.f);
	glVertex2f(pos.x - m_lshift, pos.y + m_ptSize);

	glTexCoord2f(texx + 1/16.f, texy + 1/16.f);
	glVertex2f(pos.x - m_lshift + m_ptSize, pos.y + m_ptSize);

	glTexCoord2f(texx + 1/16.f, texy);
	glVertex2f(pos.x - m_lshift + m_ptSize, pos.y);

	return m_width;
}
void
UTextureFont::UTextureGlyph::startDrawing() {
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //

	glBindTexture(GL_TEXTURE_2D, m_cache->getTexture()->getIndex());

	glBegin(GL_QUADS);
}
void
UTextureFont::UTextureGlyph::endDrawing() {
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}
int
UTextureFont::UTextureGlyph::getAdvance() const {
	return m_width;
}
*/


///
/// UTextureFont::UTextureFontCache
///


/*
UTextureFont::UTextureFontCache::UTextureFontCache(UTexture * texA,
			int widthA[256], int lshiftA[256],
			int spaceTopA, int spaceBottomA) :
m_tex(texA),
m_width(widthA),
m_lshift(lshiftA),
m_spaceTop(spaceTopA),
m_spaceBottom(spaceBottomA) {
}
*/
/*
UTextureFont::UTextureFontCache::
UTextureFontCache(const std::string & fileNameA)
	: m_fileName(fileNameA)
{
	// we are using pnm files for texture loading
	m_fileName.append(".pnm");

	UImageIO * ioL = loadImageFile();

	genGlyphMetrics(ioL);

	m_tex = new UTexture(ioL, GL_LUMINANCE_ALPHA);

	// dispose data
	delete (ioL);
}

UTexture *
UTextureFont::UTextureFontCache::getTexture() const {
	return m_tex;
}

void
UTextureFont::UTextureFontCache::getGlyphMetrics(uint8_t ch, int * width, int * lshift) const {
	if (width) {
		*width = m_width[ch];
	}
	if (lshift) {
		*lshift = m_lshift[ch];
	}
}

int
UTextureFont::UTextureFontCache::getSpaceTop() const {
	return m_spaceTop;
}
int
UTextureFont::UTextureFontCache::getSpaceBottom() const {
	return m_spaceBottom;
}

void
UTextureFont::UTextureFontCache::refresh() {
	dispose();

	UImageIO * ioL = loadImageFile();

	m_tex = new UTexture(ioL, GL_LUMINANCE_ALPHA);

	// dispose data
	delete (ioL);
}


void
UTextureFont::UTextureFontCache::dispose() {
	if (m_tex) {
		delete (m_tex);
	}
}


UImageIO *
UTextureFont::UTextureFontCache::loadImageFile() {
	UImageIO * ioL = new UImageIO();

	ioL->setStates(UImageIO::COLOR_TYPE_GRAY_ALPHA);

	// load from font directory
	if (ioL->load(UToolkit::getToolkit()->getFontDir() + "/" + m_fileName)) {
		ioL->apply();
		return ioL;
	} else {
		// try loading from file archive.
		// no error checking here
		ioL->loadFromArchive(m_fileName);
		ioL->apply();
		return ioL;
	}

	//textureL = new UTexture(ioL, GL_LUMINANCE, GL_INTENSITY);
	//return new UTexture(ioL, GL_LUMINANCE_ALPHA);
}


static void
correctGlyph(UImageIO * ioA, int indexA, int * widthA, int * lshiftA) {
	// both, width and height have to be 256
	int x_index = (indexA % 16) * 16;
	int y_index = (indexA / 16) * 16;

	unsigned char * pixels = ioA->getPixels();
	bool lineEnd = false;

	int leftMost = 16;
	int rightMost = 0;

	// don´t forget, we have two bytes per pixel (luminance + alpha)
	for (int y = y_index; y < y_index + 16; y++) {
		lineEnd = false;
		for (int x = x_index; x < x_index + 16; x++) {
			if (!lineEnd && pixels[y * 256 * 2 + x * 2] != 0) {
				leftMost = std::min(leftMost, x - x_index);
				lineEnd = true;
			}
			if (lineEnd && pixels[y * 256 * 2 + x * 2] == 0) {
				rightMost = std::max(rightMost, x - x_index);
				break;
			}
		}
	}

	if (leftMost == 16) {
		// white space
		leftMost = 0;
		rightMost = 6;
	}
	if (rightMost == 0) {
		rightMost = 16;
	}
	*lshiftA = leftMost;
	*widthA = rightMost - leftMost;
}

void
UTextureFont::UTextureFontCache::genGlyphMetrics(UImageIO * ioA) {
	//if (m_fixedSize == false) {
		for (int i=0;i<256;i++) {
			correctGlyph(ioA, i, &m_width[i], &m_lshift[i]);
		}
	*//*
	} else {
		for (int i=0;i<256;i++) {
			m_width[i] = 16;
			m_lshift[i] = 0;
		}
	}
	*//*

	// TODO
	// compute the empty space above and below the glyphs

	m_spaceTop = 0;
	m_spaceBottom = 4;
}
*/
