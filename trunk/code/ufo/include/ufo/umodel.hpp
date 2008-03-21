/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/umodel.hpp
    begin             : Thu Mar 10 2005
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

#ifndef UMODEL_HPP
#define UMODEL_HPP

#include "uobject.hpp"

#include "ukeystroke.hpp"
#include "util/ucolor.hpp"
#include "ui/ustyle.hpp"

namespace ufo {

class UIcon;
class UDocument;
class UTextLayout;

/** @short A widget model represents the application data of a widget
  *  which is painted by UStyle and modified by UWidget.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UWidgetModel {
public:
	uint32_t widgetState;
};

/** @short A border model describes the border of a widget
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UBorderModel : public UWidgetModel {
public:
	/** The border type.
	  * @see BorderType
	  */
	uint32_t borderType;
	/** For colors for every side of the widget (top, right, bottom, left).
	  */
	UColor color[4];
	/** border style used by CssBorder: top right bottom left
	  * @see UBorderStyle
	  */
	uint8_t style[4];
	/** The border width (top, right, bottom, left). */
	uint8_t width[4];
	/** The border radius used to smooth border corners. */
	uint8_t radius[4];
};

/** @short The compound model represents the application data of a compound
  *  which is used for drawing and to react on user input.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UCompoundModel : public UWidgetModel {
public:
	std::string text;
	UIcon * icon;
	int acceleratorIndex;
};

/** @short The button model represents the application data of a generic
  *  button (including check boxes and radio buttons).
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UButtonModel : public UCompoundModel {
public:
	enum ButtonFeatures {
		None = 0,
		Flat = 1,
		DefaultButton = 2,
		HasMenu = 4
	};
	enum CheckType {
		NotCheckable = 0,
		Exclusive,
		NonExclusive
	};
	int buttonFeatures;
	int checkType;
	UKeyStroke shortcut;
};

/** @short The menu item model represents the application data of a menu item
  *  which is used for drawing and to react on user input.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UMenuItemModel : public UButtonModel {
public:
	int maxIconWidth;
};

/** @short The group box model represents the application data of a group box.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UGroupBoxModel : public UWidgetModel {
public:
	std::string text;
	UIcon * icon;
	int acceleratorIndex;
};

/** @short The text model represents the application data of a text widget.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UTextModel : public UWidgetModel {
public:
	UDocument * document;
	UTextLayout * textLayout;
};

/** @short The progress bar model represents the application data of a
  *  progress bar.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UProgressBarModel : public UWidgetModel {
public:
	int minimum;
	int maximum;
	int value;
	std::string text;
	bool textVisible;
};

/** @short The complex model represents the application data of a
  *  widget with sub controls.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UComplexModel : public UWidgetModel {
public:
	UStyle::SubControls subControls;
	UStyle::SubControls activeSubControls;
};

/** @short The slider model represents the application data of a slider.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class USliderModel : public UComplexModel {
public:
	int minimum;
	int maximum;
	int tickInterval;
	int sliderPosition;
	int sliderValue;
	int unitIncrement;
	int blockIncrement;
};

/** @short The spin box model represents the application data of a spin box.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class USpinBoxModel : public UComplexModel {
public:
	float minimum;
	float maximum;
	float value;
};

/** @short The title bar model represents the application data of a title bar.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UTitleBarModel : public UComplexModel {
public:
	std::string text;
	UIcon * icon;
	int frameState;
	int frameStyle;
};

} // namespace ufo

#endif // UMODEL_HPP
