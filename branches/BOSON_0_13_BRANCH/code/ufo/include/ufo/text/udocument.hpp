/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/text/udocument.hpp
    begin             : Fri Aug 31 2001
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

#ifndef UDOCUMENT_HPP
#define UDOCUMENT_HPP

#include "../uobject.hpp"

#include "../signals/usignal.hpp"

namespace ufo {

class UCaret;

/** @short A low level interface for text documents.
  * @ingroup text
  *
  * It is an abstract character buffer for text widgets.
  * Usually this class is not used directly but via text widgets.
  *
  * @see UTextWidget
  * @author Johannes Schmidt
  */

class UFO_EXPORT UDocument : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UDocument)
public:
	/** Returns a const char pointer to the character array.
	  * Please note that this character array does not have to be
	  * NULL-terminated.
	  */
	virtual const char * getText() const = 0;

	/** Returns the total length of the text. */
	virtual unsigned int getLength() const = 0;

	/** Clears the entire content of the document. */
	virtual void clear() = 0;
	/** Appends the given character array at the end. */
	virtual void append(const char * chars, unsigned int nChars) = 0;

	/** Inserts the given character array at the current caret position
	  * replacing any currently active selection.
	  * This moves the caret to the new position behind the inserted text.
	  */
	virtual void replaceSelection(const char * chars, unsigned int nChars) = 0;

	/** A low level insert of the character at the given position without
	  * manipulating the caret.
	  */
	virtual void insertString(unsigned int offset,
		const char * chars, unsigned int nChars) = 0;

	/** Removes all characters from startA to (exclusively) endA.
	  */
	virtual void remove(unsigned int offset, unsigned int length) = 0;

	/** Replaces the given text portion with the given new text without
	  * modifying the caret position.
	  */
	virtual void replace(unsigned int offset, unsigned int length,
		const char * chars, unsigned int nChars) = 0;

	/** A caret is a text position within a document.
	  * It can also be used for text highlighting.
	  * @see UCaret
	  */
	virtual UCaret * getCaret() const = 0;

	//virtual void setCaret(UCaret * caret) = 0;


public: // Public signals
	typedef USignal4<UDocument*, unsigned int,
		unsigned int, unsigned int> DocumentSignal;

	/** document is this document,
	  * first int is text removal start, 2nd is length of the removed text,
	  * 3rd the length of the inserted text.
	  */
	DocumentSignal & sigTextReplaced();

private: // Private attributes
	DocumentSignal m_sigTextReplaced;
};

//
// inline implementation
//

inline UDocument::DocumentSignal &
UDocument::sigTextReplaced() {
	return m_sigTextReplaced;
}

} // namespace ufo

#endif // UDOCUMENT_HPP
