/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
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

// AB: this is a modified file from libufo. the license remains LGPL.
// the original file was src/ui/basic/ubasiclabelui.cpp

#include "ubolabelui.h"

#include <ufo/ui/uuimanager.hpp>
#include <ufo/ui/ustyle.hpp>
#include <ufo/uicon.hpp>
#include <ufo/ugraphics.hpp>
#include <ufo/gl/ugl_driver.hpp>
#include <ufo/widgets/ubutton.hpp>
#include <ufo/widgets/ulabel.hpp>
#include <ufo/font/ufont.hpp>
#include <ufo/font/ufont.hpp>
#include <ufo/util/udimension.hpp>


using namespace ufo;



/**
 * Split @p string into several strings. Use @p sep as separator. The separator
 * is removed from all strings.
 **/
static void
splitString(const std::string& string, char sep, std::list<std::string>* list)
{
 std::string s = string;
 while (s.size() > 0) {
	std::string::size_type i = s.find(sep);
	if (i == std::string::npos) {
		i = s.size();
	}
	list->push_back(s.substr(0, i));
	if (i != s.size()) {
		s = s.substr(i + 1, s.size() - (i + 1));
	} else {
		s = std::string();
	}
 }
}

static bool
isWrapPos(char c)
{
 if (c == ' ') {
	return true;
 }
 // AB: '\n' is not allowed here anyway, so no need to check for it
 return false;
}

/**
 * Forms a line with a most @p maxWidth width. The line starts at index 0 of @p
 * line and contains the returned number of characters.
 *
 * Used by @ref makeLines only.
 *
 * @param line A text without (!) newline characters ('\\n')
 * @param maxWidth The maximal available size (lines are wrapped if this can't
 * is not enough). -1 for unlimited.
 * @return How many characters should be painted without exceeding @p maxWidth
 **/
static int
makeLine(const std::string& line, const UFontMetrics* metrics, int maxWidth, int* textWidth)
{
 if ((int)(line.size() * metrics->getMaxCharWidth()) < maxWidth || maxWidth < 0) {
	// no need to wrap
	if (textWidth) {
		*textWidth = metrics->getStringWidth(line);
	}
	return line.size();
 }

 // AB: is this safe? is it possible that a metrics->getStringWith(s) is greater
 // than the sum of metrics->getCharWidth(s[i]) for all i ?
 // e.g. for italic fonts?
 // if so then we need to adjust this to these cases!
 // (note that doing getStringWidth() every iteration would be too slow

 int pos = -1;
 int wrapPos = -1; // wrap before this char
 int wrapw = 0; // width of chars 0..wrapPos-1
 int oldw = 0;
 int neww = 0;
 while (neww <= maxWidth) {
	oldw = neww;
	pos++;
	if (pos < (int)line.size()) {
		neww += metrics->getCharWidth(line[pos]);
		if (isWrapPos(line[pos])) {
			wrapPos = pos;
			wrapw = oldw;
		}
	} else {
		wrapPos = pos;
		wrapw = oldw;
		break;
	}
 }
 if (wrapPos < 0) {
	// we did not find a good wrap position.
	wrapPos = pos;
	wrapw = oldw;
 }

 if (textWidth) {
	*textWidth = wrapw;
 }

 return wrapPos; // this is the cound of chars before we wrap
}


/**
 * Converts a @p line into several lines, each at most @p maxWidth wide. The
 * original line must not contain anye newlines ('\\n').
 *
 * The returned string will end with '\\n'.
 *
 * @param textWidth The actual width of the returned lines is returned here if
 * non-NULL. That is the width of the widest line that is returned.
 * @param lineCount The number of returned lines is returned here, if non-NULL.
 * This is always at least 1 (the original line).
 * @param line The line that should be split up. Must not contain newlines
 * ('\\n').
 * @param maxWidth How wide the lines are allowed to be, use -1 for unlimited.
 * This parameter controls how many lines are created. If maxWidth is greater
 * that the width of @p line, then no line wrapping is neccessary and only a
 * single line is returned.
 **/
static std::string
makeLines(const std::string& line, const UFontMetrics* metrics, int maxWidth, int* textWidth, int* lineCount)
{
 if (line.size() == 0) {
	if (textWidth) {
		*textWidth = 0;
	}
	if (lineCount) {
		*lineCount = 1;
	}
	return "\n";
 }
 std::string l = line;
 std::string lines;
 while (l.size() > 0) {
	int w = 0;
	int n = makeLine(l, metrics, maxWidth, &w);

	lines += l.substr(0, n) + '\n';
	if (n < (int)l.size()) {
		l = l.substr(n, l.size() - n);
	} else {
		l = std::string();
	}

	if (textWidth) {
		if (w > *textWidth) {
			*textWidth = w;
		}
	}
	if (lineCount) {
		(*lineCount)++;
	}
	if (n == 0) {
		// oops that should not happen. somethine evil must have
		// happened if we cannot even paint a single character.
		return lines;
	}
 }
 return lines;
}

