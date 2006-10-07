/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/utextwidget.hpp
    begin             : Sa Apr 2 2005
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


#ifndef UTEXTWIDGET_HPP
#define UTEXTWIDGET_HPP

#include "uscrollablewidget.hpp"

namespace ufo {

class UDocument;
class UTextModel;
class UCaret;
class UTextLayout;

/** @short A static multi-line text widget.
  * @ingroup widgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UTextWidget : public UScrollableWidget, public std::streambuf {
	UFO_DECLARE_DYNAMIC_CLASS(UTextWidget)
	UFO_STYLE_TYPE(UStyle::CE_TextWidget)
public:
	/** Creates a new multi line text widget */
	UTextWidget();
	/** Creates a text widget with initial text
	  * @param text The initial text
	  */
	UTextWidget(const std::string & text);
	virtual ~UTextWidget();

public: // Public methods
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

	UTextLayout * getTextLayout() const;

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
	virtual void validateSelf();
	virtual void processMouseEvent(UMouseEvent * e);
	virtual UDimension getContentsSize(const UDimension & maxSize) const;

public: // Public streaming methods
	/** Returns a std stream buffer wich can be used to redirect std::cout */
	virtual std::streambuf * rdbuf();

	int overflow(int c);

	// stream-like insertion operators
	UTextWidget & operator<<(const std::string & str);
	UTextWidget & operator<<(int i);
	UTextWidget & operator<<(long i);
	UTextWidget & operator<<(float f);
	UTextWidget & operator<<(double d);
	UTextWidget & operator<<(const char * cstr);

protected:
	UTextModel * getTextModel() const;
	void docChanged(UDocument*, unsigned int,
		unsigned int, unsigned int);
protected:  // Protected attributes
	/** an internal text clipboard */
	static std::string m_textBuffer;

private:  // Private attributes
	UDocument * m_doc;
	UTextLayout * m_textLayout;

	/** the content type ( plain text, XML, .. ) in MIME style,
	  * i.e. text/plain etc.
	  */
	std::string m_type;

	unsigned int m_columns;
	unsigned int m_rows;
};

} // namespace ufo

#endif // UTEXTWIDGET_HPP
