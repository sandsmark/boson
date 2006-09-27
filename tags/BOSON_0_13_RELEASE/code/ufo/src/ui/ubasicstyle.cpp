/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/gl/ubasicstyle.cpp
    begin             : Sat Mar 5 2005
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

#include "ufo/ui/ubasicstyle.hpp"

#include "ufo/ui/ustyle.hpp"
#include "ufo/ui/ustylehints.hpp"
#include "ufo/widgets/uwidget.hpp"
#include "ufo/widgets/ubutton.hpp"
#include "ufo/widgets/ulabel.hpp"
#include "ufo/widgets/utextedit.hpp"
#include "ufo/text/udocument.hpp"
#include "ufo/text/utextlayout.hpp"
#include "ufo/text/ucaret.hpp"
#include "ufo/ugraphics.hpp"
#include "ufo/uvertexarray.hpp"
#include "ufo/font/ufontmetrics.hpp"
#include "ufo/umodel.hpp"

using namespace ufo;

UBasicStyle::UBasicStyle() {
}


int
basicstyle_sliderModelToView(int min, int max, int value, int visibleSpace) {
	float delta = float(visibleSpace) / max;

	return int(value * delta);
}

int
basicstyle_sliderViewToModel(int min, int max, int pos, int visibleSpace) {
	float delta = float(visibleSpace) / max;

	return int(pos / delta);
}

void
basicstyle_paintIndicatorArrow(
		UGraphics * g,
		UStyle::PrimitiveElement elem,
		const URectangle & rect,
		const UColor & color,
		uint32_t styleFlags)
{
	int x[3], y[3];
	int middle_x = rect.x + rect.w / 2;
	int middle_y = rect.y + rect.h / 2;

	int half_size = 2;

	switch (elem) {
		case UStyle::PE_IndicatorArrowUp:
			x[0] = middle_x;
			x[1] = middle_x + 2*half_size;
			x[2] = middle_x - 2*half_size;
			y[0] = middle_y - half_size;
			y[1] = y[2] = middle_y + half_size;
		break;
		case UStyle::PE_IndicatorArrowDown:
			x[0] = middle_x;
			x[1] = middle_x - 2*half_size;
			x[2] = middle_x + 2*half_size;
			y[0] = middle_y + half_size;
			y[1] = y[2] = middle_y - half_size;
		break;
		case UStyle::PE_IndicatorArrowLeft:
			x[0] = middle_x - half_size;
			x[1] = x[2] = middle_x + half_size;
			y[0] = middle_y;
			y[1] = middle_y - 2*half_size;
			y[2] = middle_y + 2*half_size;
		break;
		case UStyle::PE_IndicatorArrowRight:
		// shouldn't happen, but use right arrow as default
		default:
			x[0] = middle_x + half_size;
			x[1] = x[2] = middle_x - half_size;
			y[0] = middle_y;
			y[1] = middle_y + 2*half_size;
			y[2] = middle_y - 2*half_size;
		break;
	}

	g->setColor(color);
	// prettify arrow (anti-aliasing)
	/*
	ugl_driver->glPushAttrib(GL_COLOR_BUFFER_BIT);
	ugl_driver->glEnable(GL_BLEND);
	ugl_driver->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	*/
	UVertexArray vertexArray;
	vertexArray.add(x[0], y[0]);
	vertexArray.add(x[1], y[1]);
	vertexArray.add(x[2], y[2]);

	if (styleFlags & (WidgetHighlighted | WidgetPressed)) {
		bool wasAntialias = g->isEnabled(UGraphics::FillAntialiasing);
		g->setEnabled(UGraphics::FillAntialiasing, true);
		g->drawVertexArray(UGraphics::Triangles, &vertexArray);
		if (!wasAntialias) {
			g->setEnabled(UGraphics::FillAntialiasing, false);
		}
	} else {
		bool wasAntialias = g->isEnabled(UGraphics::LineAntialiasing);
		g->setEnabled(UGraphics::LineAntialiasing, true);
		// add end point
		vertexArray.add(x[0], y[0]);
		g->drawVertexArray(UGraphics::LineStrip, &vertexArray);
		if (!wasAntialias) {
			g->setEnabled(UGraphics::LineAntialiasing, false);
		}
	}
	/*
	ugl_driver->glVertex2i(x[0], y[0]);
	ugl_driver->glVertex2i(x[1], y[1]);
	ugl_driver->glVertex2i(x[2], y[2]);
	ugl_driver->glEnd();
	// unset smoothness
	ugl_driver->glPopAttrib();
	ugl_driver->glPopAttrib();
	*/
}


