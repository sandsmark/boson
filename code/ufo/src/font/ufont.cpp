/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/font/ufont.cpp
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

#include "ufo/font/ufont.hpp"

#include "ufo/font/ufontrenderer.hpp"
#include "ufo/font/ufontmetrics.hpp"

#include "ufo/utoolkit.hpp"
#include "ufo/udisplay.hpp"

using namespace ufo;

// FIXME
// should be better in files fontmetrics.cpp,
// ufontrenderer.cpp and ufontinfo.cpp
// but this would increase compile time ...
UFO_IMPLEMENT_DYNAMIC_CLASS(UFont, UObject)
UFO_IMPLEMENT_ABSTRACT_CLASS(UFontMetrics, UObject)
UFO_IMPLEMENT_ABSTRACT_CLASS(UFontRenderer, UObject)


static UFontInfo s_defaultFontInfo(
	UFontInfo::DefaultFamily,
	"",
	12.0f,
	UFontInfo::Normal,
	UFontInfo::Plain,
	UFontInfo::Encoding_Default
);

// initialize static members
UFont::displayCache_t UFont::sm_fontCache;

UFont::UFont()
	: m_renderer(NULL)
	, m_infoCache(s_defaultFontInfo)
{
	//queryAndLoad(s_defaultFontInfo);
}

UFont::UFont(UFontInfo::Family family, float pointSize, int weight, int style,
		UFontInfo::Encoding encoding)
	: m_renderer(NULL)
	, m_infoCache(UFontInfo(family, "", pointSize, weight, style, encoding))
{
	// FIXME !
	//if (face == Default) {
	//	queryAndLoad(m_infoCache);
	//}
}

UFont::UFont(const std::string & face, float pointSize, int weight, int style,
		UFontInfo::Encoding encoding)
	: m_renderer(NULL)
	, m_infoCache(UFontInfo(face, pointSize, weight, style, encoding))
{
	//queryAndLoad(m_infoCache);
}

UFont::UFont(const UFontInfo & fontInfo)
	: m_renderer(NULL)
	, m_infoCache(fontInfo)
{
	//queryAndLoad(m_infoCache);
}

UFont::UFont(UFontRenderer * renderer)
	: m_renderer(renderer)
	, m_infoCache()
{
	if (m_renderer) {
		m_infoCache = m_renderer->getFontInfo();
		m_renderer->reference();
	}
}

UFont::UFont(const UFont & font)
	: m_renderer(NULL)
	, m_infoCache()
{
	m_renderer = font.m_renderer;
	m_infoCache = font.m_infoCache;

	if (m_renderer) {
		m_renderer->reference();
	}
}

UFont&
UFont::operator=(const UFont & font) {
	if (font.m_renderer) {
		font.m_renderer->reference();
	}
	dispose();
	m_renderer = font.m_renderer;
	m_infoCache = font.m_infoCache;
	return *this;
}

UFont::~UFont() {
	dispose();
}

UFontRenderer *
UFont::getRenderer() const {
	if (!m_renderer) {
		// for the sake of logical const correctness ...
		(const_cast<UFont*>(this))->queryAndLoad(m_infoCache);
	}
	return m_renderer;
}

const UFontMetrics *
UFont::getFontMetrics() const {
	if (getRenderer()) {
		return m_renderer->getFontMetrics();
	}
	return NULL;
}

//
// further attributes
//

void
UFont::setFamiliy(UFontInfo::Family family) {
	UFontInfo info = m_infoCache;//m_renderer->getFontInfo();
	info.family = family;
	queryAndLoad(info);
}

void
UFont::setFontFace(const std::string & face) {
	UFontInfo info = m_infoCache;//m_renderer->getFontInfo();
	info.face = face;
	queryAndLoad(info);
}

std::string
UFont::getFontFace() const {
	getRenderer();
	return m_infoCache.face;
}


void
UFont::setPointSize(float pointSize) {
	UFontInfo info = m_infoCache;//m_renderer->getFontInfo();
	info.pointSize = pointSize;
	queryAndLoad(info);
}

float
UFont::getPointSize() const {
	getRenderer();
	return m_infoCache.pointSize;
}

void
UFont::setWeight(int weight) {
	UFontInfo info = m_infoCache;//m_renderer->getFontInfo();
	info.weight = weight;
	queryAndLoad(info);
}

int
UFont::getWeight() const {
	getRenderer();
	return m_infoCache.weight;
}

void
UFont::setStyle(int flag) {
	UFontInfo info = m_infoCache;//m_renderer->getFontInfo();
	info.style = flag;
	queryAndLoad(info);
}

int
UFont::getStyle() const {
	getRenderer();
	return m_infoCache.style;
}


void
UFont::setEncoding(UFontInfo::Encoding encoding) {
	UFontInfo info = m_infoCache;//m_renderer->getFontInfo();
	info.encoding = encoding;
	queryAndLoad(info);
}

UFontInfo::Encoding
UFont::getEncoding() const {
	getRenderer();
	return m_infoCache.encoding;
}

// FIXME: why does the display selection code do not work?
static std::map<UFontInfo, UFontRenderer*> fontContextCache;

void
UFont::queryAndLoad(const UFontInfo & fontInfo) {
	UToolkit * tk = UToolkit::getToolkit();

	UFontInfo info = tk->queryFont(fontInfo);

	UDisplay * display = UDisplay::getDefault();
	// FIXME: critical
	// why does this line do not work?
	// we always get a new font cache ..
	//fontCache_t fontContextCache = sm_fontCache[display];
	if (fontContextCache[info]) {
		m_renderer = fontContextCache[info];
	} else {
		fontContextCache.erase(info);
		m_renderer = tk->createFontRenderer(info);
		fontContextCache[m_renderer->getFontInfo()] = m_renderer;
	}

	m_infoCache = m_renderer->getFontInfo();
	m_renderer->reference();
}

void
UFont::dispose() {
	// ref count == 1: we have to remove it from the cache
	if (m_renderer && m_renderer->getReferenceCount() == 1) {
		// FIXME: see above
		//fontCache_t fontContextCache = sm_fontCache[display];
		if (fontContextCache[m_renderer->getFontInfo()]) {
			fontContextCache.erase(m_renderer->getFontInfo());
		}
	}
	if (m_renderer) {
		m_renderer->unreference();
	}
	m_renderer = NULL;
}

void
UFont::clearCache() {
	//sm_fontCache.clear();
	fontContextCache.clear();
}
