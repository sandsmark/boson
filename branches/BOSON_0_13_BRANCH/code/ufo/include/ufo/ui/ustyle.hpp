/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/ustyle.hpp
    begin             : Sat Nov 29 2003
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


#ifndef USTYLE_HPP
#define USTYLE_HPP

#include "../uobject.hpp"

#include "../util/udimension.hpp"
#include "../util/urectangle.hpp"
#include "../util/uinsets.hpp"

namespace ufo {

class UGraphics;
class UColor;
class UFont;
class UIcon;

class UWidget;
class UCaret;

class UStyleHints;
class UStyleOption;
class UWidgetModel;
class UBorderModel;


/** @short The style class provides the look and feel for common widgets.
  * @ingroup appearance
  *
  * Actually you do not have to have a certain widget for painting, just
  * a model which describe its contents.
  *
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UStyle : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UStyle)
public: // Public types
	enum PrimitiveElement {
		/** */
		PE_IndicatorArrowUp,
		PE_IndicatorArrowDown,
		PE_IndicatorArrowLeft,
		PE_IndicatorArrowRight,
		PE_IndicatorCheckBox,
		PE_IndicatorCheckBoxMask,
		PE_IndicatorRadioButton,
		PE_IndicatorRadioButtonMask,
		PE_IndicatorButtonDropDown,
		PE_TextCaret,
		PE_PanelButtonBevel,
		PE_PanelButtonTool,
		PE_PanelToolBar,
		PE_PanelWidget,
		PE_Frame,
		PE_FrameWindow,
		PE_FrameMenu,
		PE_FrameButtonBevel,
		PE_FrameDefaultButton,
		PE_FrameTabWidget,
		PE_FrameStatusBar,
		PE_FrameLineEdit,
		PE_FrameFocusRect,
		PE_Gripper
	};
	enum ComponentElement {
		CE_Widget,
		CE_Compound,
		CE_Label,
		CE_StaticText,
		CE_Button,
		CE_ToolButton,
		CE_CheckBox,
		CE_RadioButton,
		CE_MenuItem,
		CE_MenuBarItem,
		CE_MenuTearOff,
		CE_ListBox,
		CE_LineEdit,
		CE_TextEdit,
		CE_TextWidget,
		CE_Separator,
		CE_ProgressBar,
		CE_ProgressBarGroove,
		CE_ProgressBarContents,
		CE_TabBarTab,
		CE_Splitter,
		CE_ComboBox,
		CE_SpinBox,
		CE_Slider,
		CE_TitleBar,
		CE_InternalFrame,
		CE_ScrollBar,
		CE_GroupBox
	};
	enum SubControls {
		SC_None = 0,
		SC_ScrollBarHome = 1 << 0,
		SC_ScrollBarAddLine = 1 << 1,
		SC_ScrollBarSubLine = 1 << 2,
		SC_ScrollBarAddPage = 1 << 3,
		SC_ScrollBarSubPage = 1 << 4,
		SC_ScrollBarSlider = 1 << 5,
		SC_ScrollBarGroove = 1 << 6,
		SC_ScrollBarEnd = 1 << 7,
		SC_ScrollBarFirst = SC_ScrollBarHome,
		SC_ScrollBarLast = SC_ScrollBarEnd,
		SC_SpinBoxUp = 1 << 0,
		SC_SpinBoxDown = 1 << 1,
		SC_SpinBoxFrame = 1 << 2,
		SC_SpinBoxEditField = 1 << 3,
		SC_SpinBoxFirst = SC_SpinBoxUp,
		SC_SpinBoxLast = SC_SpinBoxEditField,
		SC_ComboBoxEditField = 1 << 0,
		SC_ComboBoxArrow = 1 << 1,
		SC_ComboBoxFrame = 1 << 2,
		SC_ComboBoxListBoxPopup = 1 << 3,
		SC_ComboBoxFirst = SC_ComboBoxEditField,
		SC_ComboBoxLast = SC_ComboBoxListBoxPopup,
		SC_SliderGroove = 1 << 0,
		SC_SliderHandle = 1 << 1,
		SC_SliderTickmarks = 1 << 2,
		SC_SliderFirst = SC_SliderGroove,
		SC_SliderLast = SC_SliderTickmarks,
		SC_ToolButton = 1 << 0,
		SC_ToolButtonMenu = 1 << 1,
		SC_TitleBarSysMenu = 1 << 0,
		SC_TitleBarMinButton = 1 << 1,
		SC_TitleBarMaxButton = 1 << 2,
		SC_TitleBarCloseButton = 1 << 3,
		SC_TitleBarLabel = 1 << 4,
		SC_TitleBarFirst = SC_TitleBarSysMenu,
		SC_TitleBarLast = SC_TitleBarLabel
	};
