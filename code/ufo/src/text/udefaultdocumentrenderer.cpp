/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/text/udefaultdocumentrenderer.cpp
    begin             : Wed Sep 5 2001
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

#include "ufo/text/udefaultdocumentrenderer.hpp"

#include <algorithm>

#include "ufo/util/ugeom.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontrenderer.hpp"
#include "ufo/text/udocument.hpp"


using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UDefaultDocumentRenderer, UDocumentRenderer)
UFO_IMPLEMENT_ABSTRACT_CLASS(UDocumentRenderer, UObject)

UDefaultDocumentRenderer::UDefaultDocumentRenderer()
	: m_tabSize(4) {
}

UDefaultDocumentRenderer::~UDefaultDocumentRenderer() {}


URectangle
UDefaultDocumentRenderer::modelToView(const UDocument * docA, int offsA,
		const UFont * fontA) const {
	const char * text = docA->getText();
	int length = docA->getLength();

	const UFontMetrics * metrics = fontA->getFontMetrics();
	int fontHeight = metrics->getHeight();
	
	int tabMultiplier = metrics->getMaxCharWidth();//'m');
	
	int lineStart = 0;
	int column = 0;
	int lineY = 0; // line y
	int addWidth = 0; // for tabs
	
	for (int i = 0; i < offsA && i < length; ++i) {
		if (text[i] == '\n') {
			lineY += fontHeight;
			
			lineStart = i + 1;
			column = 0;
			addWidth = 0;
		} else if (text[i] == '\t') {
			addWidth += m_tabSize * tabMultiplier;
			// a good font renderer should omit control characters
			// but we can't rely on that.
			// add string width up to the tab and reset linestart and column.
			addWidth += metrics->getStringWidth(text + lineStart, column);
			lineStart = i + 1;
			column = 0;
		} else {
			column++;
		}
	}
	
	return URectangle(
		metrics->getStringWidth(text + lineStart, column) + addWidth,
		lineY,
		metrics->getCharWidth(text[lineStart + column]),
		fontHeight
	);
}

int
UDefaultDocumentRenderer::viewToModel(const UDocument * docA,
		const UPoint & posA, const UFont * fontA) const {

	if (posA.y < 0 || posA.x < 0) {
		return 0;
	}
	
	int top = 0;
	int left = 0;

	const UFontMetrics * metrics = fontA->getFontMetrics();
	int fontHeight = metrics->getHeight();
	
	int tabMultiplier = metrics->getMaxCharWidth();

	const char * text = docA->getText();
	unsigned int length = docA->getLength();
	unsigned int index = 0;

	if (posA.y < fontHeight) {
		// first line
	} else {
		// or below
		for (; index < length; ++index) {
			if (text[index] == '\n') {
				top += fontHeight;

				if ((posA.y >= top) && (posA.y < top + fontHeight)) {
					// the iterator is still \n
					++index;
					break;
				}
			}
		}
	}
	/*
	if (index == length - 1) {
		return length;
	}
	*/
	for (; index < length; ++index) {
		if (text[index] == '\n') {
			break;
		} else if (text[index] == '\t') {
			int addWidth = m_tabSize * tabMultiplier;

			if ((posA.x >= left) && (posA.x <= left + addWidth)) {
				// if the cursor is nearer to the left letter, go to left
				if (posA.x >= left + addWidth / 2) {
					++index;
				}
				break; // break for loop
			}
			left += addWidth;
		} else {
			int addWidth = metrics->getCharWidth(text[index]);//fontA->getGlyph(*iter)->getAdvance();
			//if (iter != end - 1) {
			//	addWidth += 0;//fontA->getKernAdvance(*iter, *(iter + 1));
			//}

			if ((posA.x >= left) && (posA.x <= left + addWidth)) {
				// if the cursor is nearer to the right letter, go to right
				if (posA.x >= left + addWidth / 2) {
					++index;
				}
				break; // break for loop
			}
			left += addWidth;
		}
	}

	return index;
}

UDimension
UDefaultDocumentRenderer::getPreferredSize(const UDocument * docA,
		const UFont * fontA) const {
	const UFontMetrics * metrics = fontA->getFontMetrics();
	unsigned int fontHeight = metrics->getHeight();
	unsigned int height = 0;
	
	unsigned int maxWidth = 0;

	unsigned int tabMultiplier = metrics->getMaxCharWidth();

	// TODO
	// don´t create to much objects: use references,pointers,..
	const char * text = docA->getText();
	unsigned int length = docA->getLength();

	if (length == 0) {
		return UDimension();
	}
	
	unsigned int start = 0;

	unsigned int line_width = 0;

	// set an inintial height as first line
	height += fontHeight;
	
	for (unsigned int i = 0; i < length; ++i) {
		if (text[i] == '\n') {
			line_width += metrics->getStringWidth(text + start, i - start);

			maxWidth = std::max(maxWidth, line_width);
			height += fontHeight;

			line_width = 0;
			start = i + 1;
		} else if (text[i] == '\t') {
			line_width += metrics->getStringWidth(text + start, i - start);
			line_width += m_tabSize * tabMultiplier;

			start = i + 1;
		}
	}
	if (start < length) {
		line_width += metrics->getStringWidth(text + start, length - start);

		maxWidth = std::max(maxWidth, line_width);
	}
	return UDimension(maxWidth, height);
}


void
UDefaultDocumentRenderer::render(UGraphics * g, const UDocument * docA,
		const UDimension & sizeA, const UFont * fontA) {

	UPoint pos;
	int fontHeight = fontA->getFontMetrics()->getHeight();

	int tabMultiplier = fontA->getFontMetrics()->getMaxCharWidth();//fontA->getGlyph('m')->getAdvance();

	UFontRenderer * renderer = fontA->getRenderer();
	const char * text = docA->getText();
	unsigned int length = docA->getLength();

	unsigned int start = 0;
	
	for (unsigned int i = 0; i < length; ++i) {
		if (text[i] == '\n') {
			renderer->drawString(g, text + start, i - start, pos.x, pos.y);
			pos.y += fontHeight;
			pos.x = 0;
			start = i + 1;
		} else if (text[i] == '\t') {
			pos.x += renderer->drawString(g, text + start, i - start, pos.x, pos.y);
			pos.x += m_tabSize * tabMultiplier;

			start = i + 1;
		}
	}
	if (start < length) {
		renderer->drawString(g, text + start, length - start, pos.x, pos.y);
	}
}

void UDefaultDocumentRenderer::setTabSize(int tabSizeA) {
	m_tabSize = tabSizeA;
}
int UDefaultDocumentRenderer::getTabSize() {
	return m_tabSize;
}
