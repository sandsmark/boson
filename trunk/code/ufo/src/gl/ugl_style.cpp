/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/gl/ugl_style.cpp
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


#include "ufo/gl/ugl_style.hpp"

#include <algorithm>
//#include "ufo/ufo_gl.hpp"
#include "ufo/gl/ugl_driver.hpp"

#include "ufo/ugraphics.hpp"
#include "ufo/uicon.hpp"

#include "ufo/ui/uuimanager.hpp"

#include "ufo/font/ufont.hpp"

#include "ufo/util/ucolor.hpp"
#include "ufo/util/uinteger.hpp"

#include "ufo/widgets/ucompound.hpp"
#include "ufo/widgets/ubutton.hpp"
#include "ufo/widgets/umenu.hpp"
#include "ufo/text/ucaret.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UGL_Style, UStyle)


void
UGL_Style::paintBevel(UGraphics * g, UWidget * w,
		const URectangle & rect,
		Relief type)
{
	int x = rect.x; int y = rect.y;
	int width = rect.w; int height = rect.h;
	UColor highlightInner;
	UColor highlightOuter;
	UColor shadowInner;
	UColor shadowOuter;

	if (type == Raised) {
		highlightInner = w->getBackgroundColor().brighter();
		highlightOuter = highlightInner.brighter();
		shadowInner = w->getBackgroundColor().darker();
		shadowOuter = shadowInner.darker();
	} else {
		shadowInner = w->getBackgroundColor().brighter();
		shadowOuter = shadowInner.brighter();
		highlightInner = w->getBackgroundColor().darker();
		highlightOuter = highlightInner.darker();
	}

	ugl_driver->glPushMatrix();
	ugl_driver->glTranslatef(x, y, 0.0);
	ugl_driver->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	ugl_driver->glBegin(GL_LINES);
	{
		ugl_driver->glColor3fv( highlightInner.getFloat() );

		ugl_driver->glVertex2i( 0, 0 );
		ugl_driver->glVertex2i( 0, height - 1 );

		ugl_driver->glVertex2i( 1, 0 );
		ugl_driver->glVertex2i( width - 1, 0 );

		ugl_driver->glColor3fv( highlightOuter.getFloat() );

		ugl_driver->glVertex2i( 1, 1 );
		ugl_driver->glVertex2i( 1, height - 2 );

		ugl_driver->glVertex2i( 2, 1 );
		ugl_driver->glVertex2i( width - 2, 1 );

		ugl_driver->glColor3fv( shadowOuter.getFloat() );

		ugl_driver->glVertex2i( 1, height - 1 );
		ugl_driver->glVertex2i( width - 1, height - 1 );

		ugl_driver->glVertex2i( width - 1, 1 );
		//glVertex2i( width-1, height-2 );
		ugl_driver->glVertex2i( width - 1, height - 1 );

		ugl_driver->glColor3fv( shadowInner.getFloat() );

		ugl_driver->glVertex2i( 2, height - 2 );
		ugl_driver->glVertex2i( width - 2, height - 2 );

		ugl_driver->glVertex2i( width - 2, 2 );
		//glVertex2i( width-1, height-3 );
		ugl_driver->glVertex2i( width - 2, height - 2 );
	}
	ugl_driver->glEnd();

	ugl_driver->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	ugl_driver->glColor3f(1, 1, 1);

	ugl_driver->glPopMatrix();
}

