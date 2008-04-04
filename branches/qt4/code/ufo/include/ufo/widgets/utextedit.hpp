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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/


#ifndef UTEXTEDIT_HPP
#define UTEXTEDIT_HPP

#include "utextwidget.hpp"

namespace ufo {

class UTextModel;

/** @short An editable multi-line text.
  * @ingroup widgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UTextEdit : public UTextWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UTextEdit)
	UFO_STYLE_TYPE(UStyle::CE_TextEdit)
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

	//
	// methods which change the document content
	//

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

	//
	//
	/** Sets the maximum length of the document.
	  * A value of 0 indicates no limit.
	  */
	void setMaxLength(int maxLength);
	/** Returns the maximum length of the document. */
	int getMaxLength() const;

public: // Overrides UWidget
	virtual void processKeyEvent(UKeyEvent * e);
	virtual void processMouseEvent(UMouseEvent * e);

private:  // Private attributes
	int m_maxLength;
};

} // namespace ufo

#endif // UTEXTEDIT_HPP
