/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/text/ubasicdocument.cpp
    begin             : Sat Dec 15 2001
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

#include "ufo/text/ubasicdocument.hpp"

#include "ufo/signals/ufo_signals.hpp"
#include "ufo/utrait.hpp"

namespace ufo {

//-----------------------------
//
// class basic document
//
//-----------------------------

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicDocument, UDocument)
UFO_IMPLEMENT_ABSTRACT_CLASS(UDocument, UObject)

// common abbreviations
inline void
fireSigRemoved(UDocument * doc, unsigned int start, unsigned int length) {
	doc->sigTextReplaced().emit(doc, start, length, 0);
}
inline void
fireSigInserted(UDocument * doc, unsigned int start, unsigned int length) {
	doc->sigTextReplaced().emit(doc, start, 0, length);
}
inline void
fireSigReplaced(UDocument * doc, unsigned int start,
		unsigned int lengthRemoved, unsigned int lengthInserted) {
	doc->sigTextReplaced().emit(doc, start, lengthRemoved, lengthInserted);
}

UBasicDocument::UBasicDocument()
	: m_content()
	, m_caret(NULL)
{
	m_caret = createCaret();
	trackPointer(m_caret);
}

UCaret *
UBasicDocument::createCaret() {
	return new UBasicCaret(this);
}

const char *
UBasicDocument::getText() const {
	return m_content.c_str();
}

unsigned int
UBasicDocument::getLength() const {
	return m_content.length();
}

void
UBasicDocument::clear() {
	unsigned int oldLength = m_content.length();
	// FIXME msvc 6 STL woes
	//m_content.clear();
	m_content = "";
	fireSigRemoved(this, 0, oldLength);
}

void
UBasicDocument::append(const char * chars, unsigned int nChars) {
	unsigned int oldLength = m_content.length();
	m_content.append(chars, nChars);
	fireSigInserted(this, oldLength, nChars);
}

void
UBasicDocument::replaceSelection(const char * chars, unsigned int nChars) {
	unsigned int replaceStart = std::min(m_caret->getPosition(), m_caret->getMark());
	unsigned int replaceEnd = std::max(m_caret->getPosition(), m_caret->getMark());
	unsigned int length = replaceEnd - replaceStart;

	m_content.replace(replaceStart, length, chars, nChars);
	updateCaret(replaceStart, length, nChars);

	fireSigReplaced(this, replaceStart, length, nChars);
}

void
UBasicDocument::insertString(unsigned int offset,
		const char * chars, unsigned int nChars) {
	m_content.insert(offset, chars, nChars);
	updateCaret(offset, 0, nChars);

	fireSigInserted(this, offset, nChars);
}

void
UBasicDocument::remove(unsigned int offset, unsigned int length) {
	m_content.erase(offset, length);
	updateCaret(offset, length, 0);

	fireSigRemoved(this, offset, length);
}

void
UBasicDocument::replace(unsigned int offset, unsigned int length,
		const char * chars, unsigned int nChars) {
	m_content.replace(offset, length, chars, nChars);
	updateCaret(offset, length, nChars);

	fireSigReplaced(this, offset, length, nChars);
}


UCaret *
UBasicDocument::getCaret() const {
	return m_caret;
}

void
UBasicDocument::updateCaret(unsigned int offset,
		unsigned int rmLength, unsigned int insLength) {
	unsigned int length = insLength - rmLength;
	unsigned int cpos = m_caret->getPosition();
	unsigned int mark = m_caret->getMark();
	if (cpos >= offset) {
		if (cpos >= offset + rmLength) {
			m_caret->setPosition(cpos + length);
		} else {
			m_caret->setPosition(offset + insLength);
		}
	}

	if (mark != cpos) {
		if (mark >= offset) {
			if (mark >= offset + rmLength) {
				m_caret->setMark(mark + length);
			} else {
				m_caret->setMark(offset + insLength);
			}
		}
	}
}

//-----------------------------
//
// class UBasicCaret
//
//-----------------------------


UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicCaret, UCaret)
UFO_IMPLEMENT_ABSTRACT_CLASS(UCaret, UObject)

UBasicCaret::UBasicCaret(UDocument * doc)
	: m_document(doc)
	, m_position(0)
	, m_mark(0)
{}

UBasicCaret::~UBasicCaret() {}

void
UBasicCaret::setPosition(unsigned int posA) {
	m_position = posA;
	m_mark = m_position;

	if (m_document && m_document->getLength() < m_position) {
		m_position = m_document->getLength();
		m_mark = m_position;
	}
	sigPositionChanged().emit(this, m_position, m_mark);
}
unsigned int
UBasicCaret::getPosition() const {
	return m_position;
}

unsigned int
UBasicCaret::getMark() const {
	return m_mark;
}

void
UBasicCaret::setMark(unsigned int mark) {
	m_mark = mark;
	if (m_document && m_document->getLength() < mark) {
		m_mark = m_document->getLength();
	}
}

void
UBasicCaret::movePosition(unsigned int markA) {
	//m_mark = m_position;
	m_position = markA;
	// clamp
	if (m_document && m_document->getLength() < m_position) {
		m_position = m_document->getLength();
	}
	sigPositionChanged().emit(this, m_position, m_mark);
}

UDocument *
UBasicCaret::getDocument() const {
	return m_document;
}

} // namespace ufo