void
UGL_Style::paintArrow(UGraphics * g, UWidget * w,
		const URectangle & rect,
		bool highlighted, bool activated,
		Direction direction)
{
	int x = rect.x; int y = rect.y;
	int width = rect.w; int height = rect.h;

	// prettify arrow (anti-aliasing)
	//ugl_driver->glPushAttrib(GL_COLOR_BUFFER_BIT);
	//ugl_driver->glEnable(GL_BLEND);
	//ugl_driver->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (highlighted || activated) {
		ugl_driver->glPushAttrib(GL_POLYGON_BIT);
		ugl_driver->glEnable(GL_POLYGON_SMOOTH);
		ugl_driver->glBegin(GL_TRIANGLES);
	} else {
		ugl_driver->glPushAttrib(GL_LINE_BIT);
		ugl_driver->glEnable(GL_LINE_SMOOTH);
		ugl_driver->glBegin(GL_LINE_LOOP);
	}
	switch (direction) {
		case Up:
			ugl_driver->glVertex2i(x, y);
			ugl_driver->glVertex2i(x + width, y);
			ugl_driver->glVertex2i(x + width / 2, y + height);
			break;
		case Down:
			ugl_driver->glVertex2i(x + width / 2, y);
			ugl_driver->glVertex2i(x, y + height);
			ugl_driver->glVertex2i(x + width, y + height);
			break;
		case Left:
			ugl_driver->glVertex2i(x, y + height/2);
			ugl_driver->glVertex2i(x + width, y + height);
			ugl_driver->glVertex2i(x + width, y);
			break;
		case Right:
			ugl_driver->glVertex2i(x, y);
			ugl_driver->glVertex2i(x, y + height);
			ugl_driver->glVertex2i(x + width, y + height/2);
			break;
		default:
			break;
	}
	ugl_driver->glEnd();
	// unset smoothness
	ugl_driver->glPopAttrib();
}

void
UGL_Style::paintFocus(UGraphics * g, UWidget * w,
		const URectangle & rect)
{
	int x = rect.x; int y = rect.y;
	int width = rect.w; int height = rect.h;
	// paint dots, not a drawn through line
	ugl_driver->glPushAttrib(GL_LINE_STIPPLE);

	GLushort pattern = 0x3333;
	ugl_driver->glLineStipple(1, pattern);
	ugl_driver->glEnable(GL_LINE_STIPPLE);

	ugl_driver->glBegin(GL_LINE_LOOP);

	//glColor3fv(b->getUIManager()->getTheme()->getFocusColor()->getFloat());
	// FIXME:
	// add focus color to look and feel
	//glColor3fv(b->getUIManager()->getColor("UButton.foreground")->getFloat());
	ugl_driver->glColor3fv(w->getColorGroup().foreground().getFloat());

	ugl_driver->glVertex2i(x - 1, y - 1);
	ugl_driver->glVertex2i(x - 1, y + height);// + 1);
	ugl_driver->glVertex2i(x + width/* + 1*/, y + height);// + 1);
	ugl_driver->glVertex2i(x + width/* + 1*/, y - 1);

	ugl_driver->glEnd();

	ugl_driver->glPopAttrib();
}


void
UGL_Style::paintScrollTrack(UGraphics * g, UWidget * w,
		const URectangle & rect,
		bool highlighted, bool activated)
{
	g->setColor(w->getColorGroup().highlight());
	g->fillRect(rect);
	g->setColor(w->getColorGroup().highlight().darker());
	g->drawRect(rect);
	//glRecti(x, y, width, height);
}


void
UGL_Style::paintTextSelection(UGraphics * g, UWidget * w,
		int left, int right,
		const URectangle & position, const URectangle & mark)
{
	const UFontMetrics * metrics = w->getFont()->getFontMetrics();
	unsigned int lineHeight = metrics->getLineskip();


	// one of the common caret looks
	// a vertical line with a horizontal line above and below
	//glColor3f(0.9f, 0.9f, 0.95f);
	//ugl_driver->glColor3fv(w->getUIManager()->
	//	getColor("UTextEdit.selectionBackground")->getFloat());
	ugl_driver->glColor3fv(w->getColorGroup().highlight().getFloat());
	if (position.y == mark.y) {
		unsigned int minx = std::min(position.x, mark.x);
		unsigned int maxx = std::max(position.x, mark.x);

		ugl_driver->glRecti(minx, position.y, maxx, position.y + lineHeight);
	} else {
		UDimension size = w->getInnerSize();
		UPoint min, max;

		if (position.y < mark.y) {
			min = position.getLocation();
			max = mark.getLocation();
		} else  {
			min = mark.getLocation();
			max = position.getLocation();
		}

		ugl_driver->glRecti(min.x, min.y, size.w, min.y + lineHeight);
		ugl_driver->glRecti(0, min.y + lineHeight, size.w, max.y);
		ugl_driver->glRecti(0, max.y, max.x, max.y + lineHeight);
	}
}

