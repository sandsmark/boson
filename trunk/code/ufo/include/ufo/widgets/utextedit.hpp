/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/utextedit.hpp
    begin             : Wed Mar 26 2003
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


#ifndef UTEXTEDIT_HPP
#define UTEXTEDIT_HPP

#include "uwidget.hpp"


#include "../text/udocument.hpp"
#include "../text/udocumentrenderer.hpp"


#include "../text/ucaret.hpp"

namespace ufo {

/** A widget for dynamic texts.
  * @author Johannes Schmidt
  */

class UFO_EXPORT UTextEdit : public UWidget, public std::streambuf {
	UFO_DECLARE_DYNAMIC_CLASS(UTextEdit)
	UFO_UI_CLASS(UTextEditUI)
public:
	/** Creates a new multi line text widget */
	UTextEdit();
	/** Creates a text widget with initial text
	  * @param text The initial text
	  */
	UTextEdit(const std::string & text);

	/** Sets whether the text is editable by the user.
	  * This does not affect the use of operators or direct document use.
	  */
	void setEditable(bool b);
	bool isEditable() const;

	/** Sets the content type. This may change the document class.
	  * So far, only plain text documents are supported.
	  */
	void setContentType(const std::string & type);
	std::string getContentType() const;

	/** Sets the document object for this text widget.
	  * If you want to manipulate the text content directly, you must get
	  * an instance of UDocument with getDocument()
	  * @see getDocument
	  * @see UDocument
	  */
	void setDocument(UDocument * documentA);
	UDocument * getDocument() const;

	/** Sets the document renderer for this text widget. */
	void setRenderer(UDocumentRenderer * documentRendererA);
	UDocumentRenderer * getRenderer() const;

	// methods which change the document content

	/** cuts the selected text to the internal text clipboard */
	virtual void cut();
	/** copies the selected text to the internal text clipboard
	  */
	virtual void copy();
	/** pastes from the internal clipboard to selection */
	virtual void paste();

	/** Sets the text of this text widget.
	  */
	virtual void setText(const std::string & textA);

	/** Returns the entire plain text of the document.
	  */
	virtual std::string getText() const;

	/** Returns only the currently selected text. */
	virtual std::string getSelectedText() const;

	// some selection methods

	void setSelection(unsigned int indexFrom, unsigned int indexTo);
	void getSelection(unsigned int * indexFrom, unsigned int * indexTo) const;
	bool hasSelection() const;

	//
	// caret methods
	//
	/**
	  */
	void setCaretPosition(unsigned int positionA);
	/** moves the caret to the new position leaving
	  * a mark at the old position
	  */
	void moveCaretPosition(unsigned int positionA);

	//void setCaret(UCaret * caretA);
	UCaret * getCaret() const;

	//
	//
	/** Sets the maximum length of the document.
	  * A value of 0 indicates no limit.
	  */
	void setMaxLength(int maxLength);
	/** Returns the maximum length of the document. */
	int getMaxLength() const;

	//
	// size hints
	//
	/** Sets the preferred numer of columns.
	  * If set to 0, a variable size depending on the current text is used.
	  */
	void setColumns(unsigned int columnsA);
	unsigned int getColumns() const;

	/** Sets the preferred numer of rows.
	  * If set to 0, a variable size depending on the current text is used.
	  */
	void setRows(unsigned int rowsA);
	unsigned int getRows() const;

public: // Overrides UWidget
	virtual bool isActive() const;
	virtual void processKeyEvent(UKeyEvent * e);

public: // Public streaming methods
	/** Returns a std stream buffer wich can be used to redirect std::cout */
	virtual std::streambuf * rdbuf();

	int overflow(int c);

	// stream-like insertion operators
	UTextEdit & operator<<(const std::string & str);
	UTextEdit & operator<<(int i);
	UTextEdit & operator<<(long i);
	UTextEdit & operator<<(float f);
	UTextEdit & operator<<(double d);
	UTextEdit & operator<<(const char * cstr);

protected:  // Protected attributes
	/** an internal text clipboard */
	static std::string m_textBuffer;
	/** the content type ( plain text, XML, .. ) in MIME style,
	  * i.e. text/plain etc.
	  */
	std::string m_type;

private:  // Private attributes
	bool m_isEditable;

	UDocument * m_doc;
	UDocumentRenderer * m_docRenderer;

	//UCaret * m_caret;

	int m_maxLength;
	unsigned int m_columns;
	unsigned int m_rows;
};

} // namespace ufo

#endif // UTEXTEDIT_HPP
