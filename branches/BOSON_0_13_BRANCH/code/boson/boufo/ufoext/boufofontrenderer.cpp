/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!

#include "ufo/font/ufontmetrics.hpp"
#include "ufo/uplugin.hpp"
#include "ufo/utoolkit.hpp"
#include "ufo/util/ufilearchive.hpp"

#include "boufofontrenderer.h"
#include "../bosonfont/bosonglfont.h"

#include <bodebug.h>

#include <kglobal.h>
#include <kstddirs.h>

class BoUfoFontMetrics : public ufo::UFontMetrics
{
	UFO_DECLARE_ABSTRACT_CLASS(BoUfoFontMetrics)
public:
	BoUfoFontMetrics(BoUfoFontRenderer* font)
		: ufo::UFontMetrics()
	{
		mRenderer = font;
	}

	ufo::UFontRenderer* getFontRenderer() const  { return mRenderer; }
	int getAscent() const;
	int getDescent() const;
	int getLineskip() const { return getHeight() + 2; }
	int getHeight() const { return getAscent() + getDescent(); }

	int getMaxAscent() const;
	int getMaxDescent() const;
	int getMaxCharWidth() const;

	int getUnderlinePosition() const { return 1; }
	int getUnderlineThickness() const { return 1; }


	int getStringWidth(const char* text, unsigned int nChar) const;

	int getCharWidth(const wchar_t chA) const;

	unsigned int viewToModel(const char* text, unsigned int nChar,
		unsigned int w) const;
private:
	BoUfoFontRenderer* mRenderer;
};
UFO_IMPLEMENT_ABSTRACT_CLASS(BoUfoFontMetrics, ufo::UFontMetrics)


int BoUfoFontMetrics::getAscent() const
{
 // FIXME: ?
 return mRenderer->mFont->height();
}

int BoUfoFontMetrics::getDescent() const
{
 // FIXME: ?
 return 0;
}

int BoUfoFontMetrics::getMaxAscent() const
{
 // FIXME: ?
 return mRenderer->mFont->height();
}

int BoUfoFontMetrics::getMaxDescent() const
{
 return 0;
}

int BoUfoFontMetrics::getMaxCharWidth() const
{
 return mRenderer->mFont->widestChar();
}

int BoUfoFontMetrics::getStringWidth(const char* text, unsigned int length) const
{
 return mRenderer->mFont->lineWidth((const GLubyte*)text, length);
}

int BoUfoFontMetrics::getCharWidth(const wchar_t c) const
{
 return mRenderer->mFont->lineWidth((const GLubyte*)&c, 1);
}

unsigned int BoUfoFontMetrics::viewToModel(const char* text, unsigned int length, unsigned int w) const
{
 // FIXME: this is very inefficient
 unsigned int advance = 0;
 unsigned int index = 0;

 for (; index < length; index++) {

	advance += getCharWidth(text[index]);
	if (advance > w) {
		if (index > 0) {
			index--;
		}
		break;
	}
 }
 return index;
}



UFO_IMPLEMENT_ABSTRACT_CLASS(BoUfoFontRenderer, UFontRenderer)

BoUfoFontRenderer::BoUfoFontRenderer(BosonGLFont* font)
	: ufo::UFontRenderer()
{
 mFont = font;
 mFontMetrics = new BoUfoFontMetrics(this);
}

BoUfoFontRenderer::~BoUfoFontRenderer()
{
 delete mFont;
 delete mFontMetrics;
}

int BoUfoFontRenderer::drawString(ufo::UGraphics * g, const char * text, unsigned int length, int x, int y)
{
 beginDrawing(g);

#if 0
 // AB: renderStringInternal() bypasses all automatic line wrapping of
 // BosonGLFont, which we don't want here.
 // in case of GLX (i.e. bitmap) fonts, the x/y coordinates are forwarded
 // directly to glRasterPos2i().
 // AB: this is necessary for GLX fonts only it seems
// mFont->renderStringInternal(x, y + mFont->height(), (const GLubyte*)text, nChar);
#else
 // AB: for TXF fonts (which we use exclusively here atm) x/y seem to be correct
 mFont->renderStringInternal(x, y, (const GLubyte*)text, length);
#endif
 endDrawing(g);

 return mFont->lineWidth((const GLubyte*)text, length);
}

void BoUfoFontRenderer::beginDrawing(ufo::UGraphics * )
{
 glPushAttrib(GL_ENABLE_BIT);
 glEnable(GL_TEXTURE_2D);

 mFont->begin();
}

void BoUfoFontRenderer::endDrawing(ufo::UGraphics * )
{
 glPopAttrib();
}

void BoUfoFontRenderer::refresh()
{
}