void
UGL_Style::paintCaret(UGraphics * g, UWidget * w,
		const URectangle & rect, UCaret * caret)
{
	ugl_driver->glColor3f(1.f, 0.f, 0.0f);
	ugl_driver->glBegin(GL_LINES);
	ugl_driver->glVertex2i(rect.x/* - 1*/, rect.y);
	ugl_driver->glVertex2i(rect.x/* - 1*/, rect.y + rect.h - 1);

	ugl_driver->glVertex2i(rect.x - 3, rect.y);
	ugl_driver->glVertex2i(rect.x + 2, rect.y);

	ugl_driver->glVertex2i(rect.x - 3, rect.y + rect.h - 1);
	ugl_driver->glVertex2i(rect.x + 2, rect.y + rect.h - 1);
	ugl_driver->glEnd();
}


void
UGL_Style::paintTab(UGraphics * g, UWidget * w,
		const URectangle & rect, const std::string & text,
		bool highlighted, bool activated)
{
}

void
UGL_Style::paintControlCaption(UGraphics * g, UWidget * w,
		const URectangle & rect, const std::string & text)
{
	std::string clipText;
	UDimension size = g->getStringSize(text);
	// if text size greater than max size, compute the minimized string
	if (text.length() && size.w > rect.w + 2) {
		const UFontMetrics * metrics = g->getFont()->getFontMetrics();
		// cut the std::string and append "..."
		std::string appendString = "..";

		int minusWidth = metrics->getStringWidth(appendString);

		unsigned int index = metrics->viewToModel(text, rect.w - minusWidth);

		clipText.append(text.begin(), text.begin() + index).append(appendString);
	} else {
		clipText = text;
	}

	// set color
	ugl_driver->glColor3fv(w->getColorGroup().foreground().getFloat());

	//font->drawString(text, textRect.x, textRect.y);
	g->drawString(clipText, rect.x, rect.y);

	// underline accelerator character
	if (UButton * button = dynamic_cast<UButton*>(w)) {
		int accel = button->getAcceleratorIndex();
		if (accel != -1 && accel < int(clipText.length())) {
			const UFontMetrics * metrics = g->getFont()->getFontMetrics();
			int startx = metrics->getStringWidth(clipText.data(), accel);
			int width = metrics->getCharWidth(clipText[accel]);
			int height = rect.y + metrics->getAscent() + metrics->getUnderlinePosition();
			g->drawLine(rect.x + startx, height, rect.x + startx + width, height);
		}
	}
}

void
UGL_Style::paintControlBackground(UGraphics * g, UWidget * w,
		const URectangle & rect,
		bool highlighted, bool activated)
{
	UColor color;
	if (highlighted) {
		color = w->getColorGroup().highlight();
	} else if (activated) {
		// should get the right color group via getColorGroup
		color = w->getColorGroup().background();
	} else {
		color = w->getColorGroup().background();
	}
	color.getFloat()[3] = w->getOpacity();
	ugl_driver->glColor4fv(color.getFloat());
	ugl_driver->glRecti(rect.x, rect.y, rect.w, rect.h);
}

void
UGL_Style::paintCompoundTextAndIcon(UGraphics * g, UCompound * w,
		const URectangle & rect, const std::string & text,
		UIcon * icon)
{
	URectangle viewRect, textRect, iconRect;
	UInsets insets = w->getInsets();
	const UFont * f = w->getFont();
	g->setFont(f);

	viewRect.x = insets.left;
	viewRect.y = insets.top;
	viewRect.w = w->getWidth() - insets.getHorizontal();
	viewRect.h = w->getHeight() - insets.getVertical();
	// due to focus we add some spacing to the view rect
	if (w->isFocusable()) {
		viewRect.x += 1;
		viewRect.y += 1;
		viewRect.w -= 2;
		viewRect.h -= 2;
	}

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
		w->getIconTextGap()
	);
