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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/font/ufont.hpp"

#include "ufo/font/ufontrenderer.hpp"

#include "ufo/utoolkit.hpp"
#include "ufo/ucontextgroup.hpp"

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
UFont::contexGroupCache_t UFont::sm_fontCache;

UFont::UFont() : m_renderer(NULL) {
	queryAndLoad(s_defaultFontInfo);
}

UFont::UFont(UFontInfo::Family family, float pointSize, int weight, int style,
		UFontInfo::Encoding encoding) : m_renderer(NULL) {
	// FIXME !
	//if (face == Default) {
		queryAndLoad(UFontInfo(family, "", pointSize, weight, style, encoding));
	//}
}

UFont::UFont(const std::string & face, float pointSize, int weight, int style,
		UFontInfo::Encoding encoding) : m_renderer(NULL) {
	queryAndLoad(UFontInfo(face, pointSize, weight, style, encoding));
}

UFont::UFont(const UFontInfo & fontInfo) : m_renderer(NULL) {
	queryAndLoad(fontInfo);
}

UFont::UFont(UFontRenderer * renderer) : m_renderer(renderer) {
	m_renderer->reference();
}

UFont::~UFont() {
	m_renderer->unreference();
}

UFontRenderer *
UFont::getRenderer() const {
	return m_renderer;
}

const UFontMetrics *
UFont::getFontMetrics() const {
	return m_renderer->getFontMetrics();
}

//
// further attributes
//

void
UFont::setFamiliy(UFontInfo::Family family) {
	UFontInfo info = m_renderer->getFontInfo();
	info.family = family;
	UFontRenderer * oldRenderer = m_renderer;
	queryAndLoad(info);
	oldRenderer->unreference();
}

void
UFont::setFontFace(const std::string & face) {
	UFontInfo info = m_renderer->getFontInfo();
	info.face = face;
	UFontRenderer * oldRenderer = m_renderer;
	queryAndLoad(info);
	oldRenderer->unreference();
}

std::string
UFont::getFontFace() const {
	return m_infoCache.face;
}


void
UFont::setPointSize(float pointSize) {
	UFontInfo info = m_renderer->getFontInfo();
	info.pointSize = pointSize;
	UFontRenderer * oldRenderer = m_renderer;
	queryAndLoad(info);
	oldRenderer->unreference();
}

float
UFont::getPointSize() const {
	return m_infoCache.pointSize;
}

void
UFont::setWeight(int weight) {
	UFontInfo info = m_renderer->getFontInfo();
	info.weight = weight;
	UFontRenderer * oldRenderer = m_renderer;
	queryAndLoad(info);
	oldRenderer->unreference();
}

int
UFont::getWeight() const {
	return m_infoCache.weight;
}

void
UFont::setStyle(int flag) {
	UFontInfo info = m_renderer->getFontInfo();
	info.style = flag;
	UFontRenderer * oldRenderer = m_renderer;
	queryAndLoad(info);
	oldRenderer->unreference();
}

int
UFont::getStyle() const {
	return m_infoCache.style;
}


void
UFont::setEncoding(UFontInfo::Encoding encoding) {
	UFontInfo info = m_renderer->getFontInfo();
	info.encoding = encoding;
	UFontRenderer * oldRenderer = m_renderer;
	queryAndLoad(info);
	oldRenderer->unreference();
}

UFontInfo::Encoding
UFont::getEncoding() const {
	return m_infoCache.encoding;
}

void
UFont::queryAndLoad(const UFontInfo & fontInfo) {
	UToolkit * tk = UToolkit::getToolkit();

	UFontInfo info = tk->queryFont(fontInfo);

	UContextGroup * group = tk->getCurrentContext()->getContextGroup();
	fontCache_t fontContextCache = sm_fontCache[group];
	if (fontContextCache[info]) {
		m_renderer = fontContextCache[info];
	} else {
		m_renderer = tk->createFontRenderer(info);
		fontContextCache[info] = m_renderer;
		group->addVolatileData(m_renderer);
	}
	m_renderer->reference();

	m_infoCache = m_renderer->getFontInfo();
}

void
UFont::clearCache() {
	sm_fontCache.clear();
}
