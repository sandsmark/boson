/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/text/utextlayout.cpp
    begin             : Tue Mar 22 2005
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

#include "ufo/text/utextlayout.hpp"

#include "ufo/font/ufontmetrics.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UTextLayout, UObject)


UTextLayout::UTextLayout()
	: m_text(NULL)
	, m_length(0)
	, m_font()
	, m_lines()
	, m_maxSize(UDimension::maxDimension)
	, m_validLayout(false)
{}

void
UTextLayout::setFont(const UFont & font) {
//	if (font != m_font) {
		m_font = font;
		invalidate();
//	}
}

UFont
UTextLayout::getFont() const {
	return m_font;
}

void
UTextLayout::setText(const char * text, unsigned int length) {
	if (text != m_text || length != m_length) {
		m_text = text;
		m_length = length;
		invalidate();
	}
}

const char *
UTextLayout::getText() {
	return m_text;
}

unsigned int
UTextLayout::getLength() {
	return m_length;
}

void
UTextLayout::setMaximumSize(const UDimension & dim) {
	if (m_maxSize != dim) {
		m_maxSize = dim;
		invalidate();
	}
}

UDimension
UTextLayout::getPreferredSize(const UDimension & maxSize) {
	// FIXME: very expensive. We purge all cached text lines
	UDimension oldMaxSize = m_maxSize;
	if (maxSize.isValid() || m_maxSize.isValid()) {
		setMaximumSize(maxSize);
	}
	layout();
	UDimension ret;
	const UFontMetrics * metrics = m_font.getFontMetrics();
	int fontHeight = metrics->getHeight();
	for (std::vector<UTextLine>::const_iterator iter = m_lines.begin();
			iter != m_lines.end();
			++iter) {
		if ((*iter).isValid()) {
			if ((*iter).getLength()) {
				ret.w = std::max(
					ret.w,
					metrics->getStringWidth(m_text + (*iter).getOffset(),
						(*iter).getLength()
					)
				);
			}
			ret.h += fontHeight;
		}
	}
	ret.clamp(maxSize);
	if (maxSize.isValid() || m_maxSize.isValid()) {
		setMaximumSize(oldMaxSize);
	}
	return ret;
}

void
UTextLayout::render(UGraphics * g, const URectangle & rect, const UPoint & offset) {
	layout();
	int m_tabSize = 4;
	int tabMultiplier = m_font.getFontMetrics()->getMaxCharWidth();

	UFontRenderer * renderer = m_font.getRenderer();

	for (std::vector<UTextLine>::const_iterator iter = m_lines.begin();
			iter != m_lines.end();
			++iter) {
		UPoint pos(offset + (*iter).getPos());
		unsigned int start = (*iter).getOffset();
		unsigned int length = (*iter).getOffset() + (*iter).getLength();
		for (unsigned int i = (*iter).getOffset();
				i < length;
				++i) {
			if (m_text[i] == '\t') {
				pos.x += renderer->drawString(g, m_text + start, i - start, pos.x, pos.y);
				pos.x += m_tabSize * tabMultiplier;

				start = i + 1;
			}
		}
		if (start < length) {
			renderer->drawString(g, m_text + start, length - start, pos.x, pos.y);
		}
	}

}


URectangle
UTextLayout::modelToView(int offset) {
	layout();
	int m_tabSize = 4;
	UTextLine line = getLineForTextPosition(offset);
	if (!line.isValid()) {
		return URectangle();
	}
	const UFontMetrics * metrics = m_font.getFontMetrics();
	int fontHeight = metrics->getHeight();
	int tabMultiplier = metrics->getMaxCharWidth();//'m');
	int lineStart = line.getOffset();

	int addWidth = 0;
	int column = 0;
	for (int i = lineStart; i < offset && i < m_length; ++i) {
		if (m_text[i] == '\t') {
			addWidth += m_tabSize * tabMultiplier;
			// a good font renderer should omit control characters
			// but we can't rely on that.
			// add string width up to the tab and reset linestart and column.
			addWidth += metrics->getStringWidth(m_text + lineStart, column);
			lineStart = i + 1;
			column = 0;
		} else {
			column++;
		}
	}
	return URectangle(
		metrics->getStringWidth(m_text + lineStart, column) + addWidth,
		line.getPos().y,
		metrics->getCharWidth(m_text[lineStart + column]),
		fontHeight
	);
}