/*
	if (c->isPressed()) {
		g->translate(1, 1);//glTranslatef(1, 1, 0);
	}
*/
	/*if (UIcon * icon = c->getIcon()) {
		icon->paintIcon(g, c, iconRect, icon);
	}*/
	if (icon) {
		icon->paintIcon(g, w, iconRect.x, iconRect.y);
	}

	//if (printText.length() != 0) {
		paintControlCaption(g, w, textRect, text);
	//}

	//return viewRect;
}


//
//
//

void
UGL_Style::paintBorder(UGraphics * g, UWidget * w,
		const URectangle & rect, BorderType borderType)
{
	if (borderType != UIBorder && w->get("border_color")) {
		UColorObject * col = dynamic_cast<UColorObject*>(w->get("border_color"));
		if (col) {
			ugl_driver->glColor4fv(col->getFloat());
		}
	} else {
		ugl_driver->glColor4fv(w->getColorGroup().foreground().getFloat());
	}

	switch (borderType) {
		case RaisedBevelBorder:
			paintRaisedBevelBorder(g, w, rect);
			break;
		case LoweredBevelBorder:
			paintLoweredBevelBorder(g, w, rect);
			break;
		case LineBorder:
			paintLineBorder(g, w, rect);
			break;
		case TitledBorder:
			paintTitledBorder(g, w, rect);
			break;
		case UIBorder:
			paintUIBorder(g, w, rect);
			break;
		default:
			break;
	}
}

UInsets
UGL_Style::getBorderInsets(UWidget * w, BorderType borderType)
{
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
			UString * str = dynamic_cast<UString*>(w->get("border_title"));
			if (str && w->getGraphics()) {
				w->getGraphics()->setFont(w->getFont());
				add = w->getGraphics()->getStringSize(str->str()).h;
			}
			return UInsets(1 + add, 1, 1, 1);
			}
			break;
		case BorderType(UIBorder):
			return getUIBorderInsets(w, borderType);
			break;
		default:
			break;
	}
	return UInsets();
}

