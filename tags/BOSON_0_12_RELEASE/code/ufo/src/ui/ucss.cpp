/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/ucss.cpp
    begin             : Wed Mar 02 2005
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

#include "ufo/ui/ucss.hpp"

#include <fstream>

#include "ufo/ui/ustylehints.hpp"
#include "ufo/umodel.hpp"
#include "ufo/udisplay.hpp"

#include "ufo/udrawable.hpp"
#include "ufo/image/uimage.hpp"
#include "ufo/image/uimageicon.hpp"
#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontinfo.hpp"

using namespace ufo;

UCss::UCss() {}
UCss::UCss(const std::string & filename) {
	load(filename);
}

UCss::~UCss() {
	for (std::map<std::string, UStyleHints*>::iterator iter = m_hints.begin();
			iter != m_hints.end();
			++iter) {
		if ((*iter).second) {
			delete ((*iter).second);
		}
	}
	m_hints.clear();
}

void
UCss::load(const std::string & filename) {
	std::ifstream stream(filename.c_str());
	load(stream);
}


int
ufo_css_eatWhitespaces(std::istream & stream) {
	uint8_t tempChar = stream.peek();
	int ret = 0;

	// ignore leading whitespaces
	while (/*std::*/isspace(tempChar)) {
		stream.ignore(1);
		tempChar = stream.peek();
		ret++;
	}
	return ret;
}

// Checks whether a comment is following.
// If yes, eats it and returns true.
// Otherwise false.
bool
ufo_css_eatComment(std::istream & stream) {
	char comment1 = 0;
	char comment2 = 0;
	bool comment_eaten = false;

	ufo_css_eatWhitespaces(stream);
	if (stream) stream >> comment1;

	if (comment1 == '/') {
		if (stream) stream >> comment2;
		if (comment2 == '*') {
			comment1 = 0;
			comment2 = 0;
			comment_eaten = true;
			do {
				do {
					if (stream) stream >> comment1;
				} while (comment1 != '*' && stream);
				if (stream) stream >> comment2;
			} while (comment1 != '*' && comment2 != '/' && stream);
		} else {
			stream.putback(comment2);
		}
	} else {
		stream.putback(comment1);
	}
	return comment_eaten;
}

// eats comment lines and empty lines
void
ufo_css_eatComments(std::istream & stream) {
	bool eat_comment = true;
	while(eat_comment) {
		eat_comment = ufo_css_eatComment(stream);
	}
}

void
UCss::load(std::istream & stream) {
	std::string blockName;

	while (stream) {
		ufo_css_eatComments(stream);
		if (stream) stream >> blockName;

		ufo_css_eatWhitespaces(stream);
		char blockStart = 0;
		if (stream) stream >> blockStart;

		if (blockStart == '{') {
			m_hints[blockName] = parseBlock(stream);
		}
	}
}

bool
goToNextArg(std::istream & stream) {
	char trash = 0;
	// current arg
	while (trash != ' ' && trash != ';' && trash != '"' && stream) {
		stream.get(trash);
	}
	if (trash == ';') {
		return false;
	}
	ufo_css_eatWhitespaces(stream);
	return true;
}

void
eatTrailing(std::istream & stream) {
	// clean px, em, ex, ;
	char trash = 0;
	while (trash != ' ' && trash != ';' && trash != '"' && stream) {
		stream.get(trash);
	}
}

int
readInt(std::istream & stream) {
	int ret = 0;
	stream >> ret;

	eatTrailing(stream);
	return ret;
}

float
readFloat(std::istream & stream) {
	float ret = 0;
	stream >> ret;

	eatTrailing(stream);
	return ret;
}

UColor
readColor(std::istream & stream) {
	ufo_css_eatWhitespaces(stream);
	std::string color;
	stream >> color;

	if (color.size() && color[color.size() - 1] == ';') {
		color.erase(color.end() - 1);
		stream.putback(';');
	}

	eatTrailing(stream);
	return UColor(color);
}

UFontInfo::Family
readFontFamily(std::istream & stream) {
	ufo_css_eatWhitespaces(stream);
	std::string family;
	stream >> family;
	if (family.size() && family[family.size() - 1] == ';') {
		family.erase(family.end() -1);
	}
	if (family == "serif") {
		return UFontInfo::Serif;
	} else if (family == "courier") {
		return UFontInfo::MonoSpaced;
	} else if (family == "sans-serif") {
		return UFontInfo::SansSerif;
	}
	return UFontInfo::DefaultFamily;
}


UImage *
readImage(std::istream & stream) {
	ufo_css_eatWhitespaces(stream);
	char url[] = {0, 0, 0, 0, 0, 0, 0};
	if (stream) stream.get(url, 6);

	UImage * ret = NULL;
	if (std::string("url('") == url) {
		// FIXME: is 64 byte enough space?
		char buffer[64];
		stream.get(buffer, 64, '\'');

		std::string fileName(buffer, stream.gcount());
		ret = UDisplay::getDefault()->createImage(fileName);
	}

	eatTrailing(stream);
	return ret;
}

