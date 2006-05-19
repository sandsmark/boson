/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/ulabel.hpp
    begin             : Wed May 23 2001
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

#ifndef ULABEL_HPP
#define ULABEL_HPP

#include "ucompound.hpp"

namespace ufo {

class UIcon;

/** @short A short, static, non-wrapping text field used to describe a
  * nearby control or to display an icon.
  * @ingroup widgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT ULabel : public UCompound {
	UFO_DECLARE_DYNAMIC_CLASS(ULabel)
	UFO_UI_CLASS(ULabelUI)
	UFO_STYLE_TYPE(UStyle::CE_Label)
public:
	ULabel();
	ULabel(UIcon * icon);
	ULabel(const std::string & text, UIcon * icon = NULL);
	/** Creates a label with given text and sets @p buddy as buddy widget.
	  * @see setBuddy
	  */
	ULabel(const std::string & text, UWidget * buddy);
	/** Creates a label with given text and icon and
	  * sets @p buddy as buddy widget.
	  * @see setBuddy
	  */
	ULabel(const std::string & text, UIcon * icon, UWidget * buddy);

	/** Sets the buddy or control widget. If the shortcut mnemonic of this
	  * label is pressed, the buddy is focused and if it is a button control,
	  * it will be activated.
	  */
	void setBuddy(UWidget * buddy);
	/** @see setBuddy */
	UWidget * getBuddy() const;

protected:  // Overrides UWidget
	virtual UDimension getContentsSize(const UDimension & maxSize) const;
	virtual void processShortcutEvent(UShortcutEvent * e);
private:
	UWidget * m_buddy;
};

} // namespace ufo

#endif // ULABEL_HPP