std::string
UGL_Style::layoutCompoundWidget(
		const UCompound * w,
		const UFont * f,
		const std::string & text,
		const UIcon * icon,
		URectangle * viewRect,
		URectangle * iconRect,
		URectangle * textRect,
		Alignment hAlignment,
		Alignment vAlignment,
		int textIconGap)
{
	std::string clipText;

	const UFontMetrics * metrics = f->getFontMetrics();

	if (icon) {
		iconRect->w = icon->getIconWidth();
		iconRect->h = icon->getIconHeight();
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

	int maxTextWidth = viewRect->w - iconRect->w - gap;
	if (textRect->w > maxTextWidth) {
		textRect->w = maxTextWidth;
	}
/*
	// FIXME
	//    use blending (not appending '...')

	// compute the returned std::string
	if (text.length() && textRect->w > maxTextWidth) {
		// cut the std::string and append "..."
		std::string appendString = "...";

		int minusWidth = metrics->getStringWidth(appendString);

		unsigned int index = metrics->viewToModel(text, maxTextWidth - minusWidth);

		clipText.append(text.begin(), text.begin() + index).append(appendString);

		//textRect->w = metrics->getStringWidth(clipText);
		textRect->w = maxTextWidth;
	} else {
		clipText = text;
	}
*/

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
	if (hAlignment == AlignLeft) {
		dx = viewRect->x - cl_x;
	} else if (hAlignment == AlignRight) {
		dx = (viewRect->x + viewRect->w) - (cl_x + cl_width);
	} else { // ufo::AlignCenter
		dx = (viewRect->x + (viewRect->w / 2)) - (cl_x + (cl_width / 2));
	}

	// vertical
	if (vAlignment == AlignTop) {
		dy = viewRect->y - cl_y;
	} else if (vAlignment == AlignBottom) {
		dy = (viewRect->y + viewRect->h) - (cl_y + cl_height);
	} else { // ufo::AlignCenter
		dy = (viewRect->y + (viewRect->h / 2)) - (cl_y + (cl_height / 2));
	}

	textRect->x += dx;
	textRect->y += dy;

	iconRect->x += dx;
	iconRect->y += dy;

	return clipText;
}
/*
UDimension
UGL_Style::getCompoundPreferredSize(const UCompound * w) {
	std::string text = w->getText();
	UIcon * icon = w->getIcon();
	const UInsets & insets = w->getInsets();
	const UFont * font = w->getFont();

	int dx = insets.left + insets.right;
	int dy = insets.top + insets.bottom;

	if (!font && !icon) {
		std::cerr << "counpund size without font\n";
		return UDimension(dx, dy);
	} else if ( !font && icon) {
		std::cerr << "counpund size without font\n";
		return UDimension(icon->getIconWidth() + dx,
			icon->getIconHeight() + dy);
	} else {
		UDimension ret = UGL_Style::getCompoundPreferredSize(
			w,
			font,
			text,
			icon
		);

		ret.w += dx;
		ret.h += dy;
		return ret;
	}
}
*/
UDimension
UGL_Style::getCompoundPreferredSize(
		const UCompound * w,
		const UFont * f,
		const std::string & text,
		const UIcon * icon)
{
	// assume that icon is on the left and text on the right side
	const UFontMetrics * metrics = f->getFontMetrics();

	int max_width = metrics->getStringWidth(text);
	int max_height = 0;

	if (icon) {
		int icon_Width = icon->getIconWidth();

		// add text-icon gap only if both icon and text have greater width then 0
		if ( ( icon_Width ) && ( max_width ) ) {
			max_width += w->getIconTextGap();
		}

		max_width += icon_Width;
		max_height += icon->getIconHeight();
	}

	max_height = std::max(metrics->getHeight(), max_height);

	// add a small space for focus drawing
	//max_width += 2;
	//max_height += 2;

	UInsets in = w->getInsets();
	return UDimension(
		max_width + in.getHorizontal(),
		max_height + in.getVertical()
	);
	//return UDimension(max_width, max_height);
}


//
// Protected helper functions
//

void
UGL_Style::paintLineBorder(UGraphics * g, UWidget * w,
		const URectangle & rect)
{
	// FIXME: line width not taken into account in getBorderInsets
	bool lineChanged = 0;
	int lineWith = 1;

	if (UObject * l = w->get("border_line_width")) {
		UInteger * lineWidth = dynamic_cast<UInteger*>(l);
		if (lineWidth) {
			ugl_driver->glPushAttrib(GL_LINE_BIT);
			ugl_driver->glLineWidth(lineWidth->toInt());
			lineChanged = true;
		}
	}

	ugl_driver->glBegin(GL_LINE_LOOP);
	ugl_driver->glVertex2i(rect.x, rect.y);
	ugl_driver->glVertex2i(rect.x, rect.y + rect.h - lineWith);
	ugl_driver->glVertex2i(rect.x + rect.w - lineWith, rect.y + rect.h - lineWith);
	ugl_driver->glVertex2i(rect.x + rect.w - lineWith, rect.y);
	ugl_driver->glEnd();

	if (lineChanged) {
		ugl_driver->glPopAttrib();
	}
}

void
UGL_Style::paintRaisedBevelBorder(UGraphics * g, UWidget * w,
		const URectangle & rect)
{
	UColor highlightOuter = w->getBackgroundColor().darker().darker();
	UColor highlightInner = w->getBackgroundColor().darker();
	UColor shadowOuter = w->getBackgroundColor().brighter().brighter();
	UColor shadowInner = w->getBackgroundColor().brighter();

	//ugl_driver->glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	ugl_driver->glBegin(GL_LINES);
	{
		ugl_driver->glColor3fv(shadowInner.getFloat());
		// left vertical outer line
		ugl_driver->glVertex2i(rect.x, rect.y);
		ugl_driver->glVertex2i(rect.x, rect.y + rect.h - 1);
		// upper horizontal outer line
		ugl_driver->glVertex2i(rect.x + 1, rect.y);
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y);

		ugl_driver->glColor3fv(shadowOuter.getFloat());
		// left vertical inner line
		ugl_driver->glVertex2i(rect.x + 1, rect.y + 1);
		ugl_driver->glVertex2i(rect.x + 1, rect.y + rect.h - 2);
		// upper horizontal inner line
		ugl_driver->glVertex2i(rect.x + 2, rect.y + 1);
		ugl_driver->glVertex2i(rect.x + rect.w - 2, rect.y + 1);

		ugl_driver->glColor3fv(highlightOuter.getFloat());
		// lower horizontal outer line
		ugl_driver->glVertex2i(rect.x + 1, rect.y + rect.h - 1 );
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + rect.h - 1);
		// right vertical outer line
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + 1 );
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + rect.h - 1);

		ugl_driver->glColor3fv(highlightInner.getFloat());
		// lower horizontal inner line
		ugl_driver->glVertex2i(rect.x + 2, rect.y + rect.h - 2 );
		ugl_driver->glVertex2i(rect.x + rect.w - 2, rect.y + rect.h - 2);
		// right vertical inner line
		ugl_driver->glVertex2i(rect.x + rect.w - 2, rect.y + 2 );
		ugl_driver->glVertex2i(rect.x + rect.w - 2, rect.y + rect.h - 2);
	}
	ugl_driver->glEnd();

	//ugl_driver->glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void
