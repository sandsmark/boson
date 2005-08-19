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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>
#include <ufo/ux/ux.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufostandalonefont.h"
#include "boufofontinfo.h"

#include "boufomanager.h"
#include "bodebug.h"

BoUfoStandaloneFont::BoUfoStandaloneFont(BoUfoManager* manager)
{
 mFont = new ufo::UFont();
 mFontInfo = new BoUfoFontInfo();
 *mFontInfo = manager->globalFont();
 *mFont = mFontInfo->ufoFont(manager);
 if (mFont->getRenderer()) {
	mMetrics = mFont->getRenderer()->getFontMetrics();
 } else {
	mMetrics = 0;
 }
}

BoUfoStandaloneFont::BoUfoStandaloneFont(const BoUfoFontInfo& info, BoUfoManager* manager)
{
 mFont = new ufo::UFont();
 mFontInfo = new BoUfoFontInfo();
 *mFontInfo = info;
 *mFont = mFontInfo->ufoFont(manager);
 if (mFont->getRenderer()) {
	mMetrics = mFont->getRenderer()->getFontMetrics();
 } else {
	mMetrics = 0;
 }
}

BoUfoStandaloneFont::BoUfoStandaloneFont(const BoUfoStandaloneFont& font)
{
 *this = font;
}

BoUfoStandaloneFont::~BoUfoStandaloneFont()
{
 delete mFont;
 delete mFontInfo;
}

BoUfoStandaloneFont& BoUfoStandaloneFont::operator=(const BoUfoStandaloneFont& font)
{
 *mFont = *font.mFont;
 *mFontInfo = *font.mFontInfo;
 if (mFont->getRenderer()) {
	mMetrics = mFont->getRenderer()->getFontMetrics();
 } else {
	mMetrics = 0;
 }
 return *this;
}

const BoUfoFontInfo& BoUfoStandaloneFont::fontInfo() const
{
 return *mFontInfo;
}

void BoUfoStandaloneFont::drawString(const QString& string, int x, int y)
{
 // AB: note that UFontRenderer requires string not to contain any newlines!

 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 BO_CHECK_NULL_RET(tk);
 ufo::UContext* c = tk->getCurrentContext();
 BO_CHECK_NULL_RET(c);
 ufo::UGraphics* g = c->getGraphics();
 BO_CHECK_NULL_RET(g);
 BO_CHECK_NULL_RET(mFont->getRenderer());

 mFont->getRenderer()->drawString(g, string.latin1(), string.length(), x, y);
}

int BoUfoStandaloneFont::height() const
{
 if (!mMetrics) {
	return 0;
 }
 return mMetrics->getHeight();
}

int BoUfoStandaloneFont::stringWidth(const QString& string) const
{
 if (!mMetrics) {
	return 0;
 }
 return mMetrics->getStringWidth(string.latin1(), string.length());
}


