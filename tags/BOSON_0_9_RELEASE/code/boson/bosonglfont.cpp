/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define QT_CLEAN_NAMESPACE // we don't use QT-1 syntax anyway. if we don't define this INT32 in qnamespace.h conflicts with the one in Xmd.h included by **nvidia** glx.h
#include "bosonglfont.h"

#include "bosonglwidget.h"
#include "bodebug.h"

#include <GL/glx.h>

#include <qfont.h>
#include <qstringlist.h>

BosonGLFont::BosonGLFont(const QString& family)
{
 boDebug() << k_funcinfo << "creating font for " << family << endl;
 if (!BoContext::currentContext()) {
	boError() << k_funcinfo << "NULL current context" << endl;
	mFontMetrics = 0;
	return;
 }
 mFont = QFont(family);
 mFont.setStyleHint(QFont::AnyStyle, QFont::PreferBitmap);
 mFontMetrics = new QFontMetrics(mFont);

 // FIXME: i18n() support!
 mFontDisplayList = glGenLists(256);
 glXUseXFont((Font)mFont.handle(), 0, 256, mFontDisplayList);
 boDebug() << k_funcinfo << family << " font created" << endl;
}

BosonGLFont::~BosonGLFont()
{
 glDeleteLists(mFontDisplayList, 256);
 delete mFontMetrics;
}

int BosonGLFont::width(const QString& text)
{
 if (!mFontMetrics) {
	return 0;
 }
 // AB: we use the widest char of the font only. may be .. bad. but at least it
 // is fast and easy to use.
 return text.length() * metrics()->maxWidth();
}

int BosonGLFont::wrapAtPos(const GLubyte* string, int length) const
{
 return length - 1;
}

int BosonGLFont::makeLine(const GLubyte* string, int len, int width) const
{
 if (!metrics()) {
	// returning 0 could result in infinite loops...
	// 30 is a random value. 1 would suck (only 1 char per line) and result
	// in long loops.
	return 30;
 }
 if (len * metrics()->maxWidth() < width) {
	return len;
 }
 // TODO: search a white space in the text where we can wrap
 return width / metrics()->maxWidth();
}

int BosonGLFont::height(const QString& text, int maxWidth)
{
 int w = width(text);
 if (w < maxWidth) {
	return height();
 }
 GLubyte* string = (GLubyte*)text.latin1();
 const int len = text.length();
 int pos = 0;
 int lines = 0;
 while (pos < len) {
	// we can always render at least maxPos chars per line.
	int maxPos = maxWidth / metrics()->maxWidth();
	int wrapPos;
	if (maxPos > len - pos) {
		maxPos = len - pos;
		wrapPos = maxPos;
	} else {
		wrapPos = wrapAtPos(string + pos, maxPos);
	}
	pos += wrapPos;
	lines++;
 }
 return lines * height();
}

int BosonGLFont::renderText(int x, int y, const QString& text, int maxWidth, bool background)
{
 int heightSum = 0;
 if (x < 0) {
	x = 0;
 }
 if (y < 0) {
	y = 0;
 }
 QStringList lines = QStringList::split('\n', text);
 QStringList::Iterator it;
 for (it = lines.begin(); it != lines.end(); ++it) {
	heightSum += renderLine(x, y - heightSum, *it, maxWidth, background);
 }
 return heightSum;
}

int BosonGLFont::renderLine(int x, int y, const QString& text, int maxWidth, bool background)
{
 int w = width(text);
 const int len = text.length();
 GLubyte* string = (GLubyte*)text.latin1();
 // we must never ever use more height than height(..) claims we do
 int maxHeight = height(text, maxWidth);
 if (background) {
	glEnable(GL_BLEND);
	glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	glRecti(x, y - maxHeight, x + w, y);
	glDisable(GL_BLEND);
	glColor3ub(255, 255, 255);
 }
 if (w < maxWidth) {
	glRasterPos2i(x, y - maxHeight);
	glCallLists(len, GL_UNSIGNED_BYTE, string);
 } else {
	int h = 0;
	int pos = 0;
	int line = 0;
	while (pos < len) {
		int l;
		l = BosonGLFont::makeLine(string + pos, len - pos, maxWidth);
		if (l <= 0) {
			// oops!
			// must never happen!
			l = len - pos;
		}
		h = (line + 1) * height();
		if (h > maxHeight) {
			boError() << k_funcinfo
					<< "oops - invalid height! h= " << h
					<< " maxHeight=" << maxHeight << endl;
			h = maxHeight;
		}
		glRasterPos2i(x, y - h);
		glCallLists(l, GL_UNSIGNED_BYTE, string + pos);
		pos += l;
		line++;
	}
 }
 return maxHeight;
}