UGL_Style::paintLoweredBevelBorder(UGraphics * g, UWidget * w,
		const URectangle & rect)
{
	UColor highlightOuter = w->getBackgroundColor().brighter().brighter();
	UColor highlightInner = w->getBackgroundColor().brighter();
	UColor shadowOuter = w->getBackgroundColor().darker().darker();
	UColor shadowInner = w->getBackgroundColor().darker();

	//ugl_driver->glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	ugl_driver->glBegin(GL_LINES);
	{
		ugl_driver->glColor3fv(shadowInner.getFloat());
		// left vertical outer line
		ugl_driver->glVertex2i(rect.x, rect.y);
		ugl_driver->glVertex2i(rect.x, rect.y + rect.h - 1);
		// upper horizontal outer line
		ugl_driver->glVertex2i(rect.x + 1, rect.y);
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y);

		ugl_driver->glColor3fv(shadowOuter.getFloat());
		// left vertical inner line
		ugl_driver->glVertex2i(rect.x + 1, rect.y + 1);
		ugl_driver->glVertex2i(rect.x + 1, rect.y + rect.h - 2);
		// upper horizontal inner line
		ugl_driver->glVertex2i(rect.x + 2, rect.y + 1);
		ugl_driver->glVertex2i(rect.x + rect.w - 2, rect.y + 1);

		ugl_driver->glColor3fv(highlightOuter.getFloat());
		// lower horizontal outer line
		ugl_driver->glVertex2i(rect.x + 1, rect.y + rect.h - 1 );
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + rect.h - 1);
		// right vertical outer line
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + 1 );
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + rect.h - 1);

		ugl_driver->glColor3fv(highlightInner.getFloat());
		// lower horizontal inner line
		ugl_driver->glVertex2i(rect.x + 2, rect.y + rect.h - 2 );
		ugl_driver->glVertex2i(rect.x + rect.w - 2, rect.y + rect.h - 2);
		// right vertical inner line
		ugl_driver->glVertex2i(rect.x + rect.w - 2, rect.y + 2 );
		ugl_driver->glVertex2i(rect.x + rect.w - 2, rect.y + rect.h - 2);
	}
	ugl_driver->glEnd();

	//ugl_driver->glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void