void
UBasicStyle::paintPrimitive(
		UGraphics * g,
		PrimitiveElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		uint32_t widgetState)
{
	switch (elem) {
		case PE_IndicatorArrowUp:
		case PE_IndicatorArrowDown:
		case PE_IndicatorArrowLeft:
		case PE_IndicatorArrowRight:
			basicstyle_paintIndicatorArrow(g, elem, rect,
				hints->palette.foreground(), widgetState);
		break;

		case PE_Gripper: {
			UColor dark = hints->palette.background().darker();
			UColor light = hints->palette.background().brighter();
			// FIXME: wow, very inefficient
			for (int x = 2; x <= rect.w - 2; x += 4) {
				for (int y = 2; y <= rect.h - 2; y += 4) {
					g->setColor(dark);
					g->fillRect(rect.x + x, rect.y + y, 2, 2);
					g->setColor(light);
					g->fillRect(rect.x + x + 1, rect.y + y + 1, 1, 1);
				}
			}
		}
		break;
		case PE_IndicatorCheckBox:
			if (widgetState & WidgetDisabled) {
				g->setColor(UColor::gray);
			} else {
				g->setColor(UColor::black);
			}
			g->drawRect(rect);
			g->setColor(UColor::white);
			g->fillRect(rect - UInsets(1, 1, 1, 1));
		break;
		case PE_IndicatorRadioButton: {
			UVertexArray array;
			array.add(rect.x + 4, rect.y + 0);
			array.add(rect.x + 0, rect.y + 4);
			array.add(rect.x + 0, rect.y + 6);
			array.add(rect.x + 4, rect.y + 10);
			array.add(rect.x + 6, rect.y + 10);
			array.add(rect.x + 10, rect.y + 6);
			array.add(rect.x + 10, rect.y + 4);
			array.add(rect.x + 6, rect.y + 0);
			array.add(rect.x + 4, rect.y + 0);
			g->setColor(UColor::white);
			g->drawVertexArray(UGraphics::TriangleFan, &array);
			g->setColor(UColor::gray);
			g->drawVertexArray(UGraphics::LineStrip, &array);
		}
		break;
		case PE_IndicatorCheckBoxMask:
			if (widgetState & WidgetDisabled) {
				g->setColor(UColor::gray);
			} else {
				g->setColor(UColor::black);
			}
			if (widgetState & WidgetSelected) {
				//g->setLineWidth(2);
				g->drawLine(rect.x + 2, rect.y + 2, rect.x + rect.w - 2, rect.y + rect.h - 2);
				g->drawLine(rect.x + 3, rect.y + 2, rect.x + rect.w - 1, rect.y + rect.h - 2);
				g->drawLine(rect.x + 2, rect.y + rect.h - 3, rect.x + rect.w - 2, rect.y + 1);
				g->drawLine(rect.x + 3, rect.y + rect.h - 3, rect.x + rect.w - 1, rect.y + 1);
				//g->setLineWidth(1);
			}
		break;
		case PE_IndicatorRadioButtonMask:
			if (widgetState & WidgetDisabled) {
				g->setColor(UColor::gray);
			} else {
				g->setColor(UColor::black);
			}
			if (widgetState & WidgetSelected) {
				UVertexArray array;
				array.add(rect.x + 4, rect.y + 2);
				array.add(rect.x + 2, rect.y + 4);
				array.add(rect.x + 2, rect.y + 6);
				array.add(rect.x + 4, rect.y + 8);
				array.add(rect.x + 6, rect.y + 8);
				array.add(rect.x + 8, rect.y + 6);
				array.add(rect.x + 8, rect.y + 4);
				array.add(rect.x + 6, rect.y + 2);
				array.add(rect.x + 4, rect.y + 2);
				g->drawVertexArray(UGraphics::LineStrip, &array);
				g->drawVertexArray(UGraphics::TriangleFan, &array);
			}
		break;
		case PE_IndicatorButtonDropDown:
			paintPrimitive(g, PE_IndicatorArrowDown, rect, hints, widgetState);
		break;
		case PE_TextCaret: {
			g->setColor(UColor::red);
			UVertexArray array;
			array.add(rect.x/* - 1*/, rect.y);
			array.add(rect.x/* - 1*/, rect.y + rect.h - 1);
			array.add(rect.x - 3, rect.y);
			array.add(rect.x + 2, rect.y);
			array.add(rect.x - 3, rect.y + rect.h - 1);
			array.add(rect.x + 2, rect.y + rect.h - 1);
			g->drawVertexArray(UGraphics::Lines, &array);
		}
		break;
		case PE_PanelButtonBevel:
		case PE_PanelButtonTool:
			if (hints->opacity) {
				UColor col(hints->palette.background());
				if (widgetState & (WidgetPressed | WidgetSelected)) {
					col = col.darker();
				}
				col.getFloat()[3] = hints->opacity;
				g->setColor(col.brighter());
				// don't paint edges (frame)
				g->drawLine(rect.x + 2, rect.y + 1, rect.x + rect.w - 2, rect.y + 1);

				UVertexArray bg;
				bg.setColor(col.brighter());
				bg.add(rect.x + 1, rect.y + 1);
				bg.add(rect.x + rect.w, rect.y + 1);
				bg.setColor(col);
				bg.add(rect.x + rect.w, rect.y + rect.h - 1);
				bg.add(rect.x + 1, rect.y + rect.h - 1);
				g->drawVertexArray(UGraphics::TriangleFan, &bg);

				g->setColor(col);
				//g->fillRect(rect.x + 1, rect.y + 2, rect.w - 2, rect.h - 4);
				g->drawLine(rect.x + 2, rect.y + rect.h - 1, rect.x + rect.w - 2, rect.y + rect.h - 1);
			}
			if (widgetState & (WidgetSelected | WidgetHasMouseFocus)) {
				UColor col(hints->palette.highlight());
				col.getFloat()[3] = hints->opacity;
				g->setColor(col);
				g->drawLine(rect.x + 3, rect.y + 1, rect.x + rect.w - 3, rect.y + 1);
				g->drawLine(rect.x + 3, rect.y + rect.h - 2, rect.x + rect.w - 3, rect.y + rect.h - 2);
			}
		break;
		case PE_PanelWidget:
			if (hints->opacity) {
				if (hints->background) {
					hints->background->paintDrawable(g, rect.x, rect.y, rect.w, rect.h);
				} else {
					UColor col(hints->palette.background());
					col.getFloat()[3] = hints->opacity;
					g->setColor(col);
					g->fillRect(rect.x, rect.y, rect.w, rect.h);
				}
			}
		break;
		case PE_FrameDefaultButton:
		case PE_FrameButtonBevel:
		if (hints->opacity) {
			g->setColor(hints->palette.background().darker());
			UVertexArray array;
			array.add(rect.x + 2, rect.y);
			array.add(rect.x + rect.w - 3, rect.y);
			array.add(rect.x + rect.w - 1, rect.y + 2);
			array.add(rect.x + rect.w - 1, rect.y + rect.h - 3);
			array.add(rect.x + rect.w - 3, rect.y + rect.h - 1);
			array.add(rect.x + 2, rect.y + rect.h - 1);
			array.add(rect.x, rect.y + rect.h - 3);
			array.add(rect.x, rect.y + 2);
			array.add(rect.x + 2, rect.y);
			g->drawVertexArray(UGraphics::LineStrip, &array);
		}
		break;
		case PE_FrameTabWidget:
		break;
		case PE_FrameStatusBar:
		case PE_FrameLineEdit:
		case PE_Frame:
		case PE_FrameMenu:
			g->setColor(hints->palette.background().darker());
			g->drawRect(rect);
		break;
		case PE_FrameWindow:
			g->setColor(UColor::black);
			g->drawRect(rect);

			g->setColor(hints->palette.background());
			g->drawRect(rect - UInsets(1, 1, 1, 1));

			g->setColor(UColor::darkGray);
			g->drawLine(rect.x + 2, rect.y + 2, rect.x + 2, rect.y + rect.h - 3);
			g->drawLine(rect.x + 2, rect.y + rect.h - 3, rect.x + rect.w - 3, rect.y + rect.h - 3);
			g->drawLine(rect.x + rect.w - 3, rect.y + rect.h - 3, rect.x + rect.w - 3, rect.y + 2);
		break;

		case PE_FrameFocusRect: {
			g->setColor(hints->palette.foreground());
			// mimic line stipple
			int stipple_length = 2;
			UVertexArray array;
			int i = 1;
			// top h line
			for (; i < rect.w; i += stipple_length) {
				array.add(rect.x + i, rect.y);
			}
			i = (i - rect.w) % stipple_length;
			// right v line
			for (; i < rect.h; i += stipple_length) {
				array.add(rect.x + rect.w - 1, rect.y + i);
			}
			i = (i - rect.h) % stipple_length;
			// bottom h line
			for (; i < rect.w; i += stipple_length) {
				array.add(rect.x + + rect.w - 1 - i, rect.y + rect.h - 1);
			}
			i = (i - rect.w) % stipple_length;
			// left v line
			for (; i < rect.h; i += stipple_length) {
				array.add(rect.x, rect.y + rect.h - 1 - i);
			}
			g->drawVertexArray(UGraphics::Lines, &array);

			//g->drawRect(rect);

		#if 0
			// paint dots, not a drawn through line
			ugl_driver->glPushAttrib(GL_LINE_STIPPLE);

			GLushort pattern = 0x3333;
			ugl_driver->glLineStipple(1, pattern);
			ugl_driver->glEnable(GL_LINE_STIPPLE);

			ugl_driver->glBegin(GL_LINE_LOOP);

			ugl_driver->glColor3fv(hints->palette.foreground().getFloat());

			ugl_driver->glVertex2i(rect.x - 1, rect.y - 1);
			ugl_driver->glVertex2i(rect.x - 1, rect.y + rect.h);// + 1);
			ugl_driver->glVertex2i(rect.x + rect.w/* + 1*/, rect.y + rect.h);// + 1);
			ugl_driver->glVertex2i(rect.x + rect.w/* + 1*/, rect.y - 1);

			ugl_driver->glEnd();

			ugl_driver->glPopAttrib();
			#endif
		}
		break;
		case PE_PanelToolBar:
			// FIXME: not yet implemented
		break;
		default:
		break;
	}
}

#if 0
void
paintSolidLine(int x1, int y1, int x2, int y2, int width) {
	ugl_driver->glLineWidth(width);
	ugl_driver->glBegin(GL_LINES);
	ugl_driver->glVertex2i(x1, y1);
	ugl_driver->glVertex2i(x2, y2);
	ugl_driver->glEnd();
}

void
paintDottedLine(int x1, int y1, int x2, int y2, int width) {
	ugl_driver->glLineWidth(width);
	// paint dots, not a drawn through line
	GLushort pattern = 0x3333;
	ugl_driver->glLineStipple(1, pattern);
	ugl_driver->glEnable(GL_LINE_STIPPLE);

	ugl_driver->glBegin(GL_LINES);
	ugl_driver->glVertex2i(x1, y1);
	ugl_driver->glVertex2i(x2, y2);
	ugl_driver->glEnd();
	ugl_driver->glDisable(GL_LINE_STIPPLE);
}

void
paintDashedLine(int x1, int y1, int x2, int y2, int width) {
	ugl_driver->glLineWidth(width);
	// paint dots, not a drawn through line
	GLushort pattern = 0x3333;
	ugl_driver->glLineStipple(1, pattern);
	ugl_driver->glEnable(GL_LINE_STIPPLE);

	ugl_driver->glBegin(GL_LINES);
	ugl_driver->glVertex2i(x1, y1);
	ugl_driver->glVertex2i(x2, y2);
	ugl_driver->glEnd();
	ugl_driver->glDisable(GL_LINE_STIPPLE);
}


