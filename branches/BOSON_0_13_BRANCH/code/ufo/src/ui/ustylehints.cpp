/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/ustylehints.cpp
    begin             : Thu Mar 3 2005
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

#include "ufo/ui/ustylehints.hpp"

#include "ufo/font/ufont.hpp"

#include "ufo/umodel.hpp"

using namespace ufo;


UStyleHints::UStyleHints()
	: minimumSize(UDimension::invalid)
	, maximumSize(UDimension::invalid)
	, preferredSize(UDimension::invalid)
	, border(new UBorderModel())
	, margin()
	, hAlignment(AlignNone)
	, vAlignment(AlignNone)
	, direction(NoDirection)
	, orientation(NoOrientation)
	, font()
	, palette()
	, opacity(1.0f)
	, background(NULL)
	, icon(NULL)
{
	border->borderType = 0;
	border->width[0] = 1;
	border->width[1] = 1;
	border->width[2] = 1;
	border->width[3] = 1;
	border->style[0] = 0;
	border->style[1] = 0;
	border->style[2] = 0;
	border->style[3] = 0;
	border->radius[0] = 0;
	border->radius[1] = 0;
	border->radius[2] = 0;
	border->radius[3] = 0;
}

UStyleHints::~UStyleHints() {
	delete (border);
}

void
UStyleHints::transcribe(UStyleHints * hints) {
	if (!hints) {
		return;
	}

	minimumSize.transcribe(hints->minimumSize);
	maximumSize.transcribe(hints->maximumSize);
	preferredSize.transcribe(hints->preferredSize);
	// FIXME:
	if (hints->border->borderType != NoBorder) {
		border->borderType = hints->border->borderType;
	}
	for (int i = 0; i < 4; ++i) {
		if (hints->border->color[i] != UColor()) {
			border->color[i] = hints->border->color[i];
		}
		if (hints->border->style[i] != NoBorderStyle) {
			border->style[i] = hints->border->style[i];
		}
		if (hints->border->width[i] != 0) {
			border->width[i] = hints->border->width[i];
		}
		if (hints->border->radius[i] != 0) {
			border->radius[i] = hints->border->radius[i];
		}
	}
	if (hints->margin != UInsets())
		margin = hints->margin;
	if (hints->hAlignment != AlignNone)
		hAlignment = hints->hAlignment;
	if (hints->vAlignment != AlignNone)
		vAlignment = hints->vAlignment;
	if (hints->direction != Up)
		direction = hints->direction;
	if (hints->orientation != Horizontal)
		orientation = hints->orientation;
	if (hints->direction != Up)
		direction = hints->direction;
	if (hints->font.getRenderer() != UFont().getRenderer())
		font = hints->font;
	palette.transcribe(hints->palette);
	if (hints->opacity != 1.0f)
		opacity = hints->opacity;
	if (hints->background != NULL)
		background = hints->background;
	if (hints->icon != NULL)
		icon = hints->icon;
}

void
UStyleHints::update(UStyleHints * hints) {
	if (!hints) {
		std::cerr << "oops, no style hints\n";
		return;
	}
	minimumSize.update(hints->minimumSize);
	maximumSize.update(hints->maximumSize);
	preferredSize.update(hints->preferredSize);
	// FIXME:
	if (border->borderType == NoBorder) {
		border->borderType = hints->border->borderType;
	}
	for (int i = 0; i < 4; ++i) {
		if (border->color[i] == UColor()) {
			border->color[i] = hints->border->color[i];
		}
		if (border->style[i] == NoBorderStyle) {
			border->style[i] = hints->border->style[i];
		}
		if (border->width[i] == 0) {
			border->width[i] = hints->border->width[i];
		}
		if (border->radius[i] == 0) {
			border->radius[i] = hints->border->radius[i];
		}
	}/*
		delete (border);
		border = new UBorderModel(*(hints->border));
	}*/
	if (margin == UInsets())
		margin = hints->margin;
	if (hAlignment == AlignNone)
		hAlignment = hints->hAlignment;
	if (vAlignment == AlignNone)
		vAlignment = hints->vAlignment;
	if (direction == Up)
		direction = hints->direction;
	if (orientation == Horizontal)
		orientation = hints->orientation;
	if (direction == Up)
		direction = hints->direction;
	if (font.getRenderer() == UFont().getRenderer())
		font = hints->font;
	palette.update(hints->palette);
	if (opacity == 1.0f)
		opacity = hints->opacity;
	if (background == NULL)
		background = hints->background;
	if (icon == NULL)
		icon = hints->icon;
}

UStyleHints *
UStyleHints::clone() const {
	UStyleHints * ret = new UStyleHints(*this);
	ret->border = new UBorderModel(*(this->border));
	return ret;
}

std::ostream &
ufo::operator<<(std::ostream & os, const UStyleHints * hints) {
	if (!hints) {
		return os << "NULL";
	}
	return os << "UStyleHints["
	<< "min size=" << hints->minimumSize
	<< "; max size=" << hints->maximumSize
	<< "; pref size=" << hints->preferredSize
	<< "; border=" << hints->border->borderType
	<< "; margin=" << hints->margin
	<< "; halign=" << hints->hAlignment
	<< "; valign=" << hints->vAlignment
	<< "; dir=" << hints->direction
	<< "; orient=" << hints->orientation
	<< "; opacity=" << hints->opacity
	<< "; bg=" << hints->background
	<< "]";
}