const ufo::UFontMetrics * BoUfoFontRenderer::getFontMetrics() const
{
 return mFontMetrics;
}

ufo::UFontInfo BoUfoFontRenderer::getFontInfo() const
{
 // uh, dirty conversion
 std::string face;
 if (!mFont->fontInfo().name().isEmpty()) {
	face = mFont->fontInfo().name().latin1();
 }
 ufo::UFontInfo ret(
	ufo::UFontInfo::DefaultFamily,
	face,
	(float)mFont->fontInfo().pointSize(),
	ufo::UFontInfo::Normal, // weight
	ufo::UFontInfo::Plain, // style
	ufo::UFontInfo::Encoding_Default // FIXME?
 );
 // set weight
 if (mFont->fontInfo().bold()) {
	ret.weight = ufo::UFontInfo::Bold;
 } else {
	ret.weight = ufo::UFontInfo::Normal;
 }
 // set style
 if (mFont->fontInfo().italic()) {
	ret.style |= ufo::UFontInfo::Italic;
 }
 if (mFont->fontInfo().underline()) {
	ret.style |= ufo::UFontInfo::Underline;
 }
 if (mFont->fontInfo().strikeOut()) {
	ret.style |= ufo::UFontInfo::StrikeOut;
 }
 return ret;
}

std::string BoUfoFontRenderer::getSystemName() const
{
 // FIXME
 // probably not necessary ...
 return "";
}


class BoUfoFontPlugin : public ufo::UFontPlugin
{
	UFO_DECLARE_DYNAMIC_CLASS(BoUfoFontPlugin)
public:
	// AB: this class ignores the fontInfo.family!
	// TODO: maybe we could use a fixed font if
	// fontInfo.family==ufo::UFontInfo::MonoSpaced
	virtual ufo::UFontRenderer* createFontRenderer(const ufo::UFontInfo& _fontInfo)
	{
		ufo::UFontInfo fontInfo(_fontInfo);
		if (fontInfo.face == "") {
			boDebug() << k_funcinfo << "empty font face requested. trying to find replacement" << endl;
			fontInfo = queryFont(fontInfo);
			if (fontInfo.face == "") {
				boDebug() << k_funcinfo << "no replacement found. let's hope the best..." << endl;
			} else {
				boDebug() << k_funcinfo << "replacement face: " << fontInfo.face.c_str() << endl;
			}
		}
		BoFontInfo info;

		// AB: we support textured fonts only currently due to 2 main
		// reasons:
		// 1. they look a LOT better
		// 2. on some systems (e.g. mine) QFont::handle() returns 0 no
		//    matter what we do, however such fonts cannot be loaded
		//    using glXUseXFont.
		//    so as a matter of consistency we support textured fonts
		//    only which will be available on all systems.
		//    note that if no textured font can be loaded, the code will
		//    fallback to GLX anyway!
		info.setTextured(true);

		info.setPointSize((int)fontInfo.pointSize);
		info.setName(fontInfo.face.c_str());
		if (fontInfo.style & ufo::UFontInfo::Italic) {
			info.setItalic(true);
		}
		if (fontInfo.weight > ufo::UFontInfo::Normal) {
			info.setBold(true);
		}

		BosonGLFont* font = new BosonGLFont(info);
		font->loadFont(info);

		// AB: the renderer takes ownership of the BosonGLFont
		BoUfoFontRenderer* r = new BoUfoFontRenderer(font);
		return r;
	}

