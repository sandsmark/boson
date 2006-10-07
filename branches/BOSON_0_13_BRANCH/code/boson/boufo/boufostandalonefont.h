/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!
#ifndef BOUFOSTANDALONEFONT_H
#define BOUFOSTANDALONEFONT_H

namespace ufo {
	class UFont;
	class UFontInfo;
	class UFontMetrics;
};


class BoUfoManager;
class BoUfoFontInfo;


// AB: TODO: get rid of the BoUfoManager requirement
/**
 * @short A font class for rendering custom strings without BoUfo widgets
 *
 * Usually it is easiest to use a @ref BoUfoLabel or a @ref BoUfoTextEdit in
 * readonly mode to draw text. However sometimes it may be necessary to draw
 * text onto the screen without using such a widget. In such a case this class
 * may be an easy solution.
 *
 * WARNING: you need to delete an object of this class on destruction yourself!
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoStandaloneFont
{
public:
	BoUfoStandaloneFont(const BoUfoFontInfo&, BoUfoManager* manager);
	BoUfoStandaloneFont(BoUfoManager*);
	BoUfoStandaloneFont(const BoUfoStandaloneFont&);
	~BoUfoStandaloneFont();

	BoUfoStandaloneFont& operator=(const BoUfoStandaloneFont&);

	const BoUfoFontInfo& fontInfo() const;

	/**
	 * Draw a string
	 * @param string The string to draw. Must not contain newlines!
	 **/
	void drawString(const QString& string, int x = 0, int y = 0);

	int height() const;
	int stringWidth(const QString& string) const;

private:
	BoUfoFontInfo* mFontInfo;
	ufo::UFont* mFont;
	const ufo::UFontMetrics* mMetrics;
};

#endif
