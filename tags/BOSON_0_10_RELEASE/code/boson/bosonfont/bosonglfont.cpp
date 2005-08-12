/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "fnt.h"

#include "../bosonglwidget.h"
#include "bodebug.h"

#include <klocale.h>

#include <GL/glx.h>
#include <X11/Xlib.h>
#include <math.h>

#include <qfont.h>
#include <qstringlist.h>

QString BoFontInfo::toString() const
{
 QString s;
 QString name = this->name();
 if (name.contains(QChar(','))) {
	boError() << k_funcinfo << "font name contains a comma - won't save correct font!" << endl;
	name = QString::null;
 }
 s = QString::number((int)textured()) + QChar(',')
	+ QString::number((int)bold()) + QChar(',')
	+ QString::number((int)italic()) + QChar(',')
	+ QString::number(pointSize()) + QChar(',')
	+ QString::number((int)fixed()) + QChar(',')
	+ QString::number((int)underline()) + QChar(',')
	+ QString::number((int)strikeOut()) + QChar(',')
	+ name;
 return s;
}

bool BoFontInfo::fromString(const QString& s)
{
 if (s.isEmpty()) {
	// default font
	*this = BoFontInfo();
	return true;
 }
 QStringList l = QStringList::split(QChar(','), s);
 if (l.count() != 8) {
	boError() << k_funcinfo << "invalid font string " << s << endl;
	return false;
 }
 setTextured((bool)l[0].toInt());
 setBold((bool)l[1].toInt());
 setItalic((bool)l[2].toInt());
 setPointSize(l[3].toInt());

 mFixedPitch = (bool)l[4].toInt();
 mUnderline = (bool)l[5].toInt();
 mStrikeOut = (bool)l[6].toInt();
 mName = l[7];
 return true;
}

QString BoFontInfo::guiName() const
{
 if (name().isNull()) {
	return i18n("Default");
 }
 return name();
}


class BoFont {
public:
	BoFont()
	{
	}
	virtual ~BoFont()
	{
	}
	virtual void begin() = 0;

	virtual int renderString(int x, int y, const GLubyte* string, unsigned int len) = 0;

	virtual int widestChar() const = 0;
	virtual int width(const QString& text) const = 0;
	virtual int height() const = 0;
	virtual void setPointSize(int s) = 0;
	virtual void setItalic(bool i) = 0;
};

class BoGLXFont : public BoFont{
public:
	BoGLXFont()
	{
		mFontMetrics = 0;
		mFontDisplayList = 0;
	}
	~BoGLXFont();

	bool loadFont(const QString& family);

	virtual void begin()
	{
		glListBase(mFontDisplayList);
	}

	// string must not contain \0 or \n!
	virtual int renderString(int x, int y, const GLubyte* string, unsigned int len);

	virtual int widestChar() const
	{
		if (!mFontMetrics) {
			return 0;
		}
		return mFontMetrics->maxWidth();
	}

	virtual int width(const QString& text) const
	{
		return text.length() * widestChar();
	}
	virtual int height() const
	{
		return mFontMetrics->height();
	}
	virtual void setPointSize(int s)
	{
		Q_UNUSED(s);
	}
	virtual void setItalic(bool i)
	{
		Q_UNUSED(i);
	}

private:
	QFont mFont;
	QFontMetrics* mFontMetrics;
	GLuint mFontDisplayList;
};

BoGLXFont::~BoGLXFont()
{
 delete mFontMetrics;
 glDeleteLists(mFontDisplayList, 256);
}

bool BoGLXFont::loadFont(const QString& family)
{
 mFont = QFont(family);
 mFont.setStyleHint(QFont::AnyStyle, QFont::PreferBitmap);
 mFont.setFixedPitch(true); // necessary on some systems. we support fixed size fonts only anyway.

 int handle = (int)mFont.handle();

 if (handle == 0) {
	boError() << k_funcinfo << "qt could not load any bitmap font!" << endl;
	int count;
	char** names = XListFonts(QPaintDevice::x11AppDisplay(), "*", 0xffff, &count);
	for (int i = 0; i < count && handle == 0; i++) {
		mFont.setRawName(names[i]);
		handle = (int)mFont.handle();
	}
	XFreeFontNames(names);
 }
 if (handle == 0) {
	boError() << k_funcinfo << "no bitmap font found. cannot display GL text" << endl;
 } else {
	// FIXME: i18n() support!
	mFontDisplayList = glGenLists(256);
	glXUseXFont((Font)handle, 0, 256, mFontDisplayList);
	boDebug() << k_funcinfo << family << " font created. display lists base=" << mFontDisplayList << endl;
 }
 delete mFontMetrics;
 mFontMetrics = new QFontMetrics(mFont);
 return true;
}