void
paintStyledLineH(int x1, int x2, int y, int width, uint32_t style,
		const UColor & color) {
	ugl_driver->glPushAttrib(GL_LINE_BIT);
	switch (style) {
		case NoBorderStyle:
		break;
		case BorderSolid:
			ugl_driver->glColor4fv(color.getFloat());
			paintSolidLine(x1, y, x2, y, width);
		break;
		case BorderDotted:
			ugl_driver->glColor4fv(color.getFloat());
			paintDottedLine(x1, y, x2, y, width);
		break;
		case BorderDashed:
			ugl_driver->glColor4fv(color.getFloat());
			paintDashedLine(x1, y, x2, y, width);
		break;
		case BorderDouble:
			ugl_driver->glColor4fv(color.getFloat());
			paintSolidLine(x1, y, x2, y, 1);
			paintSolidLine(x1, y + width - 1, x2, y + width - 1, 1);
		break;
		case BorderGroove:
			ugl_driver->glColor4fv(color.getFloat());
			paintSolidLine(x1, y, x2, y, width / 2);
			ugl_driver->glColor4fv(color.brighter().getFloat());
			paintSolidLine(x1, y + width / 2 - 1, x2, y + width / 2 - 1, (width + 1) / 2);
		break;
		case BorderRidge:
			ugl_driver->glColor4fv(color.brighter().getFloat());
			paintSolidLine(x1, y, x2, y, width / 2);
			ugl_driver->glColor4fv(color.getFloat());
			paintSolidLine(x1, y + width / 2 - 1, x2, y + width / 2 - 1, (width + 1) / 2);
		break;
		case BorderInset:
		case BorderOutset:
			paintSolidLine(x1, y, x2, y, width);
			paintSolidLine(x1, y, x2, y, width);
		break;
		break;
	}
	ugl_driver->glPopAttrib();
}

void
paintStyledLineV(int y1, int y2, int x, int width, uint32_t style,
		const UColor & color) {
	ugl_driver->glPushAttrib(GL_LINE_BIT);
	switch (style) {
		case NoBorderStyle:
		break;
		case BorderSolid:
			ugl_driver->glColor4fv(color.getFloat());
			paintSolidLine(x, y1, x, y2, width);
		break;
		case BorderDotted:
			ugl_driver->glColor4fv(color.getFloat());
			paintDottedLine(x, y1, x, y2, width);
		break;
		case BorderDashed:
			ugl_driver->glColor4fv(color.getFloat());
			paintDashedLine(x, y1, x, y2, width);
		break;
		case BorderDouble:
			ugl_driver->glColor4fv(color.getFloat());
			paintSolidLine(x, y1, x, y2, 1);
			paintSolidLine(x + width - 1, y1, x + width - 1, y2, 1);
		break;
		case BorderGroove:
			ugl_driver->glColor4fv(color.getFloat());
			paintSolidLine(x, y1, x, y2, width / 2);
			ugl_driver->glColor4fv(color.brighter().getFloat());
			paintSolidLine(x + width / 2 - 1, y1, x + width / 2 - 1, y2, (width + 1) / 2);
		break;
		case BorderRidge:
			ugl_driver->glColor4fv(color.brighter().getFloat());
			paintSolidLine(x, y1, x, y2, width / 2);
			ugl_driver->glColor4fv(color.getFloat());
			paintSolidLine(x + width / 2 - 1, y1, x + width / 2 - 1, y2, (width + 1) / 2);
		break;
		case BorderInset:
		case BorderOutset:
			paintSolidLine(x, y1, x, y2, width);
			paintSolidLine(x, y1, x, y2, width);
		break;
		break;
	}
	ugl_driver->glPopAttrib();
}
#endif

void
UBasicStyle::paintBorder(UGraphics * g,
		uint32_t borderType,
		const URectangle & rect,
		const UStyleHints * hints,
		uint32_t widgetState)
{
#if 0
	if (borderType == CssBorder) {
	paintStyledLineH(
		rect.x, rect.x + rect.w - 1,
		rect.y,
		hints->border->width[0],
		hints->border->style[0],
		hints->border->color[0]
	);

	paintStyledLineV(
		rect.y, rect.y + rect.h - 1,
		rect.x + rect.w - 1,
		hints->border->width[1],
		hints->border->style[1],
		hints->border->color[1]
	);

	paintStyledLineH(
		rect.x, rect.x + rect.w - 1,
		rect.y + rect.h - 1,
		hints->border->width[2],
		hints->border->style[2],
		hints->border->color[2]
	);

	paintStyledLineV(
		rect.y, rect.y + rect.h - 1,
		rect.x,
		hints->border->width[3],
		hints->border->style[3],
		hints->border->color[3]
	);
	} else
#endif
	if (borderType == LineBorder) {
		g->setColor(hints->border->color[0]);
		g->drawRect(rect);
	} else if (borderType == BottomLineBorder) {
		g->setColor(hints->border->color[0]);
		g->fillRect(rect.x, rect.y + rect.h - hints->border->width[0] - 1, rect.w, rect.y + rect.h - 1);
		/*
		ugl_driver->glRecti(
			rect.x, rect.y + rect.h - hints->border->width[0] - 1,
			rect.x + rect.w, rect.y + rect.h - 1
		);
		*/
#if 0
	} else if (borderType == UBorderModel::ControlBorder) {
		g->setColor(hints->border->color[0]);
		ugl_driver->glBegin(GL_LINE_LOOP);
		ugl_driver->glVertex2i(rect.x + 2, rect.y);
		ugl_driver->glVertex2i(rect.x + rect.w - 3, rect.y);
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + 2);
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + rect.h - 3);
		ugl_driver->glVertex2i(rect.x + rect.w - 3, rect.y + rect.h - 1);
		ugl_driver->glVertex2i(rect.x + 2, rect.y + rect.h - 1);
		ugl_driver->glVertex2i(rect.x, rect.y + rect.h - 3);
		ugl_driver->glVertex2i(rect.x, rect.y + 2);
		ugl_driver->glEnd();
#endif
	}
}

UInsets
UBasicStyle::getInsets(
		ComponentElement elem,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w)
{
	UInsets ret(hints->margin);

	// add border insets
	switch (hints->border->borderType) {
		case NoBorder:
		break;
		case LineBorder:
			ret += UInsets(hints->border->width[0], hints->border->width[0],
			hints->border->width[0], hints->border->width[0]);
		break;
		case BottomLineBorder:
			ret += UInsets(0, 0, hints->border->width[0], 0);
		break;
	}
	switch (elem) {
		case CE_Button:
			// focus rect and border
			return ret + UInsets(3, 3, 3, 3) + UInsets(2, 2, 2, 2);
		break;
		case CE_MenuBarItem:
			// plus border
			return ret + UInsets(2, 2, 2, 2) + UInsets(2, 2, 2, 2);
		break;
		case CE_RadioButton:
		case CE_CheckBox:
			if ((static_cast<const UCompoundModel*>(model))->icon == NULL) {
				// add icon
				if (hints->orientation == Vertical) {
					return ret + UInsets(4 + 12, 0, 0, 0);
				} else  {
					return ret + UInsets(0, 4 + 12, 0, 0);
				}
			}
			return ret;
		break;
		case CE_ComboBox:
			// arrow
			return ret + UInsets(0, 0, 0, 20);
		break;
		case CE_SpinBox:
			// arrows
			return ret + UInsets(0, 0, 0, 16);
		break;
		case CE_Slider:
			if (hints->orientation == Vertical) {
				return ret + UInsets(0, 0, 0, 14);
			} else {
				return ret + UInsets(14, 0, 0, 0);
			}
		break;
		case CE_MenuItem: {
			const UMenuItemModel * item = static_cast<const UMenuItemModel*>(model);
			if (item->buttonFeatures & UMenuItemModel::HasMenu) {
				ret.right += 12;
			}
			// add always space for menu icon
			//if (item->icon == NULL && item->checkType != UMenuItemModel::NotCheckable) {
				ret.left += 4 + 12;
			//}
			// add a 2 pixel extra space as margin
			//ret += UInset(2, 2, 2, 2);
			return ret;
		}
		break;
		case CE_GroupBox: {
			//const UGroupBoxModel * item = static_cast<const UGroupBoxModel*>(model);
			// + border and text
			//ret += UInsets(20, 3, 3, 3);
			return ret + UInsets(20, 3, 3, 3);
		}
		break;
		case CE_InternalFrame:
			// plus border
			return ret + UInsets(2, 3, 3, 3);
		break;
		case CE_TabBarTab:
			// plus border
			return ret + UInsets(3, 3, 2, 3);
		break;
		default:
			return ret;
		break;
	}
	return ret;
}

