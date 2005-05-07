/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2004 by Andreas Beckermann
    email             : b_mann at gmx.de
                             -------------------

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

#ifndef UBOPROGRESSUI_H
#define UBOPROGRESSUI_H

#include <ufo/ui/uwidgetui.hpp>


namespace ufo {

class UBoProgress;
class UColor;

class UFO_EXPORT UBoProgressUI : public UWidgetUI {
	UFO_DECLARE_DYNAMIC_CLASS(UBoProgressUI)
public:
	static UBoProgressUI * createUI(UWidget * w);

	void paint(UGraphics * g, UWidget * w);

	const std::string & getLafId();

	UDimension getPreferredSize(const UWidget * w);

protected:
	void paintGradient(UGraphics * g, const UBoProgress * progress, const UColor& from, const UColor& to);

private:  // Private attributes
	/**  */
	static UBoProgressUI * m_progressUI;
	/** The shared look and feel id */
	static std::string m_lafId;
};

} // namespace ufo

#endif

