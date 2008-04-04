/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// note the copyright above: this is LGPL!
#ifndef BOUFOFONTINFO_H
#define BOUFOFONTINFO_H

#include <qobject.h>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
template<class T1, class T2> class QMap;
template<class T1> class QValueList;
class QDomElement;

namespace ufo {
	class UFont;
	class UFontInfo;
};


class BoUfoManager;

/**
 * @short Class that stores information about a font
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoFontInfo
{
public:
	enum Styles {
		StyleItalic = 1,
		StyleBold = 2,
		StyleUnderline = 4,
		StyleStrikeOut = 8
	};
public:
	BoUfoFontInfo();
	BoUfoFontInfo(const BoUfoFontInfo&);
	BoUfoFontInfo(const QString& fontPlugin, const ufo::UFontInfo&);

	BoUfoFontInfo& operator=(const BoUfoFontInfo&);

	void setFontPlugin(const QString& plugin) { mFontPlugin = plugin; }
	const QString& fontPlugin() const { return mFontPlugin;}

	void setFamily(const QString& family) { mFamily = family; }
	const QString& family() const { return mFamily; }

	void setPointSize(float s) { mPointSize = s; }
	float pointSize() const { return mPointSize; }

	void setItalic(bool e)
	{
		if (e) {
			mStyle |= StyleItalic;
		} else {
			mStyle &= ~StyleItalic;
		}
	}
	bool italic() const { return mStyle & StyleItalic; }
	void setBold(bool e)
	{
		if (e) {
			mStyle |= StyleBold;
		} else {
			mStyle &= ~StyleBold;
		}
	}
	bool bold() const { return mStyle & StyleBold; }
	void setUnderline(bool e)
	{
		if (e) {
			mStyle |= StyleUnderline;
		} else {
			mStyle &= ~StyleUnderline;
		}
	}
	bool underline() const { return mStyle & StyleUnderline; }
	void setStrikeOut(bool e)
	{
		if (e) {
			mStyle |= StyleStrikeOut;
		} else {
			mStyle &= ~StyleStrikeOut;
		}
	}
	bool strikeOut() const { return mStyle & StyleStrikeOut; }

	/**
	 * Set the style directly using values from the @ref Styles enum.
	 * Ususually you should use @ref setItalic, @ref setBold, ... instead
	 **/
	void setStyle(int style) { mStyle = style; }
	int style() const { return mStyle; }

	void setFixedSize(bool e) { mFixedSize = e; }
	bool fixedSize() const { return mFixedSize; }

	bool operator==(const BoUfoFontInfo& info) const;

	/**
	 * This creates a font according to the properties of this
	 * font. If there is no such font available, libufo will return an
	 * alternative font.
	 *
	 * This font can be used in a ufo widget.
	 **/
	ufo::UFont ufoFont(BoUfoManager* manager) const;

	ufo::UFontInfo ufoFontInfo() const;

	QString debugString() const;

private:
	void init(const QString& fontPlugin, const ufo::UFontInfo& font);

private:
	QString mFontPlugin;
	QString mFamily;
	int mStyle;
	float mPointSize;
	bool mFixedSize;
};



#endif