	virtual ufo::UFontInfo queryFont(const ufo::UFontInfo& _fontInfo)
	{
		ufo::UFontInfo fontInfo(_fontInfo);

		// AB: we support the textured font of BosonGLFont only.
		//     these fonts are configurable concerning style settings
		//     (however only italic supported atm) and pointSize, so if
		//     we find a font, we always get the correct size/style.

		// AB: weight is not supported anyway here.
		fontInfo.weight = ufo::UFontInfo::AnyWeight;

		std::vector<ufo::UFontInfo> fonts = listFonts(fontInfo);
		if (fonts.size() == 0) {
			// AB: what are the semantics of queryFont() ? is it
			// supposed to _always_ return a valid font?
			// we treat it like that here - if no font with the
			// desired face is found, we use the first face we can
			// find
			fonts = listFonts();
		}

		if (fonts.size() > 0) {
			if (fontInfo.face == "") {
				// use a hardcoded default font, if possible.
				// otherwise we will just use the first
				// available font
				for (std::vector<ufo::UFontInfo>::iterator it = fonts.begin(); it != fonts.end(); ++it) {
					if ((*it).face.find("Courier") != std::string::npos) {
						return *it;
					}
				}
			}
			return fonts[0];
		}
		return ufo::UFontInfo();
	}
	virtual std::vector<ufo::UFontInfo> listFonts(const ufo::UFontInfo & fontInfo)
	{
		std::vector<ufo::UFontInfo> allFonts = listFonts();
		std::vector<ufo::UFontInfo> ret;
		for (std::vector<ufo::UFontInfo>::iterator it = allFonts.begin(); it != allFonts.end(); ++it) {
			if (fontInfo.face != "" && (*it).face != fontInfo.face) {
				continue;
			}
			if ((*it).pointSize != fontInfo.pointSize) {
				continue;
			}
			if (fontInfo.style != ufo::UFontInfo::AnyStyle) {
				if ((*it).style != ufo::UFontInfo::AnyStyle) {
					if ((*it).style != fontInfo.style) {
						continue;
					}
				}
			}
			if (fontInfo.weight != ufo::UFontInfo::AnyWeight) {
				if ((*it).weight != ufo::UFontInfo::AnyWeight) {
					if ((*it).weight >= ufo::UFontInfo::Bold && fontInfo.weight < ufo::UFontInfo::Bold) {
						continue;
					} else if ((*it).weight < ufo::UFontInfo::Bold && fontInfo.weight >= ufo::UFontInfo::Bold) {
						continue;
					}
				}
			}
			ret.push_back(*it);
		}
		return ret;
	}
	virtual std::vector<ufo::UFontInfo> listFonts();

};
UFO_IMPLEMENT_DYNAMIC_CLASS(BoUfoFontPlugin, ufo::UFontPlugin)

std::vector<ufo::UFontInfo> BoUfoFontPlugin::listFonts()
{
 std::vector<ufo::UFontInfo> ret;
 QStringList txfFonts;

 // the default location. this can be both normal system dir as
 // well as ~/.kde
 if (KGlobal::_instance) {
	txfFonts = KGlobal::dirs()->findAllResources("data", "boson/fonts/*.txf");
 }
 // the fallback dir, set by UFO_FONT_DIR
 if (ufo::UToolkit::getToolkit()->getFontDir() != "") {
	// (code shamelessy stolen from
	// ufo::UTextureFont::listFonts(), to keep KDE/Qt
	// classes at a mimimum here)
	std::string path = ufo::UToolkit::getToolkit()->getFontDir();
	if (path.length() > 0 && path[path.length() - 1] != '/') {
		path += "/";
	}
	std::vector<std::string> files = ufo::UFileArchive::readDir(path);
	for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it) {
		if ((*it).find(".txf") != std::string::npos) {
			txfFonts.append((path + (*it)).c_str());
		}
	}
 }

 for (QStringList::iterator it = txfFonts.begin(); it != txfFonts.end(); ++it) {
	std::string face((*it).latin1());
	ret.push_back(ufo::UFontInfo(
			face,
			4.0f,
			ufo::UFontInfo::Normal,
			ufo::UFontInfo::Plain
	));
 }
 std::vector<ufo::UFontInfo> tmp;
 tmp = ret;
 ret.clear();

 // AB: actually any pointsize is possible.
 // 4 to 20 is hardcoded default. feel free to change if
 // required.
 for (std::vector<ufo::UFontInfo>::iterator it = tmp.begin(); it != tmp.end(); ++it) {
	// AB: we use 2 loops so that size=12 is at the beginning and will be
	// our fallback font
	for (float pointSize = 12.0f; pointSize < 21.0f; pointSize += 1.0f) {
		ret.push_back(ufo::UFontInfo(
				(*it).face,
				pointSize,
				(*it).weight,
				(*it).style
		));
	}
	for (float pointSize = 4.0f; pointSize < 12.0f; pointSize += 1.0f) {
		ret.push_back(ufo::UFontInfo(
				(*it).face,
				pointSize,
				(*it).weight,
				(*it).style
		));
	}
 }

 tmp = ret;
 ret.clear();
 for (std::vector<ufo::UFontInfo>::iterator it = tmp.begin(); it != tmp.end(); ++it) {
	ret.push_back(ufo::UFontInfo(
			(*it).face,
			(*it).pointSize,
			(*it).weight,
			ufo::UFontInfo::Plain
		));
	ret.push_back(ufo::UFontInfo(
			(*it).face,
			(*it).pointSize,
			(*it).weight,
			ufo::UFontInfo::Italic
		));
 }

 return ret;
}

ufo::UPluginBase* BoUfoFontRenderer::createPlugin()
{
 return new BoUfoFontPlugin();
}

void BoUfoFontRenderer::destroyPlugin(ufo::UPluginBase * plugin)
{
 if (dynamic_cast<BoUfoFontPlugin*>(plugin)) {
	delete plugin;
 }
}

