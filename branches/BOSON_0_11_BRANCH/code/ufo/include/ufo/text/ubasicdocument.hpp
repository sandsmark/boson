/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/text/ubasicdocument.hpp
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

#ifndef UBASICDOCUMENT_HPP
#define UBASICDOCUMENT_HPP

#include "udocument.hpp"
#include "ucaret.hpp"

namespace ufo {

/** @short A plain text document
  * @ingroup text
  * @ingroup internal
  *
  * @see UDocumentFactory
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBasicDocument : public UDocument  {
	UFO_DECLARE_DYNAMIC_CLASS(UBasicDocument)
public:
	UBasicDocument();

	virtual UCaret * createCaret();

public: // Implements UDocument
	virtual const char * getText() const;

	virtual unsigned int getLength() const;

	virtual void clear();
	virtual void append(const char * chars, unsigned int nChars);

	virtual void replaceSelection(const char * chars, unsigned int nChars);

	virtual void insertString(unsigned int offset,
		const char * chars, unsigned int nChars);

	virtual void remove(unsigned int offset, unsigned int length);

	virtual void replace(unsigned int offset, unsigned int length,
		const char * chars, unsigned int nChars);

	virtual UCaret * getCaret() const;
	//virtual void setCaret(UCaret * caret);

protected: // Protected methods
	virtual void updateCaret(unsigned int offset,
		unsigned int rmLength, unsigned int insLength);

private: // Private attributes
	std::string m_content;
	UCaret * m_caret;
};

/** A basic implementation of a text caret
  * @author Johannes Schmidt
  */
class UFO_EXPORT UBasicCaret : public UCaret {
	UFO_DECLARE_DYNAMIC_CLASS(UBasicCaret)
public:
	UBasicCaret(UDocument * document);
	virtual ~UBasicCaret();

public: // Implements UCaret
	virtual void setPosition(unsigned int posA);
	virtual unsigned int getPosition() const;

	virtual void setMark(unsigned int mark);
	virtual unsigned int getMark() const;

	virtual void movePosition(unsigned int newPosition);

	virtual UDocument * getDocument() const;

private:
	UDocument * m_document;
	unsigned int m_position;
	unsigned int m_mark;
};

} // namespace ufo

#endif // UBASICDOCUMENT_HPP
