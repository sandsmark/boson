/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/basic/ubasictexteditui.hpp
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

#ifndef UBASICTEXTEDITUI_HPP
#define UBASICTEXTEDITUI_HPP

#include "../utexteditui.hpp"

namespace ufo {

class UCaret;
class UTextEdit;

/**
  *
  * This class is not part of the official UFO API and
  * may be changed without warning.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBasicTextEditUI : public UTextEditUI {
	UFO_DECLARE_DYNAMIC_CLASS(UBasicTextEditUI)
public:
	UBasicTextEditUI();
	virtual ~UBasicTextEditUI();

	static UBasicTextEditUI * createUI(UWidget * w);

	void paint(UGraphics * g, UWidget * w);

	void installUI(UWidget * w);
	void uninstallUI(UWidget * w);

	const std::string & getLafId();

	UDimension getPreferredSize(const UWidget * w);

protected: // Protected methods
	//virtual void paintSelection(UGraphics * g, UTextEdit * textEdit, UCaret * caret);
	//virtual void paintCaret(UGraphics * g, UTextEdit * textEdit, UCaret * caret);

protected: // Protected methods
	virtual void installSignals(UTextEdit * textEdit);
	virtual void uninstallSignals(UTextEdit * textEdit);

private:
	static UBasicTextEditUI * m_textEditUI;
	/** The shared look and feel id */
	static std::string m_lafId;
};

} // namespace ufo

#endif // UBASICTEXTEDITUI_HPP