void
UBasicStyle::paintComponent(UGraphics * g,
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w)
{
	UInsets insets(getInsets(elem, hints, model));//hints->margin + getBorderInsets(elem, hints));
	switch (elem) {
		case  CE_Widget:
			paintPrimitive(g, PE_PanelWidget, rect, hints, model->widgetState);
		break;
		case  CE_Compound: {
			const UCompoundModel * compound = static_cast<const UCompoundModel*>(model);
			paintCompound(g, hints, compound->text, compound->icon, rect, model->widgetState, compound->acceleratorIndex);
		}
		break;
		case  CE_Label: {
			paintPrimitive(g, PE_PanelWidget, rect, hints, model->widgetState);
			paintComponent(g, CE_Compound, rect - insets, hints, model);
		}
		break;
		case  CE_Button: {
			paintPrimitive(g, PE_PanelButtonBevel, rect, hints, model->widgetState);
			if (model->widgetState & WidgetPressed) {
				g->translate(1, 1);
			}
			paintComponent(g, CE_Compound, rect - insets/* - UInsets(3, 3, 3, 3)*/, hints, model);
			if (model->widgetState & WidgetPressed) {
				g->translate(-1, -1);
			}
			paintPrimitive(g, PE_FrameButtonBevel, rect, hints, model->widgetState);
			if (model->widgetState & WidgetHasFocus) {
				paintPrimitive(g, PE_FrameFocusRect, rect - UInsets(3, 3, 3, 3), hints, model->widgetState);
			}
		}
		break;
		case  CE_CheckBox: {
			//paintPrimitive(g, PE_PanelButtonBevel, rect, hints, model->widgetState);
			paintPrimitive(g, PE_PanelWidget, rect, hints, model->widgetState);
			//const UCompoundModel * compound = static_cast<const UCompoundModel*>(model);
			URectangle icon_rect(0, rect.h / 2 - 6, 12, 12);
			//if (compound->icon == NULL) {
				paintPrimitive(g, PE_IndicatorCheckBox, icon_rect, hints, model->widgetState);
				paintPrimitive(g, PE_IndicatorCheckBoxMask, icon_rect, hints, model->widgetState);
			//}
			URectangle compound_rect(12+4, 0, rect.w - 12 - 4, rect.h);
			paintComponent(g, CE_Compound, compound_rect, hints, model);
		}
		break;
		case  CE_RadioButton: {
			//paintPrimitive(g, PE_PanelButtonBevel, rect, hints, model->widgetState);
			//const UCompoundModel * compound = static_cast<const UCompoundModel*>(model);
			paintPrimitive(g, PE_PanelWidget, rect, hints, model->widgetState);
			URectangle icon_rect(0, rect.h / 2 - 6, 12, 12);
			//if (compound->icon == NULL) {
				paintPrimitive(g, PE_IndicatorRadioButton, icon_rect, hints, model->widgetState);
				paintPrimitive(g, PE_IndicatorRadioButtonMask, icon_rect, hints, model->widgetState);
			//}
			URectangle compound_rect(12+4, 0, rect.w - 12 - 4, rect.h);
			paintComponent(g, CE_Compound, compound_rect, hints, model);
		}
		break;
		case  CE_MenuItem: {
			const UMenuItemModel * menuItem = static_cast<const UMenuItemModel*>(model);
			//paintPrimitive(g, PE_PanelWidget, rect, hints, model->widgetState);
			if (menuItem->widgetState & WidgetHasMouseFocus ||
				menuItem->widgetState & WidgetHighlighted) {

				UVertexArray bg;
				bg.setColor(hints->palette.highlight().brighter());
				bg.add(rect.x, rect.y);
				bg.add(rect.x + rect.w, rect.y);
				bg.setColor(hints->palette.highlight());
				bg.add(rect.x + rect.w, rect.y + rect.h);
				bg.add(rect.x, rect.y + rect.h);
				g->drawVertexArray(UGraphics::TriangleFan, &bg);

				//g->setColor(hints->palette.highlight());
				//g->fillRect(rect);
			}
			URectangle icon_rect(0, rect.h / 2 - 6, 12, 12);
			URectangle compound_rect(rect - insets);
			// add always space for icon
			//compound_rect -= UInsets(0, 16, 0, 0);
			if (menuItem->icon == NULL && menuItem->checkType == UMenuItemModel::Exclusive) {
				paintPrimitive(g, PE_IndicatorRadioButton, icon_rect, hints, model->widgetState);
				paintPrimitive(g, PE_IndicatorRadioButtonMask, icon_rect, hints, model->widgetState);
			} else if (menuItem->icon == NULL && menuItem->checkType == UMenuItemModel::NonExclusive) {
				paintPrimitive(g, PE_IndicatorCheckBox, icon_rect, hints, model->widgetState);
				paintPrimitive(g, PE_IndicatorCheckBoxMask, icon_rect, hints, model->widgetState);
			}
			paintComponent(g, CE_Compound, compound_rect, hints, model);
			if (menuItem->buttonFeatures & UMenuItemModel::HasMenu) {
				URectangle menuArrowRect(rect.w - 8, rect.y + 4, 8, 8);
				paintPrimitive(g, PE_IndicatorArrowRight, menuArrowRect, hints, menuItem->widgetState);
			}
		}
		break;
		case  CE_MenuBarItem: {
			//paintPrimitive(g, PE_ControlBackground, rect, hints, model);
			if ((model->widgetState & WidgetHighlighted ||
				model->widgetState & WidgetHasMouseFocus)) {
				paintPrimitive(g, PE_PanelButtonBevel, rect, hints, model->widgetState);
				paintPrimitive(g, PE_FrameButtonBevel, rect, hints, model->widgetState);
			}
			/*if (model->widgetState & WidgetPressed) {
				g->setColor(hints->palette.foreground());
				g->fillRect(rect);
			}*/
			paintComponent(g, CE_Compound, rect - insets, hints, model);
		}
		break;
		case  CE_MenuTearOff:
		break;
		case  CE_ListBox:
			paintPrimitive(g, PE_PanelWidget, rect, hints,  model->widgetState);
		break;
		case  CE_LineEdit:
		case  CE_TextEdit:
		case  CE_TextWidget: {
			const UTextModel * textModel = static_cast<const UTextModel*>(model);
			if (hints->opacity) {
				UColor col = (textModel->widgetState & WidgetEditable) ?
					hints->palette.base() : hints->palette.background();
				col.getFloat()[3] = hints->opacity;
				g->setColor(col);
				g->fillRect(rect.x, rect.y, rect.w, rect.h);
			}
			UFont font = hints->font;

			// translate using the widget's insets
			g->translate(insets.left, insets.top);

			UCaret * caret = textModel->document->getCaret();
			URectangle pos_rect = textModel->textLayout->modelToView(caret->getPosition());
			// paint text selection
			if (caret->getPosition() != caret->getMark()) {
				URectangle mark_rect =  textModel->textLayout->modelToView(caret->getMark());
				unsigned int lineHeight = hints->font.getFontMetrics()->getLineskip();

				g->setColor(hints->palette.highlight());
				if (pos_rect.y == mark_rect.y) {
					unsigned int minx = std::min(pos_rect.x, mark_rect.x);
					unsigned int maxx = std::max(pos_rect.x, mark_rect.x);

					g->fillRect(minx, pos_rect.y,
						maxx - minx - 1, lineHeight);
				} else {
					UDimension size = rect.getSize() - insets;
					UPoint min, max;

					if (pos_rect.y < mark_rect.y) {
						min = pos_rect.getLocation();
						max = mark_rect.getLocation();
					} else  {
						min = mark_rect.getLocation();
						max = pos_rect.getLocation();
					}

					g->fillRect(min.x, min.y, size.w - min.x, lineHeight);
					g->fillRect(0, min.y + lineHeight, size.w, max.y - min.y - lineHeight);
					g->fillRect(0, max.y, max.x, lineHeight);
				}
			}


			g->setColor(hints->palette.text());

			// FIXME: move caret to visible rect
			UPoint offset;
			if (pos_rect.x /*+ pos_rect.w*/ > rect.x + rect.w) {
				offset.x = rect.x + rect.w - pos_rect.x - 10;
			}
			if (pos_rect.y + pos_rect.h > rect.y + rect.h) {
				offset.y = rect.y + rect.h - pos_rect.y - font.getFontMetrics()->getLineskip();
			}
			textModel->textLayout->render(g, rect - insets, offset);

			if (textModel->widgetState & WidgetHasFocus) {
				paintPrimitive(g, PE_TextCaret,
					pos_rect + offset, hints, model->widgetState);
			}
			g->translate(-insets.left, -insets.top);
		}
		break;
		case  CE_ScrollBar: {
			const USliderModel * sliderModel = static_cast<const USliderModel*>(model);
			// draw sub controls
			// sub line first
			if (sliderModel->subControls & SC_ScrollBarSubLine) {
				uint32_t state = sliderModel->widgetState;
				state &= ~(WidgetPressed | WidgetHasMouseFocus);
				URectangle sc_rect(getSubControlBounds(elem, rect,
					hints, model, SC_ScrollBarSubLine)
				);
				// FIXME: add mouse focus?
				if (sliderModel->activeSubControls & SC_ScrollBarSubLine) {
					state |= WidgetPressed;
				}
				paintPrimitive(g, PE_PanelButtonBevel, sc_rect, hints, state);
				if (hints->orientation == Vertical) {
					paintPrimitive(g, PE_IndicatorArrowUp, sc_rect, hints, state);
				} else {
					paintPrimitive(g, PE_IndicatorArrowLeft, sc_rect, hints, state);
				}
				paintPrimitive(g, PE_FrameButtonBevel, sc_rect, hints, state);
			}
			// add line
			if (sliderModel->subControls & SC_ScrollBarAddLine) {
				uint32_t state = sliderModel->widgetState;
				state &= ~(WidgetPressed | WidgetHasMouseFocus);
				URectangle sc_rect(getSubControlBounds(elem, rect,
					hints, model, SC_ScrollBarAddLine)
				);
				// FIXME: add mouse focus?
				if (sliderModel->activeSubControls & SC_ScrollBarAddLine) {
					state |= WidgetPressed;
				}
				paintPrimitive(g, PE_PanelButtonBevel, sc_rect, hints, state);
				if (hints->orientation == Vertical) {
					paintPrimitive(g, PE_IndicatorArrowDown, sc_rect, hints, state);
				} else {
					paintPrimitive(g, PE_IndicatorArrowRight, sc_rect, hints, state);
				}
				paintPrimitive(g, PE_FrameButtonBevel, sc_rect, hints, state);
			}
			// sub page
			if (sliderModel->subControls & SC_ScrollBarSubPage) {
				URectangle sc_rect(getSubControlBounds(elem, rect,
					hints, model, SC_ScrollBarSubPage)
				);
				if (sliderModel->activeSubControls & SC_ScrollBarSubPage) {
					g->setColor(hints->palette.background().darker());
				} else {
					g->setColor(hints->palette.background().brighter());
				}
				g->fillRect(sc_rect);
			}

			// add page
			if (sliderModel->subControls & SC_ScrollBarAddPage) {
				URectangle sc_rect(getSubControlBounds(elem, rect,
					hints, model, SC_ScrollBarAddPage)
				);
				if (sliderModel->activeSubControls & SC_ScrollBarAddPage) {
					g->setColor(hints->palette.background().darker());
				} else {
					g->setColor(hints->palette.background().brighter());
				}
				g->fillRect(sc_rect);
			}
			// slider knob
			if (sliderModel->subControls & SC_ScrollBarSlider) {
				URectangle slider_rect(getSubControlBounds(elem, rect,
					hints, model, SC_ScrollBarSlider)
				);
				if (sliderModel->activeSubControls & SC_ScrollBarSlider) {
					g->setColor(hints->palette.highlight().brighter());
				} else {
					g->setColor(hints->palette.highlight());
				}
				g->fillRect(slider_rect);
				paintPrimitive(g, PE_Gripper, slider_rect - UInsets(2, 0, 2, 0), hints, model->widgetState);
				g->setColor(hints->palette.highlight().darker());
				g->drawRect(slider_rect);
			}
		}
		break;
		case  CE_Separator:
			if (hints->orientation == Vertical) {
				g->setColor(hints->palette.background().darker());
				g->drawLine(rect.x, rect.y, rect.x, rect.y + rect.h);

				g->setColor(hints->palette.background().darker().darker());
				g->drawLine(rect.x + 1, rect.y + 1, rect.x + 1, rect.y + rect.h);
			} else {
				g->setColor(hints->palette.background().darker());
				g->drawLine(rect.x, rect.y, rect.x + rect.w, rect.y);

				g->setColor(hints->palette.background().darker().darker());
				g->drawLine(rect.x + 1, rect.y + 1, rect.x + rect.w, rect.y + 1);
			}
		break;
		case  CE_ProgressBar: {
			const UProgressBarModel * pModel = static_cast<const UProgressBarModel*>(model);

			int width = int(rect.w * float(pModel->value) /
				(pModel->maximum - pModel->minimum)
			);
			//g->setColor(hints->palette.highlight());
			//g->fillRect(rect.x, rect.y, width, rect.h);
			UVertexArray bg;
			bg.setColor(hints->palette.highlight().darker());
			bg.add(rect.x, rect.y);
			bg.add(rect.x + width, rect.y);
			bg.setColor(hints->palette.highlight().brighter());
			bg.add(rect.x + width, rect.y + rect.h);
			bg.add(rect.x, rect.y + rect.h);
			g->drawVertexArray(UGraphics::TriangleFan, &bg);

			paintBorder(g, LineBorder, rect, hints, model->widgetState);
			if (pModel->textVisible) {
				g->setColor(hints->palette.text());
				int stringWidth = hints->font.getFontMetrics()
					->getStringWidth(pModel->text);
				g->drawString(
					pModel->text,
					rect.x + rect.w / 2 - stringWidth / 2,
					rect.y
				);
			}
		}
		break;
		case  CE_ProgressBarGroove:
		break;
		case  CE_ProgressBarContents:
		break;
		case  CE_Slider: {
			const USliderModel * sliderModel = static_cast<const USliderModel*>(model);
			paintPrimitive(g, PE_PanelWidget, rect, hints, model->widgetState);

			// slider groove
			if (sliderModel->subControls & SC_SliderGroove) {
/*
				UPoint left(5, rect.h / 2);
				UPoint right(rect.w - 5, left.y);
				g->setColor(hints->palette.dark());
				g->drawLine(left, right);
*/
				int mid_y = rect.h / 2;
				g->setColor(hints->palette.background().darker());
				g->drawRect(URectangle(0, mid_y - 2, rect.w, 4));
			}

			// slider knob
			if (sliderModel->subControls & SC_SliderHandle) {
				URectangle slider_rect(getSubControlBounds(elem, rect,
					hints, model, SC_SliderHandle)
				);
				/*
				if (sliderModel->activeSubControls & SC_SliderHandle) {
					g->setColor(hints->palette.highlight().brighter());
				} else {
					g->setColor(hints->palette.highlight());
				}
				g->fillRect(slider_rect);
				g->setColor(hints->palette.highlight().darker());
				g->drawRect(slider_rect);
				*/
				paintPrimitive(g, PE_PanelButtonBevel, slider_rect, hints, model->widgetState);
				paintPrimitive(g, PE_FrameButtonBevel, slider_rect, hints, model->widgetState);

				paintPrimitive(g, PE_Gripper, slider_rect - UInsets(2, 1, 2, 1), hints, model->widgetState);
			}
		}
		break;
		case CE_ComboBox: {
			URectangle arrow_rect(getSubControlBounds(
				elem,
				rect,
				hints,
				model,
				SC_ComboBoxArrow));
			int state = model->widgetState;
			//state &= ~(WidgetPressed | WidgetHasMouseFocus);
			//if (sliderModel->activeSubControls & SC_ComboBoxArrow) {
			//	state |= WidgetPressed;
			//}
			paintPrimitive(g, PE_PanelButtonBevel, arrow_rect, hints, state);
			paintPrimitive(g, PE_IndicatorArrowDown, arrow_rect, hints, state);
			paintPrimitive(g, PE_FrameButtonBevel, arrow_rect, hints, state);
		}
		break;
		case CE_SpinBox: {
			const USpinBoxModel * spinBox = static_cast<const USpinBoxModel*>(model);
			URectangle arrow_up_rect(getSubControlBounds(
				elem,
				rect,
				hints,
				model,
				SC_SpinBoxUp));
			int state = model->widgetState;
			state &= ~(WidgetPressed | WidgetHasMouseFocus);
			if (spinBox->activeSubControls & SC_SpinBoxUp) {
				state |= WidgetPressed;
			}
			paintPrimitive(g, PE_PanelButtonBevel, arrow_up_rect, hints, state);
			paintPrimitive(g, PE_IndicatorArrowUp, arrow_up_rect, hints, state);
			//paintPrimitive(g, PE_FrameButtonBevel, arrow_up_rect, hints, state);

			URectangle arrow_down_rect(getSubControlBounds(
				elem,
				rect,
				hints,
				model,
				SC_SpinBoxDown));
			state = model->widgetState;
			state &= ~(WidgetPressed | WidgetHasMouseFocus);
			if (spinBox->activeSubControls & SC_SpinBoxDown) {
				state |= WidgetPressed;
			}
			paintPrimitive(g, PE_PanelButtonBevel, arrow_down_rect, hints, state);
			paintPrimitive(g, PE_IndicatorArrowDown, arrow_down_rect, hints, state);
			//paintPrimitive(g, PE_FrameButtonBevel, arrow_down_rect, hints, state);
		}
		break;
		case  CE_TitleBar: {
			const UTitleBarModel * titleBar = static_cast<const UTitleBarModel*>(model);
			if (titleBar->frameState & FrameActive) {
				g->setColor(hints->palette.highlight());
			} else {
				g->setColor(hints->palette.background());
			}

			g->fillRect(rect.x, rect.y, rect.w, rect.h);

			// draw a small border line
			g->setColor(hints->palette.background().darker());
			g->drawLine(
				rect.x, rect.y + rect.h - 1,
				rect.x + rect.w - 1, rect.y + rect.h - 1
			);

			// draw title

			// subcontrols
			// titlebar
			if (titleBar->subControls & SC_TitleBarLabel) {
				URectangle label_rect(getSubControlBounds(elem, rect,
					hints, model, SC_TitleBarLabel)
				);
				//if (titleBar->frameStyle & FrameTitleBar) {
					g->setColor(hints->palette.foreground());
					g->drawString(titleBar->text, label_rect.x, label_rect.y);
				//}
			}

			// close
			if (titleBar->subControls & SC_TitleBarCloseButton) {
				URectangle close_rect(getSubControlBounds(elem, rect,
					hints, model, SC_TitleBarCloseButton)
				);
				int state = model->widgetState;
				state &= ~(WidgetPressed | WidgetHasMouseFocus);
				if (titleBar->activeSubControls & SC_TitleBarCloseButton) {
					state |= WidgetPressed;
				}

				paintPrimitive(g, PE_PanelButtonBevel, close_rect, hints, state);
				// FIXME: oops, add separate icon
				paintPrimitive(g, PE_IndicatorCheckBoxMask, close_rect, hints, state | WidgetSelected);
				paintPrimitive(g, PE_FrameButtonBevel, close_rect, hints, state);
			}
			// max
			if (titleBar->subControls & SC_TitleBarMaxButton) {
				URectangle max_rect(getSubControlBounds(elem, rect,
					hints, model, SC_TitleBarMaxButton)
				);
				int state = model->widgetState;
				state &= ~(WidgetPressed | WidgetHasMouseFocus);
				if (titleBar->activeSubControls & SC_TitleBarMaxButton) {
					state |= WidgetPressed;
				}
				paintPrimitive(g, PE_PanelButtonBevel, max_rect, hints, state);
				paintPrimitive(g, PE_IndicatorArrowUp, max_rect, hints, state);
				paintPrimitive(g, PE_FrameButtonBevel, max_rect, hints, state);
			}
			// min
			if (titleBar->subControls & SC_TitleBarMinButton) {
				URectangle min_rect(getSubControlBounds(elem, rect,
					hints, model, SC_TitleBarMinButton)
				);
				int state = model->widgetState;
				state &= ~(WidgetPressed | WidgetHasMouseFocus);
				if (titleBar->activeSubControls & SC_TitleBarMinButton) {
					state |= WidgetPressed;
				}
				paintPrimitive(g, PE_PanelButtonBevel, min_rect, hints, state);
				paintPrimitive(g, PE_IndicatorArrowDown, min_rect, hints, state);
				paintPrimitive(g, PE_FrameButtonBevel, min_rect, hints, state);
			}
			// sys menu
			if (titleBar->subControls & SC_TitleBarSysMenu && titleBar->icon) {
				URectangle menu_rect(getSubControlBounds(elem, rect,
					hints, model, SC_TitleBarSysMenu)
				);
				int state = model->widgetState;
				state &= ~(WidgetPressed | WidgetHasMouseFocus);
				if (titleBar->activeSubControls & SC_TitleBarMinButton) {
					state |= WidgetPressed;
				}
				titleBar->icon->paintIcon(g, menu_rect, hints, state);
			}
		}
		break;
		case  CE_InternalFrame:
			paintPrimitive(g, PE_FrameWindow, rect, hints, model->widgetState);
		break;
		case  CE_Splitter:
		break;
		case  CE_TabBarTab: {
			//paintPrimitive(g, PE_PanelButtonBevel, rect, hints, model->widgetState);

			if (hints->opacity) {
				UColor col(hints->palette.background());
				if (model->widgetState & (WidgetPressed | WidgetSelected)) {
					col = col.darker();
				}
				col.getFloat()[3] = hints->opacity;
				g->setColor(col.brighter());
				// don't paint edges (frame)
				g->drawLine(rect.x + 2, rect.y + 1, rect.x + rect.w - 2, rect.y + 1);

				UVertexArray bg;
				bg.setColor(col.brighter());
				bg.add(rect.x + 1, rect.y + 2);
				bg.add(rect.x + rect.w, rect.y + 2);
				bg.setColor(col);
				bg.add(rect.x + rect.w, rect.y + rect.h);
				bg.add(rect.x + 1, rect.y + rect.h);
				g->drawVertexArray(UGraphics::TriangleFan, &bg);
			}

			paintComponent(g, CE_Compound, rect - insets, hints, model);

			g->setColor(hints->palette.background().darker());
			UVertexArray array;
			if (model->widgetState & WidgetSelected) {
				array.add(rect.x, rect.y + rect.h);
				array.add(rect.x, rect.y + 2);
				array.add(rect.x + 2, rect.y);
				array.add(rect.x + rect.w - 3, rect.y);
				array.add(rect.x + rect.w - 1, rect.y + 2);
				array.add(rect.x + rect.w - 1, rect.y + rect.h);
			} else {
				array.add(rect.x, rect.y + rect.h);
				array.add(rect.x, rect.y + 2);
				array.add(rect.x + rect.w - 1, rect.y + 2);
				array.add(rect.x + rect.w - 1, rect.y + rect.h);
			}
			g->drawVertexArray(UGraphics::LineStrip, &array);
		}
		break;
		case CE_GroupBox: {
			const UGroupBoxModel * box = static_cast<const UGroupBoxModel*>(model);
			UDimension size = g->getStringSize(box->text);
			// or use clipping an PE_FrameButtonBevel
			g->setColor(hints->palette.background().darker());
			UVertexArray array;
			array.add(rect.x + 9 + size.w, rect.y + size.h/2);
			array.add(rect.x + rect.w - 3, rect.y + size.h/2);
			array.add(rect.x + rect.w - 1, rect.y + size.h/2 + 2);
			array.add(rect.x + rect.w - 1, rect.y + rect.h - 3);
			array.add(rect.x + rect.w - 3, rect.y + rect.h - 1);
			array.add(rect.x + 2, rect.y + rect.h - 1);
			array.add(rect.x, rect.y + rect.h - 3);
			array.add(rect.x, rect.y + size.h/2 + 2);
			array.add(rect.x + 2, rect.y + size.h/2);
			g->drawVertexArray(UGraphics::LineStrip, &array);
			g->setColor(hints->palette.foreground());
			g->drawString(box->text, 5, 0);
		}
		break;
		case CE_StaticText:
		case CE_ToolButton:
			// FIXME: not yet implemented
		break;
		default:
		break;
	}
}

