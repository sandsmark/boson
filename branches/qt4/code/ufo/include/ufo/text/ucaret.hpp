/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/text/ucaret.hpp
    begin             : Thu Sep 6 2001
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

#ifndef UCARET_HPP
#define UCARET_HPP

#include "../uobject.hpp"
#include "../signals/usignal.hpp"

namespace ufo {

class UWidget;
class UTextEdit;
class UDocument;

/** @short A Caret represents a text cursor within a document.
  * @ingroup text
  *
  * Further it supports basic selection features via a position and a mark
  * handle.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UCaret : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UCaret)
public:/*
	UCaret();
	virtual ~UCaret();
*/
	/** Sets the position. The position is the actual place of the caret and
	  * the insertion point for charaters in the text widget. */
	virtual void setPosition(unsigned int posA) = 0;
	virtual unsigned int getPosition() const = 0;

	/** Explicitely sets the mark.
	  * Generally, you should use movePosition.
	  * @see movePosition
	  */
	virtual void setMark(unsigned int mark) = 0;
	/** The mark is a second text mark which is used to select regions.
	  * A region is selected between the caret position and the caret mark. */
	virtual unsigned int getMark() const = 0;


	/** Moves the cursor position from its current pos to the new pos, leaving
	  * a mark behind it (i.e. selecting the text).
	  */
	virtual void movePosition(unsigned int newPosition) = 0;

	virtual UDocument * getDocument() const = 0;

public: // Public signals
	typedef USignal3<UCaret*, unsigned int, unsigned int> CaretSignal;
	/** caret is the caret, first int the position, 2nd the mark */
	CaretSignal & sigPositionChanged();

private: // Private attributes
	CaretSignal m_sigPositionChanged;
};

//
// inline implementation
//

inline UCaret::CaretSignal &
UCaret::sigPositionChanged() {
	return m_sigPositionChanged;
}
/*
	// Paints the caret
	virtual void paint();

	// Sets the default blink rate of the cursor. Currently not supported.
	virtual void setBlinkRate(int blinkRateA);
	virtual int getBlinkRate() const;

	// @return the text widget which this caret is dedicated to.

	virtual const UTextEdit * getWidget() const;

	virtual void setVisible(bool v);
	virtual bool isVisible() const;

protected: // Protected slots
	void textInserted(UDocument * doc, unsigned int offset, unsigned int length);
	void textRemoved(UDocument * doc, unsigned int offset, unsigned int length);

protected:
	int m_position;
	int m_mark;

	int m_blinkRate;

	UTextEdit * m_textWidget;

	bool m_isVisible;
};
*/
} // namespace ufo

#endif // UCARET_HPP