int BoGLXFont::renderString(int x, int y, const GLubyte* string, unsigned int len)
{
 glRasterPos2i(x, y);
 glCallLists(len, GL_UNSIGNED_BYTE, string);
 return len;
}

class BoTXFFont : public BoFont {
public:
	BoTXFFont()
	{
		mFont = 0;
		mPointSize = 10.0f;
		mItalic = false;
	}
	~BoTXFFont();

	virtual void begin()
	{
		if (!mFont) {
			return;
		}
		mFont->begin();

	}
	bool loadFont(const QString& fileName);

	// string must not contain \0 or \n!
	virtual int renderString(int x, int y, const GLubyte* string, unsigned int len);

	// '\n's are disallowed in text!
	virtual int width(const QString& text) const
	{
		if (!mFont) {
			return 0;
		}
		float w = 0.0f;
		for (unsigned int i = 0; i < text.length(); i++) {
			char c = text[i];
			w += mFont->getWidth(c, mPointSize);
			if (c != ' ') {
				w += mFont->getGap() * mPointSize;
			}
		}
		return (int)ceilf(w);
	}
	virtual int height() const
	{
		return (int)mPointSize;
	}
	virtual int widestChar() const
	{
		if (!mFont) {
			return 0;
		}
		return (int)ceilf((mFont->getWidestChar() + mFont->getGap()) * mPointSize);
	}
	virtual void setPointSize(int s)
	{
		mPointSize = (float)s;
	}
	virtual void setItalic(bool i)
	{
		mItalic = i;
	}

private:
	fntTexFont* mFont;
	float mPointSize;
	bool mItalic;
};

BoTXFFont::~BoTXFFont()
{
 delete mFont;
}

bool BoTXFFont::loadFont(const QString& fileName)
{
 mFont = new fntTexFont();
 if (mFont->load(fileName.latin1()) != FNT_TRUE) {
	boError() << k_funcinfo << "could not load txf font " << fileName << endl;
	return false;
 }
 mFont->setGap(0.0f);
 return true;
}

int BoTXFFont::renderString(int x, int y, const GLubyte* string, unsigned int len)
{
 if (!mFont) {
	return 0;
 }
 glPushAttrib(GL_COLOR_BUFFER_BIT);
 glEnable(GL_ALPHA_TEST);
// glEnable(GL_BLEND);
 glAlphaFunc(GL_GREATER, 0.1f);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 float pos[3] = { (float)x, (float)y, 0.0f };
 for (unsigned int i = 0; i < len; i++) {
	mFont->putch(pos, mPointSize, mItalic ? 1.0f : 0.0f, (char)string[i]);
 }
 glPopAttrib();
 return len;
}

BosonGLFont::BosonGLFont(const BoFontInfo& font)
{
 boDebug() << k_funcinfo << "creating font for " << font.name() << endl;
 if (!BoContext::currentContext()) {
	boError() << k_funcinfo << "NULL current context" << endl;
	return;
 }
 mFont = 0;

 mFontInfo = font;

 if (!loadFont(mFontInfo)) {
	boError() << k_funcinfo << "could not load font " << mFontInfo.name() << endl;
 }
}

BosonGLFont::BosonGLFont()
{
 mFont = 0;
}

BosonGLFont::BosonGLFont(const QString& family)
{
 boDebug() << k_funcinfo << "creating font for " << family << endl;
 if (!BoContext::currentContext()) {
	boError() << k_funcinfo << "NULL current context" << endl;
	return;
 }
 mFont = 0;

 mFontInfo.setName(family);

 if (!loadFont(mFontInfo)) {
	boError() << k_funcinfo << "could not load font " << mFontInfo.name() << endl;
 }
}

BosonGLFont::~BosonGLFont()
{
 delete mFont;
}