UInsets
UBasicStyle::getBorderInsets(ComponentElement elem, const UStyleHints * hints)
{
	switch (hints->border->borderType) {
		case NoBorder:
		break;
		case LineBorder:
			return UInsets(hints->border->width[0], hints->border->width[0],
			hints->border->width[0], hints->border->width[0]);
		break;
		case BottomLineBorder:
			return UInsets(0, 0, hints->border->width[0], 0);
		break;
		case CssBorder:
			return UInsets(
				hints->border->width[0],
				hints->border->width[1],
				hints->border->width[2],
				hints->border->width[3]
			);
			break;
		case StyleBorder:
			switch (elem) {
				// control border
				case CE_Button:
				case CE_MenuBarItem:
					return UInsets(2, 2, 2, 2);
				break;
				case CE_InternalFrame:
					return UInsets(2, 3, 3, 3);
				break;
				default:
				break;
			}
		break;
		default:
		break;
	}
	return UInsets();
	//return UInsets(hints->border->width[0], hints->border->width[3],
	//	hints->border->width[2], hints->border->width[1]);
#if 0
	switch (borderType) {
		case NoBorder:
			break;
		case RaisedBevelBorder:
		case LoweredBevelBorder:
			return UInsets(2, 2, 2, 2);
			break;
		case LineBorder:
			return UInsets(1, 1, 1, 1);
			break;
		case TitledBorder: {
			int add = 0;
			/*UString * str = dynamic_cast<UString*>(w->get("border_title"));
			if (str) {
				add = hints->font.getFontMetrics()->getHeight();
			}*/
			return UInsets(1 + add, 1, 1, 1);
			}
			break;
		/*case BorderType(UIBorder):
			return getUIBorderInsets(w, borderType);
			break;*/
		default:
			break;
	}
#endif
	return UInsets();
}