/**
 * Create text for a @ref UCompound widget. See also @ref makeLines that is used
 * here.
 * @param text Use UCompoung::getText here
 * @param maxWidth The maximal width allowed for the text. Use -1 for unlimited
 * @param size The required size is returned in here, if non-NULL. The text area
 * must be at least that large.
 **/
static std::string
makeCompoundText(const std::string& text, const UFontMetrics* metrics, int maxWidth, UDimension* size)
{
 std::string allLines;
 std::string s = text;
 std::list<std::string> list;
 std::list<std::string>::iterator it;
 splitString(text, '\n', &list);
 int lineCount = 0;
 int width = 0;
 for (it = list.begin(); it != list.end(); ++it) {
	int w = 0;
	int count = 0;
	std::string lines  = makeLines(*it, metrics, maxWidth, &w, &count);
	if (w > width) {
		width = w;
	}
	allLines += lines;
	lineCount += count;
 }
 if (size) {
	*size = UDimension(width, lineCount * metrics->getHeight());
 }
 return allLines;
}







UFO_IMPLEMENT_DYNAMIC_CLASS(UBoLabelUI, ULabelUI)

UBoLabelUI * UBoLabelUI::m_labelUI = new UBoLabelUI();

std::string UBoLabelUI::m_lafId("ULabel");

UBoLabelUI * UBoLabelUI::createUI(UWidget * w) {
	return m_labelUI;
}

void
UBoLabelUI::paint(UGraphics * g, UWidget * w) {
	UWidgetUI::paint(g, w);

	ULabel * label;
	//const UFont * f;
	//UIcon * icon;
	const UInsets & insets = w->getInsets();

	label = dynamic_cast<ULabel *>(w);
	//f = label->getFont();
	//icon = label->getIcon();
	
	URectangle viewRect(insets.left, insets.top,
		label->getWidth() - insets.getHorizontal(),
		label->getHeight() - insets.getVertical());

	stylePaintCompoundTextAndIcon(g, label, viewRect,
		label->getText(), label->getIcon());
}

const std::string &
UBoLabelUI::getLafId() {
	return m_lafId;
}



UDimension
UBoLabelUI::getPreferredSize(const UWidget * w) {
	const ULabel * label = dynamic_cast<const ULabel *>(w);
	//return UUIUtilities::getCompoundPreferredSize(label);
	return getStyleCompoundPreferredSize(
		label,
		label->getFont(),
		label->getText(),
		label->getIcon()
	);
}

