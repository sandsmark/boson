/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uspinbox.hpp
    begin             : Wed Apr 6 2005
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

#ifndef USPINBOX_HPP
#define USPINBOX_HPP

#include "uwidget.hpp"

namespace ufo {

class USpinBoxModel;
class ULineEdit;

/** @short A Spin Box is a (editable) number input whose content may be
  *  changed via up/down buttons.
  * @ingroup widgets
  *
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT USpinBox : public UWidget {
	UFO_DECLARE_CLASS(USpinBox)
	UFO_STYLE_TYPE(UStyle::CE_SpinBox)
public:
	USpinBox();
	virtual ~USpinBox();

public:
	float getMinimum() const;
	void setMinimum(float min);

	float getMaximum() const;
	void setMaximum(float max);

	void setRange(float min, float max);

	float getValue() const;
	void setValue(float newValue);

public:
	USignal1<USpinBox*> & sigValueChanged();

protected: // Overrides UWidget
	virtual void processMouseEvent(UMouseEvent * e);

protected:
	USpinBoxModel * getSpinBoxModel() const;
	void valueChanged(UActionEvent * e);

private:
	// line edit
	ULineEdit * m_lineEdit;
private: // Private signals
	/**  */
	USignal1<USpinBox*> m_sigValueChanged;
};

//
// inline implementation
//

inline USignal1<USpinBox*> &
USpinBox::sigValueChanged() {
	return m_sigValueChanged;
}

} // namespace ufo

#endif // USPINBOX_HPP