UStyle::SubControls
UBasicStyle::getSubControlAt(
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		const UPoint & pos,
		UWidget * w)
{
	switch (elem) {
		case  CE_Slider: {
			URectangle rect = getSubControlBounds(elem, rect, hints, model, SC_SliderHandle);
			if (rect.contains(pos)) {
				return SC_SliderHandle;
			} else {
				return SC_SliderGroove;
			}
		}
		break;
		case  CE_ScrollBar: {
			URectangle sc_rect;
			unsigned int control = SC_ScrollBarFirst;
			while (control <= SC_ScrollBarLast) {
				sc_rect = getSubControlBounds(elem, rect, hints, model, UStyle::SubControls(control));
				if (sc_rect.contains(pos)) {
					return UStyle::SubControls(control);
				}
				control <<= 1;
			}
		}
		break;
		case  CE_SpinBox: {
			URectangle sc_rect;
			unsigned int control = SC_SpinBoxFirst;
			while (control <= SC_SpinBoxLast) {
				sc_rect = getSubControlBounds(elem, rect, hints, model, UStyle::SubControls(control));
				if (sc_rect.contains(pos)) {
					return UStyle::SubControls(control);
				}
				control <<= 1;
			}
		}
		break;
		case  CE_TitleBar: {
			URectangle sc_rect;
			unsigned int control = SC_TitleBarFirst;
			while (control <= SC_TitleBarLast) {
				sc_rect = getSubControlBounds(elem, rect, hints, model, UStyle::SubControls(control));
				if (sc_rect.contains(pos)) {
					return UStyle::SubControls(control);
				}
				control <<= 1;
			}
		}
		break;
		default:
		break;
	}
	return SC_None;
}