UGL_Style::paintTitledBorder(UGraphics * g, UWidget * w,
		const URectangle & rect)
{
	std::string title;
	if (UString * str = dynamic_cast<UString*>(w->get("border_title"))) {
		title = *str;
	}

	if (title != "") {
		UDimension size = g->getStringSize(title);
		int right = std::min(rect.w - 1, 9 + size.w);
		int top = size.h / 2;
		ugl_driver->glBegin(GL_LINE_STRIP);
		ugl_driver->glVertex2i(rect.x + 5, rect.y + top);
		ugl_driver->glVertex2i(rect.x, rect.y + top);
		ugl_driver->glVertex2i(rect.x, rect.y + rect.h - 1);
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + rect.h - 1);
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + top);
		ugl_driver->glVertex2i(rect.x + right, rect.y + top);
		ugl_driver->glEnd();
		std::string clipText;
		if (size.w + 9 > rect.w) {
			// if text size greater than max size, compute the minimized string
			const UFontMetrics * metrics = g->getFont()->getFontMetrics();
			// cut the std::string and append "..."
			std::string appendString = "..";

			int minusWidth = metrics->getStringWidth(appendString);
			unsigned int index = metrics->viewToModel(title, rect.w - minusWidth);

			clipText.append(title.begin(), title.begin() + index).append(appendString);
		} else {
			clipText = title;
		}
		g->drawString(clipText, rect.x + 7);
	} else {
		ugl_driver->glBegin(GL_LINE_LOOP);
		ugl_driver->glVertex2i(rect.x, rect.y);
		ugl_driver->glVertex2i(rect.x, rect.y + rect.h - 1);
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + rect.h - 1);
		ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y);
		ugl_driver->glEnd();
	}
}


void
UGL_Style::paintUIBorder(UGraphics * g, UWidget * w,
		const URectangle & rect)
{
	std::string ui = w->getUIClassID();

	if (ui == "UButtonUI") {
		// control border
		paintControlBorder(g, w, rect);
	} else if (ui == "UMenuItemUI") {
		// menu border
		paintMenuBorder(g, w, rect);
	} else if (ui == "UMenuBarUI") {
		paintMenuBarBorder(g, w, rect);
	} else if (ui == "UInternalFrameUI") {
		paintInternalFrameBorder(g, w, rect);
	}
}

UInsets
UGL_Style::getUIBorderInsets(UWidget * w, BorderType borderType)
{
	std::string ui = w->getUIClassID();

	if (ui == "UButtonUI") {
		// control border
		return UInsets(2, 2, 2, 2);
	} else if (ui == "UMenuItemUI") {
		// menu border
		if (dynamic_cast<UMenu*>(w)) {
			return UInsets(2, 2, 2, 2);
		}
	} else if (ui == "UMenuBarUI") {
		// menu bar border
		return UInsets(0, 0, 2, 0);
	} else if (ui == "UInternalFrameUI") {
		// internal frame border
		return UInsets(2, 3, 3, 3);
	}
	return UInsets();
}


void
UGL_Style::paintControlBorder(UGraphics * g, UWidget * w,
		const URectangle & rect)
{
	UButton * b = dynamic_cast<UButton*>(w);
	if (b == NULL) {
		return;
	}

	bool isPressed = b->isPressed();
	bool isRollover = b->isRollover();
	//bool isDsp = false; // NOTE :: not yet supported

	if (isPressed) {
		paintBevel(g, w, rect, Lowered);
	} else if (isRollover) {
		paintBevel(g, w, rect, Raised);
	} else if (w->isOpaque()) {
		UColor brighter = w->getBackgroundColor().brighter();
		UColor darker = w->getBackgroundColor().darker();

		ugl_driver->glBegin(GL_LINES);

		ugl_driver->glColor3fv(brighter.getFloat());
		ugl_driver->glVertex2f(rect.x, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x, rect.y);
		ugl_driver->glVertex2f(rect.x, rect.y);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y);

		ugl_driver->glColor3fv(darker.getFloat());
		ugl_driver->glVertex2f(rect.x, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y);

		ugl_driver->glEnd();
	}
}