bool BosonGLFont::loadFont(const BoFontInfo& font)
{
 bool ret = false;
 BoFontInfo f = font;
 if (f.textured()) {
	ret = loadTXFFont(f);
	if (!ret) {
		boError() << k_funcinfo << "unable to load textured font " << f.name() << ". reverting to default GLX font" << endl;
		f.setName(QString::null);
		f.setTextured(false);
		ret = loadGLXFont(f);
	}
 } else {
	ret = loadGLXFont(f);
	if (!ret) {
		boError() << k_funcinfo << "unable to load GLX font " << f.name() << ". reverting to default GLX font" << endl;
		f.setName(QString::null);
		ret = loadGLXFont(f);
	}
 }
 if (!ret) {
	boError() << k_funcinfo << "unable to load font" << endl;
	return false;
 }
 mFontInfo = f;
 return true;
}

bool BosonGLFont::loadGLXFont(const BoFontInfo& font)
{
 if (!BoContext::currentContext()) {
	boError() << k_funcinfo << "NULL current context" << endl;
	return false;
 }
 BoGLXFont* f = new BoGLXFont();
 if (!f->loadFont(font.name())) {
	delete f;
	return false;
 }
 delete mFont;
 mFont = f;
 return true;
}

bool BosonGLFont::loadTXFFont(const BoFontInfo& font)
{
 if (!BoContext::currentContext()) {
	boError() << k_funcinfo << "NULL current context" << endl;
	return false;
 }
 BoTXFFont* f = new BoTXFFont();
 if (!f->loadFont(font.name())) {
	delete f;
	return false;
 }
 delete mFont;
 mFont = f;
 mFont->setPointSize(font.pointSize());
 mFont->setItalic(font.italic());
 return true;
}

const BoFontInfo& BosonGLFont::fontInfo() const
{
 return mFontInfo;
}

void BosonGLFont::begin()
{
 if (mFont) {
	mFont->begin();
 }
}

int BosonGLFont::width(const QString& text)
{
 if (!mFont) {
	return 0;
 }
 return mFont->width(text);
}

int BosonGLFont::wrapAtPos(const GLubyte* string, int length) const
{
 return length;
}

int BosonGLFont::makeLine(const GLubyte* string, int len, int width) const
{
 int maxW = widestChar();
 if (maxW == 0) {
	// returning 0 could result in infinite loops...
	// 30 is a random value. 1 would suck (only 1 char per line) and result
	// in long loops.
	return 30;
 }
 if (len * maxW < width) {
	return len;
 }
 // TODO: search a white space in the text where we can wrap
 return width / maxW;
}

int BosonGLFont::widestChar() const
{
 if (!mFont) {
	return 0;
 }
 return mFont->widestChar();
}

int BosonGLFont::height(const QString& text, int maxWidth)
{
 if (widestChar() <= 0) {
	return 0;
 }
 int w = width(text);
 if (w < maxWidth) {
	return height();
 }
 if (maxWidth < widestChar()) {
	// we can't even render a single char!
	return 0;
 }
 GLubyte* string = (GLubyte*)text.latin1();
 const int len = text.length();
 int pos = 0;
 int lines = 0;
 while (pos < len) {
	// we can always render at least maxPos chars per line.
	int maxPos = maxWidth / widestChar();
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
// renderer.begin();
 for (it = lines.begin(); it != lines.end(); ++it) {
	heightSum += renderLine(x, y - heightSum, *it, maxWidth, background);
 }
// renderer.end();
 return heightSum;
}

int BosonGLFont::height() const
{
 if (!mFont) {
	return 0;
 }
 return mFont->height();
}

int BosonGLFont::renderLine(int x, int y, const QString& text, int maxWidth, bool background)
{
 int w = width(text);
 const int len = text.length();
 GLubyte* string = (GLubyte*)text.latin1();
 // we must never ever use more height than height(..) claims we do
 int maxHeight = height(text, maxWidth);
 if (background) {
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	glRecti(x, y - maxHeight, x + w, y);
	glColor3ub(255, 255, 255);
	glPopAttrib();
 }
 if (w < maxWidth) {
	renderStringInternal(x, y - maxHeight, string, len);
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
		renderStringInternal(x, y - h, string + pos, l);
		pos += l;
		line++;
	}
 }
 return maxHeight;
}

int BosonGLFont::renderStringInternal(int x, int y, const GLubyte* string, unsigned int len)
{
 if (!mFont) {
	return 0;
 }
 return mFont->renderString(x, y, string, len);
}