URectangle
UBasicStyle::getSubControlBounds(
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		SubControls subElem,
		UWidget * w)
{
	URectangle ret;
	switch (elem) {
		case CE_Slider: {
			const USliderModel * sliderModel = static_cast<const USliderModel*>(model);
			int width = 8;
			int height = 14;
			int maxlen = (hints->orientation == Vertical) ? rect.h : rect.w;
			int pos = basicstyle_sliderModelToView(
				sliderModel->minimum,
				sliderModel->maximum,
				sliderModel->sliderValue,
				maxlen - width);
			switch (subElem) {
				case  SC_SliderHandle: {
					if (hints->orientation == Vertical) {
						ret.setBounds(rect.x + rect.w / 2 - height / 2, pos,
							height, width);
					} else {
						ret.setBounds(pos, rect.y + rect.h / 2 - height / 2,
							width, height);
					}
				}
				break;
				default:
				break;
			}
		}
		break;
		case CE_ScrollBar: {
			const USliderModel * sliderModel = static_cast<const USliderModel*>(model);
			int width = 16;
			int maxlen = (hints->orientation == Vertical) ? rect.h : rect.w;
			maxlen -= 2*width;
			int range = sliderModel->maximum - sliderModel->minimum;
			int sliderlen = (sliderModel->blockIncrement * maxlen) / (range + sliderModel->blockIncrement);
			int sliderstart = width + basicstyle_sliderModelToView(
				sliderModel->minimum,
				sliderModel->maximum,
				sliderModel->sliderValue,
				maxlen - sliderlen);

			switch (subElem) {
				case  SC_ScrollBarSubLine:
					if (hints->orientation == Vertical) {
						ret.setBounds(0, 0, width, width);
					} else {
						ret.setBounds(0, 0, width, width);
					}
				break;
				case  SC_ScrollBarAddLine:
					if (hints->orientation == Vertical) {
						ret.setBounds(0, rect.h - width, width, width);
					} else {
						ret.setBounds(rect.w - width, 0, width, width);
					}
				break;
				case  SC_ScrollBarSubPage:
					if (hints->orientation == Vertical) {
						ret.setBounds(0, width, width, sliderstart - width);
					} else {
						ret.setBounds(width, 0, sliderstart - width, width);
					}
				break;
				case  SC_ScrollBarAddPage:
					if (hints->orientation == Vertical) {
						ret.setBounds(0, sliderstart + sliderlen,
							width, rect.h - sliderstart - sliderlen - width);
					} else {
						ret.setBounds(sliderstart + sliderlen, 0,
							rect.w - sliderstart - sliderlen - width, width);
					}
				break;
				case  SC_ScrollBarGroove:
					if (hints->orientation == Vertical) {
						ret.setBounds(0, width, width, rect.h - 2 * width);
					} else {
						ret.setBounds(width, 0, rect.w - 2 * width, width);
					}
				break;
				case  SC_ScrollBarSlider:
					if (hints->orientation == Vertical) {
						ret.setBounds(0, sliderstart, width, sliderlen);
					} else {
						ret.setBounds(sliderstart, 0, sliderlen, width);
					}
				break;
				default:
				break;
			}
		}
		break;
		case CE_ComboBox:
			switch (subElem) {
				case SC_ComboBoxEditField:
				break;
				case SC_ComboBoxArrow:
					ret.setBounds(rect.w - 16, 0, 16, 16);
				break;
				case SC_ComboBoxListBoxPopup:
				break;
				default:
				break;
			}
		break;
		case CE_SpinBox:
			switch (subElem) {
				case SC_SpinBoxEditField:
				break;
				case SC_SpinBoxUp:
					ret.setBounds(rect.w - 12, 0, 12, 8);
				break;
				case SC_SpinBoxDown:
					ret.setBounds(rect.w - 12, 8, 12, 8);
				break;
				default:
				break;
			}
		break;
		case CE_TitleBar: {
			const UTitleBarModel * titleBar = static_cast<const UTitleBarModel*>(model);
			int width = 16;
			int y = rect.h / 2 - width / 2;
			if (subElem & titleBar->subControls) {
				switch (subElem) {
					case SC_TitleBarSysMenu:
						ret.setBounds(2, y, width, width);
					break;
					case SC_TitleBarMinButton:
						ret.setBounds(rect.w - 3*width - 6, y, width, width);
					break;
					case SC_TitleBarMaxButton:
						ret.setBounds(rect.w - 2*width - 4, y, width, width);
					break;
					case SC_TitleBarCloseButton:
						ret.setBounds(rect.w - width - 2, y, width, width);
					break;
					case SC_TitleBarLabel: {
						const UFontMetrics * metrics = hints->font.getFontMetrics();
						ret.setBounds(20, y, metrics->getStringWidth(titleBar->text), metrics->getHeight());
					}
					break;
					default:
					break;
				}
			}
		}
		break;
		default:
		break;
	}
	return ret;
}