int
UTextLayout::viewToModel(const UPoint & pos) {
	if (pos.y < 0 || pos.x < 0) {
		return 0;
	}
	layout();
	int m_tabSize = 4;
	UTextLine line;
	for (std::vector<UTextLine>::const_iterator iter = m_lines.begin();
			iter != m_lines.end();
			++iter) {
		line = *iter;
		if (((*iter).getPos().y + (*iter).getHeight()) > pos.y) {
			break;
		}
	}
	const UFontMetrics * metrics = m_font.getFontMetrics();
	int tabMultiplier = metrics->getMaxCharWidth();//'m');

	unsigned int index = line.getOffset();

	int left = 0;

	for (;index < line.getOffset() + line.getLength() && index < m_length;
			++index) {
		if (m_text[index] == '\t') {
			int addWidth = m_tabSize * tabMultiplier;

			if ((pos.x >= left) && (pos.x <= left + addWidth)) {
				// if the cursor is nearer to the left letter, go to left
				if (pos.x >= left + addWidth / 2) {
					++index;
				}
				break; // break for loop
			}
			left += addWidth;
		} else {
			int addWidth = metrics->getCharWidth(m_text[index]);

			if ((pos.x >= left) && (pos.x <= left + addWidth)) {
				// if the cursor is nearer to the right letter, go to right
				if (pos.x >= left + addWidth / 2) {
					++index;
				}
				break; // break for loop
			}
			left += addWidth;
		}
	}

	return index;
}

bool
isWrapCharacter(char c) {
	return (c == ' ' ||
		c =='\n' ||
		c =='\t');
}

void
UTextLayout::layout() {
	if (m_validLayout) {
		return;
	}
	const UFontMetrics * metrics = m_font.getFontMetrics();
	int lineHeight = metrics->getHeight();
	int tabMultiplier = metrics->getMaxCharWidth();//'m');
	int m_tabSize = 4;

	int width = 0;
	int maxWidth = m_maxSize.w;

	m_lines.clear();
	unsigned int index = 0;

	unsigned int lineStart = 0;
	UPoint pos;

	// oops
	if (maxWidth <= 0) {
		UTextLine line(0, m_length, lineHeight, pos);
		m_lines.push_back(line);
		return;
	}

	while (index < m_length) {
		if (m_text[index] == '\t') {
			width += m_tabSize * tabMultiplier;
		} else if (m_text[index] == '\n') {
			UTextLine line(lineStart, index - lineStart, lineHeight, pos);
			m_lines.push_back(line);
			pos.y += lineHeight;
			// skip newline
			lineStart = index + 1;
			width = 0;
		} else {
			int addWidth = metrics->getCharWidth(m_text[index]);

			if (width + addWidth >= maxWidth) {
				unsigned int wrap_index = (index > 0) ? index - 1 : 0;
				char c = m_text[wrap_index];
				while (!isWrapCharacter(c) && wrap_index > lineStart) {
					--wrap_index;
					c = m_text[wrap_index];
				}
				if (lineStart == wrap_index) {
					wrap_index = (index > 0) ? index - 1 : 0;
				}
				if (wrap_index < lineStart) {
					// shouldn't happen?
					// but otherwise we would get a line length of -1
					wrap_index = lineStart;
				}
				UTextLine line(lineStart, wrap_index - lineStart, lineHeight, pos);
				m_lines.push_back(line);
				pos.y += lineHeight;
				lineStart = wrap_index + 1;
				width = 0;
			}
			width += addWidth;
		}
		index++;
	}
	if (lineStart < m_length || (m_length && m_text[m_length - 1] == '\n')) {
		UTextLine line(lineStart, m_length - lineStart, lineHeight, pos);
		m_lines.push_back(line);
	}
	m_validLayout = true;
}

int
UTextLayout::getLineCount() {
	return m_lines.size();
}

UTextLine
UTextLayout::getLine(int i) {
	if (i < getLineCount()) {
		return m_lines[i];
	}
	return UTextLine();
}

UTextLine
UTextLayout::getLineForTextPosition(int i) {
	layout();
	for (std::vector<UTextLine>::const_iterator iter = m_lines.begin();
			iter != m_lines.end();
			++iter) {
		if (((*iter).getOffset() + (*iter).getLength()) >= i) {
			return *iter;
		}
	}
	return UTextLine();
}

void
UTextLayout::invalidate() {
	m_validLayout = false;
}