public: // basic drawing
	/** Paints a primitive element like arrows and frames. */
	virtual void paintPrimitive(UGraphics * g,
		PrimitiveElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		uint32_t widgetState) = 0;

	/** Paints a component of a widget resp. the whole widget.
	  * For most components, this is only a call to paintModelBackground and
	  * paintModel.
	  */
	virtual void paintComponent(UGraphics * g,
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w = NULL) = 0;

	/** Paints a border. */
	virtual void paintBorder(UGraphics * g,
		uint32_t borderType,
		const URectangle & rect,
		const UStyleHints * hints,
		uint32_t widgetState) = 0;

	/** @return The insets used by a border. */
	virtual UInsets getBorderInsets(
		ComponentElement elem,
		const UStyleHints * hints) = 0;

	/** @return The sub control at the given position or @p SC_None.
	  */
	virtual SubControls getSubControlAt(
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		const UPoint & pos,
		UWidget * w = NULL) = 0;

	/** @return The bounding rectangle of a sub control .*/
	virtual URectangle getSubControlBounds(
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		SubControls subElem,
		UWidget * w = NULL) = 0;


	/** @return The insets used between content (like icons, text)
	  *  and the actual widget bounds.
	  */
	virtual UInsets getInsets(
		ComponentElement elem,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w = NULL) = 0;

	/** May return a slightly enlarged dimension to add space
	  * for focus, styled borders or icons.
	  */
	virtual UDimension getSizeFromContents(
		ComponentElement elem,
		const UDimension & contentsSize,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w = NULL) = 0;

	/** Paints an icon with text. */
	virtual void paintCompound(
		UGraphics * g,
		const UStyleHints * hints,
		const std::string & text,
		UIcon * icon,
		const URectangle & rect,
		uint32_t widgetState,
		int acceleratorIndex = -1) = 0;

	/** @return The preferred size for an icon with text */
	virtual UDimension getCompoundPreferredSize(
		const UStyleHints * hints,
		const std::string & text,
		UIcon * icon) = 0;

	/** Lays out an icon with text. */
	virtual void layoutCompound(
		const UStyleHints * hints,
		const std::string & text,
		UIcon * icon,
		const URectangle & viewRect,
		URectangle * textRect,
		URectangle * iconRect) = 0;

	/** Stub method. Configures a widget to be used with this style.
	  * Not yet implementd.
	  */
	virtual void install(UWidget * w) = 0;
	virtual void uninstall(UWidget * w) = 0;
#if 0 
// API ideas
	/** @return The preferred contents size for the given model.
	  */
	virtual UDimension getPreferredSizeFromModel(
		ComponentElement elem,
		const UDimension & maxSize,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w = NULL) = 0;

	/** Paints the background for the given component element,
	  * eventually using the given model.
	  */
	virtual void paintModelBackground(
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w = NULL) = 0;

	/** Paints the contents of the given model.
	  * This may be a label and icon of buttons, text of text widgets etc.
	  */
	virtual void paintModel(
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w = NULL) = 0;

	/** Returns a two dimensional model index currently represented as UPoint.
	  *
	  */
	virtual UPoint viewToModel(
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		const UPoint & pos,
		UWidget * w = NULL) = 0;

	/** @return The bounding rectangle of a model index.*/
	virtual URectangle modelToView(
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		const UPoint & pos,
		UWidget * w = NULL) = 0;
#endif
};

} // namespace ufo

#endif // USTYLE_HPP