UDimension
UBasicStyle::getSizeFromContents(
		ComponentElement elem,
		const UDimension & contentsSize,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w)
{
	UInsets insets(getInsets(elem, hints, model, w));
	return contentsSize + insets;
}

void
UBasicStyle::install(UWidget * w) {}
void
UBasicStyle::uninstall(UWidget * w) {}


void
UBasicStyle::paintCompound(
		UGraphics * g,
		const UStyleHints * hints,
		const std::string & text,
		UIcon * icon,
		const URectangle & rect,
		uint32_t widgetState,
		int acceleratorIndex)
{
	URectangle viewRect, textRect, iconRect;
	//UInsets insets = getBorderInsets(hints->border, hints) +
	//	hints->margin;
	const UFont & f = hints->font;
	g->setFont(f);

	viewRect = rect;
	//viewRect -= insets;
	// due to focus we add some spacing to the view rect
	//if (w->isFocusable()) {
	//	viewRect += UInsets(1, 1, 1, 1);
	//}
/*
	std::string printText = layoutCompoundWidget(
		w,
		f,
		text,
		icon,
		&viewRect,
		&iconRect,
		&textRect,
		w->getHorizontalAlignment(),
		w->getVerticalAlignment(),
		//Horizontal,
		LeftToRight,
		w->getIconTextGap()
	);*/
	layoutCompound(
		hints,
		text,
		icon,
		viewRect,
		&textRect,
		&iconRect);

	// icon
	if (icon) {
		icon->paintIcon(g, iconRect, hints);
	}

	// text
	UDimension size = g->getStringSize(text);
	// if text size greater than max size, compute the minimized string
	std::string clipText;
	if (text.length() && size.w > textRect.w + 2) {
		const UFontMetrics * metrics = g->getFont().getFontMetrics();
		// cut the std::string and append "..."
		std::string appendString = "..";

		int minusWidth = metrics->getStringWidth(appendString);

		unsigned int index = metrics->viewToModel(text, textRect.w - minusWidth);

		clipText = text.substr(0, index);
		clipText.append(appendString);
	} else {
		clipText = text;
	}

	// set color
	if (widgetState & WidgetDisabled) {
		g->setColor(UColor::gray);
	} else {
		g->setColor(hints->palette.foreground());
	}

	//font->drawString(text, textRect.x, textRect.y);
	g->drawString(clipText, textRect.x, textRect.y);

	// underline accelerator character
	//if (UButton * button = dynamic_cast<UButton*>(w)) {

		int accel = acceleratorIndex;//button->getAcceleratorIndex();
		if (accel != -1 && accel < int(clipText.length())) {
			const UFontMetrics * metrics = g->getFont().getFontMetrics();
			int startx = metrics->getStringWidth(clipText.data(), accel);
			int width = metrics->getCharWidth(clipText[accel]);
			int height = textRect.y + metrics->getAscent() + metrics->getUnderlinePosition();
			g->drawLine(textRect.x + startx, height, textRect.x + startx + width, height);
		}
	//}
}


UDimension
UBasicStyle::getCompoundPreferredSize(
		const UStyleHints * hints,
		const std::string & text,
		UIcon * icon)
{
	// assume that icon is on the left and text on the right side
	const UFontMetrics * metrics = hints->font.getFontMetrics();

	UDimension text_size(metrics->getStringWidth(text), metrics->getHeight());
	UDimension icon_size(0, 0);
	if (icon) {
		icon_size = icon->getIconSize();
	}
	int textIconGap = 4;
	if (icon_size.isEmpty() || text_size.isEmpty()) {
		textIconGap = 0;
	}

	if (hints->orientation == Vertical) {
		return UDimension(
			std::max(text_size.w, icon_size.w),
			text_size.h + textIconGap + icon_size.w
		);
	} else {
		return UDimension(
			text_size.w + textIconGap + icon_size.w,
			std::max(text_size.h, icon_size.h)
		);
	}
}

void
UBasicStyle::layoutCompound(
		const UStyleHints * hints,
		const std::string & text,
		UIcon * icon,
		const URectangle & viewRect,
		URectangle * textRect,
		URectangle * iconRect)
{
	std::string clipText;
	int textIconGap = 4;

	const UFontMetrics * metrics = hints->font.getFontMetrics();

	if (icon) {
		*iconRect = URectangle(icon->getIconSize());
	} else {
		iconRect->w = iconRect->h = 0;
	}
	if (text.length() != 0) {
		textRect->w = metrics->getStringWidth(text);
		textRect->h = metrics->getHeight();
	} else {
		textRect->w = textRect->h = 0;
	}

	int gap = (text.length() && iconRect->w) ? textIconGap : 0;

	int maxTextWidth = (hints->orientation == Vertical) ?
		viewRect.w : viewRect.w - iconRect->w - gap;
	if (textRect->w > maxTextWidth) {
		textRect->w = maxTextWidth;
	}

	URectangle * rect1 = iconRect;
	URectangle * rect2 = textRect;

	if (hints->direction == RightToLeft) {
		rect1 = textRect;
		rect2 = iconRect;
	}

	// assume horizontal orientation
	//
	// add this to the trailing rect
	int addx = rect1->w + gap;
	int addy = 0;

	// gap between the two rects
	int hgap = gap;
	int vgap = 0;

	// alignment
	int valign = hints->vAlignment;
	int halign = hints->hAlignment;

	// if vertical, swap them
	if (hints->orientation == Vertical) {
		addx = 0;
		addy = rect1->h + gap;
		std::swap(hgap, vgap);
	}

	int xabundance = viewRect.w - (rect1->w + rect2->w + hgap);
	int yabundance = viewRect.h - (rect1->h + rect2->h + vgap);

	if (halign == AlignStart) {
		rect1->x = viewRect.x;
		rect2->x = viewRect.x + addx;
	} else if (halign == AlignEnd) {
		rect1->x = viewRect.x + xabundance;
		rect2->x = viewRect.x + addx + xabundance;
	} else {
		rect1->x = viewRect.x + xabundance / 2;
		rect2->x = viewRect.x + addx + xabundance / 2;
	}

	if (valign == AlignStart) {
		rect1->y = viewRect.y;
		rect2->y = viewRect.y + addy;
	} else if (valign == AlignEnd) {
		rect1->y = viewRect.y + yabundance;
		rect2->y = viewRect.y + addy + yabundance;
	} else {
		rect1->y = viewRect.y + yabundance / 2;
		rect2->y = viewRect.y + addy + yabundance / 2;
	}

/*

	// align Icon and text

	// new virtual rect that contains the icon and the label

	int cl_x = 0;
	int cl_y = 0;

	int cl_width = iconRect->w + gap + textRect->w;

	int cl_height = std::max(iconRect->h, textRect->h);


	textRect->x = cl_x + (iconRect->w ) + gap;
	textRect->y = cl_y + (cl_height - textRect->h) / 2;

	iconRect->x = cl_x;
	iconRect->y = cl_y + (cl_height - iconRect->h) / 2;

	// alignment

	int dx, dy;
	// horizontal
	if (hints->horizontalAlignment == AlignLeft) {
		dx = viewRect.x - cl_x;
	} else if (hints->horizontalAlignment == AlignRight) {
		dx = (viewRect.x + viewRect.w) - (cl_x + cl_width);
	} else { // ufo::AlignCenter
		dx = (viewRect.x + (viewRect.w / 2)) - (cl_x + (cl_width / 2));
	}

	// vertical
	if (hints->verticalAlignment == AlignTop) {
		dy = viewRect.y - cl_y;
	} else if (hints->verticalAlignment == AlignBottom) {
		dy = (viewRect.y + viewRect.h) - (cl_y + cl_height);
	} else { // ufo::AlignCenter
		dy = (viewRect.y + (viewRect.h / 2)) - (cl_y + (cl_height / 2));
	}

	textRect->x += dx;
	textRect->y += dy;

	iconRect->x += dx;
	iconRect->y += dy;
*/
	//return clipText;
}