void
UBoLabelUI::stylePaintCompoundTextAndIcon(UGraphics * g, UCompound * w,
		const URectangle & rect, const std::string & text,
		UIcon * icon) {
	// AB: this method is a copy of UGL_Style::paintCompoundTextAndIcon()
	// it is forked so that line wrapping can be implemented


	URectangle viewRect, textRect, iconRect;
	UInsets insets = w->getInsets();
	const UFont * f = w->getFont();
	g->setFont(f);

	viewRect.x = insets.left;
	viewRect.y = insets.top;
	viewRect.w = w->getWidth() - insets.getHorizontal();
	viewRect.h = w->getHeight() - insets.getVertical();

	// due to focus we add some spacing to the view rect

	// AB: this is a problem: viewRect is the size of
	// getCompoundPreferredSize(). however we _depend_ on that size being
	// correct and it must be given to both - layoutCompoundWidget() and to
	// the actual paintControlCaption(). otherwise we have fewer space
	// available than we were promised to have.
	//
	// UPDATE: I worked around this by fixing the "-= 2" in
	// getCompoundPreferredSize(). Johannes: please check if that's ok
	if (w->isFocusable()) {
		viewRect.x += 1;
		viewRect.y += 1;
		viewRect.w -= 2;
		viewRect.h -= 2;
	}

	std::string printText = styleLayoutCompoundWidget(
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
		stylePaintControlCaption(g, w, textRect, printText);
	//}

}

std::string
UBoLabelUI::styleLayoutCompoundWidget(
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

	int gap = (text.length() && iconRect->w) ? textIconGap : 0;

	int maxTextWidth = viewRect->w - iconRect->w - gap;
	if (text.length() != 0) {
		UDimension textDimension;
		clipText = makeCompoundText(text, metrics, maxTextWidth, &textDimension);

		textRect->w = textDimension.w;
		textRect->h = textDimension.h;
	} else {
		textRect->w = textRect->h = 0;
	}

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

void
UBoLabelUI::stylePaintControlCaption(UGraphics * g, UWidget * w,
		const URectangle & rect, const std::string & text)
{
	if (text.size() == 0) {
		// nothing to do
		return;
	}

	// AB: this cuts the string if it is too long, but I want it to get
	// wrapped in that case.
	// this code is obsolete now. line wrapping implemented. this code
	// should be removed.
#if 0
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
#endif

	// set color
	ugl_driver->glColor3fv(w->getColorGroup().foreground().getFloat());

	std::string string = text;
	std::list<std::string> list;
	splitString(text, '\n', &list);

	std::list<std::string>::iterator it;
	int y = rect.y;
	const UFontMetrics* metrics = g->getFont()->getFontMetrics();

	for (it = list.begin(); it != list.end(); ++it) {
		// AB: a lot of stuff in this method (all the line wrapping
		// stuff) could be done in UGraphics. so some other methods
		// might benefit from it

		std::string s = *it;

		// AB: actually the size is not required here. we use it for
		// sanity checks only.
		UDimension size = g->getStringSize(s);
		if (y + size.h > rect.y + rect.h) {
			// oops - layoutCompoundWidget() gave other values to us
			// than we are using here
			// -> what to do now?
			// --> in order to keep other widgets working correctly
			//     (the current one is kinda broken now), we just
			//     ignore the rest of the text.
			//
			// FIXME: should we output an error to stderr?
			// std::cerr << "line too high - layoutCompoundWidget() probably returned too many lines for a too small rect." << std::endl;
			break;
		}
		if (size.w > rect.w + 2) {
			// oops - layoutCompoundWidget() gave an invalid string
			// to us.
			// we cut the rest of the line off - the hard way.
			//
			// FIXME: should we output an error to stderr? it would
			// be useful, cause at this point we have a but that
			// should be fixed, but libufo does not seem to do such
			// things :-(
			// std::cerr << "string too wide. layoutCompoundWidget() gave invalid string." << std::endl;
			unsigned int index = metrics->viewToModel(s, rect.w);
			s = s.substr(0, index + 1);
		}
		g->drawString(s, rect.x, y);
		y += size.h;
	}


	// underline accelerator character
	if (UButton * button = dynamic_cast<UButton*>(w)) {
		int accel = button->getAcceleratorIndex();
		if (accel != -1 && accel < int(text.length())) {
			const UFontMetrics * metrics = g->getFont()->getFontMetrics();

			int y = 0;
			int index = accel;
			std::list<std::string>::iterator it;
			std::string line;
			for (it = list.begin(); it != list.end(); ++it) {
				line = *it;
				if (index > (int)line.length()) {
					// no char in this line is the accel.
					// +1 for the \n which is never accel
					index -= line.length() + 1;
					y += metrics->getHeight();
				} else if (index < (int)line.length()) {
					break;
				} else { // index == (*it).length() -> \n !
					index = accel = -1;
					break;
				}
			}
			if (index >= 0 && index < (int)line.length()) {
				int startx = metrics->getStringWidth(line.data(), index);
				int width = metrics->getCharWidth(line[index]);
				y += metrics->getHeight();
				g->drawLine(rect.x + startx, y, rect.x + startx + width, y);
			}
		}
	}
}

UDimension
UBoLabelUI::getStyleCompoundPreferredSize(
		const UCompound * w,
		const UFont * f,
		const std::string & text,
		const UIcon * icon)
{
	// assume that icon is on the left and text on the right side
	const UFontMetrics * metrics = f->getFontMetrics();

	UDimension textDimension;
	makeCompoundText(text, metrics, -1, &textDimension);

	// warning these are dangerous. e.g. when width is reduced, then we need
	// a greater height!
	int max_width = textDimension.w;
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

	max_height = std::max(textDimension.h, max_height);

	// AB: these "+= 2" are necessary, as paintCompundTextAndIcon()
	// substracts them from our view rect if the widget is focusable.
	// these lines were present already, but by some reason commented out.
	// I left them here so that Johannes can check whether it's correct here
#if 0
	// add a small space for focus drawing
	//max_width += 2;
	//max_height += 2;
#else
	if (w->isFocusable()) {
		max_width += 2;
		max_height += 2;
	}
#endif

	UInsets in = w->getInsets();
	return UDimension(
		max_width + in.getHorizontal(),
		max_height + in.getVertical()
	);
	//return UDimension(max_width, max_height);
}