void
UGL_Style::paintMenuBorder(UGraphics * g, UWidget * w,
		const URectangle & rect)
{
	UMenu * m;
	if (!(m = dynamic_cast<UMenu*>(w))) {
		return ;
	}
	//if (m->isRollover()) || m->isPopupMenuVisible()) {
	//	paintBevel(g, w, rect, Lowered);
	//}
	if (m->isPopupMenuVisible()) {
		UColor brighter = w->getBackgroundColor().brighter();
		UColor darker = w->getBackgroundColor().darker();

		ugl_driver->glBegin(GL_LINES);

		ugl_driver->glColor3fv(darker.getFloat());
		ugl_driver->glVertex2f(rect.x, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x, rect.y);
		ugl_driver->glVertex2f(rect.x, rect.y);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y);

		ugl_driver->glColor3fv(brighter.getFloat());
		ugl_driver->glVertex2f(rect.x, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y);

		ugl_driver->glEnd();
	} else if (m->isRollover()) {
		//paintBevel(g, w, rect, Lowered);
		UColor brighter = w->getBackgroundColor().brighter();
		UColor darker = w->getBackgroundColor().darker();

		ugl_driver->glBegin(GL_LINES);

		ugl_driver->glColor3fv(brighter.getFloat());
		ugl_driver->glVertex2f(rect.x, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x, rect.y);
		ugl_driver->glVertex2f(rect.x, rect.y);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y);

		ugl_driver->glColor3fv(darker.getFloat());
		ugl_driver->glVertex2f(rect.x, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y + rect.h - 1);
		ugl_driver->glVertex2f(rect.x + rect.w - 1, rect.y);

		ugl_driver->glEnd();
	}
}

void
UGL_Style::paintMenuBarBorder(UGraphics * g, UWidget * w,
		const URectangle & rect)
{
	UColor darker = w->getBackgroundColor().darker();
	UColor darkerDarker = darker.darker();

	ugl_driver->glBegin(GL_LINES);
	// inner border
	ugl_driver->glColor3fv(darker.getFloat());
	ugl_driver->glVertex2i(rect.x, rect.y + rect.h - 2);
	ugl_driver->glVertex2i(rect.x + rect.w, rect.y + rect.h - 2);

	// outer border
	ugl_driver->glColor3fv(darkerDarker.getFloat());
	ugl_driver->glVertex2i(rect.x + 1, rect.y + rect.h - 1);
	ugl_driver->glVertex2i(rect.x + rect.w, rect.y + rect.h - 1);
	ugl_driver->glEnd();
}

void
UGL_Style::paintInternalFrameBorder(UGraphics * g, UWidget * w,
		const URectangle & rect)
{
	//UUIManager * manager = w->getUIManager();

	ugl_driver->glColor3fv(UColor::black.getFloat());

	ugl_driver->glBegin(GL_LINE_LOOP);
	ugl_driver->glVertex2i(rect.x, rect.y);
	ugl_driver->glVertex2i(rect.x, rect.y + rect.h - 1);
	ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + rect.h - 1);
	ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y);
	ugl_driver->glEnd();

	//ugl_driver->glColor3fv(manager->getColor("UInternalFrame.activeTitleBackground")->getFloat());
	ugl_driver->glColor3fv(w->getColorGroup().background().getFloat());

	ugl_driver->glBegin(GL_LINE_LOOP);
	ugl_driver->glVertex2i(rect.x + 1, rect.y + 1);
	ugl_driver->glVertex2i(rect.x + rect.w - 2, rect.y + 1);
	ugl_driver->glVertex2i(rect.x + rect.w - 2, rect.y + rect.h - 2);
	ugl_driver->glVertex2i(rect.x + 1, rect.y + rect.h - 2);
	ugl_driver->glEnd();

	ugl_driver->glColor3fv(UColor::darkGray.getFloat());

	ugl_driver->glBegin(GL_LINE_STRIP);
	ugl_driver->glVertex2i(rect.x + 2, rect.y + 2);
	ugl_driver->glVertex2i(rect.x + 2, rect.y + rect.h - 3);
	ugl_driver->glVertex2i(rect.x + rect.w - 3, rect.y + rect.h - 3);
	ugl_driver->glVertex2i(rect.x + rect.w - 3, rect.y + 2);
	ugl_driver->glEnd();
}

