/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/ulineedit.hpp
    begin             : Thu Sep 16 2004
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

#ifndef ULINEEDIT_HPP
#define ULINEEDIT_HPP

#include "utextedit.hpp"

namespace ufo {

class UActionEvent;
class UKeyEvent;
class UValidator;

/** @short A widget for single line input fields.
  * @ingroup widgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT ULineEdit : public UTextEdit {
	UFO_DECLARE_DYNAMIC_CLASS(ULineEdit)
	UFO_STYLE_TYPE(UStyle::CE_LineEdit)
public:
	/** */
	ULineEdit();
	/** Creates a text widget with initial text
	  * @param text The initial text
	  */
	ULineEdit(const std::string & text);

public: // Public methods
	void setValidator(const UValidator * val);
	const UValidator * getValidator() const;

	int getInt() const;
	double getDouble() const;

	/** If @p text is not empty, the text size is used as preferred size. */
	void setPrototype(const std::string & text);

public: // Public signals
	/** This signal is fired when the user pressed Enter or Return. */
	USignal1<UActionEvent*> & sigActivated();

protected: // Overrides UTextEdit
	virtual void processKeyEvent(UKeyEvent * e);
	virtual UDimension getContentsSize(const UDimension & maxSize) const;

private: // Private attributes
	const UValidator * m_validator;
	std::string m_prototype;

private: // Private signals
	USignal1<UActionEvent*> m_sigActivated;
};

//
// inline implementation
//

inline
USignal1<UActionEvent*> &
ULineEdit::sigActivated() {
	return m_sigActivated;
}

} // namespace ufo

#endif // ULINEEDIT_HPP