UIcon *
readImageIcon(std::istream & stream) {
	ufo_css_eatWhitespaces(stream);
	char url[] = {0, 0, 0, 0, 0, 0, 0};
	if (stream) stream.get(url, 6);

	UIcon * ret = NULL;
	if (std::string("url('") == url) {
		// FIXME: is 64 byte enough space?
		char buffer[64];
		stream.get(buffer, 64, '\'');

		std::string fileName(buffer, stream.gcount());
		ret = new UImageIcon(fileName);;
	}

	eatTrailing(stream);
	return ret;
}

uint32_t
stringToBorderStyle(const std::string & str) {
	std::string text(str);
	if (text.size() && text[text.size() - 1] == ';') {
		text.erase(text.end() - 1);
	}
	if (text == "solid") {
		return BorderSolid;
	}
	if (text == "dotted") {
		return BorderDotted;
	}
	if (text == "dashed") {
		return BorderDashed;
	}
	if (text == "double") {
		return BorderDouble;
	}
	if (text == "groove") {
		return BorderGroove;
	}
	if (text == "ridge") {
		return BorderRidge;
	}
	if (text == "inset") {
		return BorderInset;
	}
	if (text == "outset") {
		return BorderOutset;
	}
	return NoBorderStyle;
}

void
readBorderStyle(std::istream & stream, UBorderModel * model) {
	std::string r;
	stream >> r;
	model->style[0] = model->style[1] = model->style[2]
		= model->style[3] = stringToBorderStyle(r);

	if (goToNextArg(stream)) {
		stream >> r;
		model->style[1] = model->style[3] = stringToBorderStyle(r);
	}
	if (goToNextArg(stream)) {
		stream >> r;
		model->style[2] = stringToBorderStyle(r);
	}
	if (goToNextArg(stream)) {
		stream >> r;
		model->style[3] = stringToBorderStyle(r);
	}
}

UInsets
readInsets(std::istream & stream) {
	UInsets ret;

	int val = 0;
	stream >> val;
	ret.top = ret.right = ret.bottom = ret.left = val;
	if (!goToNextArg(stream)) return ret;

	stream >> val;
	ret.right = ret.left = val;
	if (!goToNextArg(stream)) return ret;

	stream >> val;
	ret.bottom = val;
	if (!goToNextArg(stream)) return ret;

	stream >> val;
	ret.left = val;

	eatTrailing(stream);
	return ret;
}

UStyleHints *
UCss::parseBlock(std::istream & stream, UStyleHints * hints) {
	UStyleHints * ret = hints;
	if (!ret) {
		ret = new UStyleHints();
	}
	std::string key;
	UFontInfo info;
	while (stream && key != "}") {
		stream >> key;
		if (key == "width:") {
			ret->preferredSize.w = readInt(stream);
		} else if (key == "height:") {
			ret->preferredSize.h = readInt(stream);
		} else if (key == "min-width:") {
			ret->minimumSize.w = readInt(stream);
		} else if (key == "min-height:") {
			ret->minimumSize.h = readInt(stream);
		} else if (key == "max-width:") {
			ret->maximumSize.w = readInt(stream);
		} else if (key == "max-height:") {
			ret->maximumSize.w = readInt(stream);
		} else if (key == "margin:") {
			ret->margin = readInsets(stream);
		} else if (key == "margin-top:") {
			ret->margin.top = readInt(stream);
		} else if (key == "margin-right:") {
			ret->margin.right = readInt(stream);
		} else if (key == "margin-bottom:") {
			ret->margin.bottom = readInt(stream);
		} else if (key == "margin-left:") {
			ret->margin.left = readInt(stream);
		} else if (key == "opacity:") {
			ret->opacity = readFloat(stream);
		} else if (key == "color:") {
			ret->palette.setColor(UPalette::Foreground, readColor(stream));
		} else if (key == "background-color:") {
			ret->palette.setColor(UPalette::Background, readColor(stream));
		} else if (key == "background-image:") {
			ret->background = readImage(stream);
		} else if (key == "font-size:") {
			info.pointSize = readInt(stream);
		} else if (key == "font-weight:") {
			//info = readInt(stream);
		} else if (key == "font-family:") {
			info.family = readFontFamily(stream);
		} else if (key == "list-style-image:") {
			ret->icon = readImageIcon(stream);
		}
		ufo_css_eatComments(stream);
	}
	if (info != UFontInfo()) {
		ret->font = UFont(info);
	}
	return ret;
}

UStyleHints *
UCss::getStyleHints(const std::string & key) {
	return m_hints[key];
}

std::map<std::string, UStyleHints*>
UCss::getStyleHints() {
	return m_hints;
}
