/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#define QT_CLEAN_NAMESPACE // we don't use QT-1 syntax anyway. if we don't define this INT32 in qnamespace.h conflicts with the one in Xmd.h included by **nvidia** glx.h
#include "bosonglfont.h"

#include "../../bomemory/bodummymemory.h"
#include "fnt.h"

#include "bodebug.h"
#include <boglx.h>

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <X11/Xlib.h>
#include <math.h>

#include <qfont.h>
#include <qstringlist.h>
#include <qpaintdevice.h>
#include <QX11Info>

BoFontInfo::BoFontInfo()
{
 mBold = false;
 mItalic = false;
 mPointSize = 10;
 mFixedPitch = false;
 mUnderline = false;
 mStrikeOut = false;
 mTextured = true;

 QStringList list;
 if (KGlobal::hasMainComponent()) {
	list = KGlobal::dirs()->findAllResources("data", "boson/fonts/*.txf");
 }
 if (!list.isEmpty()) {
	QStringList list2 = list.filter("AvantGarde-Demi.txf");
	if (!list2.isEmpty()) {
		mName = list2[0];
	} else {
		mName = list[0];
	}
 } else {
	mTextured = false;
	mName = QString::null;
 }
}

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
 QStringList l = s.split(QChar(','));
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

	/**
	 * newlines ('\n') are not allowed in @p string.
	 **/
	virtual int renderString(int x, int y, const GLubyte* string, unsigned int len) = 0;

	/**
	 * @return The width of the widest char
	 **/
	virtual int widestChar() const = 0;

	/**
	 * @return The width of @p text. Any occurances of newlines ('\n') in @p
	 * text are ignored.
	 **/
	int width(const QString& text) const
	{
		QByteArray tmp = text.toLatin1();
		return width((GLubyte*)tmp.constData(), tmp.length());
	}
	/**
	 * @return The width of @p string . Any occurances of newlines ('\n') in @p
	 * string are ignored.
	 * @param len The length of @p string.
	 **/
	virtual int width(const GLubyte* string, int len) const = 0;

	/**
	 * @return The height of a line.
	 **/
	virtual int height() const = 0;

	virtual void setPointSize(int s) = 0;
	virtual void setItalic(bool i) = 0;

	/**
	 * @return The maximal position of a char in @p string where the width
	 * of the string is still <= maxWidth. @p string can be rendered up to
	 * that position without exceeding @p maxWidth.
	 **/
	virtual int maxWidthPos(const GLubyte* string, int len, int maxWidth) const = 0;
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

	virtual int width(const GLubyte* string, int len) const
	{
		Q_UNUSED(string);
		return len * widestChar();
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

	virtual int maxWidthPos(const GLubyte* string, int len, int maxWidth) const
	{
		// AB: note that width(string, len) == len * widestChar() !
		int pos = (maxWidth / widestChar()) - 1;
		if (pos >= len) {
			pos = len - 1;
		}
		return pos;
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
	char** names = XListFonts(QX11Info::display(), "*", 0xffff, &count);
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

	// '\n's are disallowed in string!
	virtual int width(const GLubyte* string, int len) const
	{
		if (!mFont) {
			return 0;
		}
		float w = 0.0f;
		for (int i = 0; i < len; i++) {
			char c = string[i];
			w += ceilf(mFont->getWidth(c, mPointSize));
			if (c != ' ') {
				w += mFont->getGap() * mPointSize;
			}
			if (c == '\n') {
				boWarning() << k_funcinfo << "\\n is not allowed in the text parameter" << endl;
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

	virtual int maxWidthPos(const GLubyte* string, int len, int maxWidth) const
	{
		for (int pos = len - 1; pos > 0; pos--) {
			int w = width(string, pos + 1);
			if (w <= maxWidth) {
				return pos;
			}
		}
		return 1; // render _always_ at least one char per line, no matter what maxWidth is. avoids dangerous infinite loops.
	}
private:
	BofntTexFont* mFont;
	float mPointSize;
	bool mItalic;
};

BoTXFFont::~BoTXFFont()
{
 delete mFont;
}

bool BoTXFFont::loadFont(const QString& fileName)
{
 mFont = new BofntTexFont();
 mFont->setGap(0.0f);
 QByteArray tmp = fileName.toLatin1();
 if (mFont->load(tmp.constData(), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR) != FNT_TRUE) {
	boError() << k_funcinfo << "could not load txf font " << fileName << endl;
	return false;
 }
 return true;
}

int BoTXFFont::renderString(int x, int y, const GLubyte* string, unsigned int len)
{
 if (!mFont) {
	return 0;
 }
 glPushAttrib(GL_COLOR_BUFFER_BIT);
 glEnable(GL_ALPHA_TEST);
 glEnable(GL_BLEND);
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
// boDebug() << k_funcinfo << "creating font for " << ((font.name().isEmpty()) ? QString("(empty name)") : font.name()) << endl;
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
 int wmax = 0;
 QStringList lines = text.split('\n');
 QStringList::Iterator it;
 for (it = lines.begin(); it != lines.end(); ++it) {
	int w = mFont->width(*it);
	if (w > wmax) {
		wmax = w;
	}
 }
 return wmax;
}

int BosonGLFont::lineWidth(const GLubyte* text, int len) const
{
 return mFont->width(text, len);
}

int BosonGLFont::wrapAtPos(const GLubyte* string, int length) const
{
 for (int i = length - 1; i > 0; i--) {
	if (string[i] == ' ') {
		return i;
	}
 }
 return length - 1; // could not find a good place to wrap. wrap at the maximum width.
}

// return a len, so that width(string[0]..string[len-1]) <= maxWidth.
// string must not contain '\n'
int BosonGLFont::makeLine(const GLubyte* string, int len, int maxWidth) const
{
 // maxPos is the maximal length of the string with width < maxWidth
 int maxPos = mFont->maxWidthPos(string, len, maxWidth);

 int wrapPos;
 if (maxPos < len - 1) {
	// try to wrap after a ' ' or so (not inside a word)
	wrapPos = wrapAtPos(string, maxPos + 1);
 } else {
	// we won't wrap at all
	wrapPos = maxPos;
 }

 return wrapPos + 1; // return a length, not a pos.
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
 if (text.contains('\n')) {
	int h = 0;
	QStringList lines = text.split('\n');
	QStringList::Iterator it = lines.begin();
	for (; it != lines.end(); ++it) {
		h += height(*it, maxWidth);
	}
	return h;
 }
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
 QByteArray tmp = text.toLatin1();
 GLubyte* string = (GLubyte*)tmp.constData();
 const int len = tmp.length();
 int pos = 0;
 int lines = 0;
 while (pos < len) {
	int l = makeLine(string + pos, len - pos, maxWidth);
	if (l == 0) {
		// we cannot display a single char in this line?!
		boWarning() << k_funcinfo << "unable to process whole string. makeLine() returned 0." << endl;
		return lines;
	}
	pos += l;
	lines++;
 }
 return lines * height();
}

int BosonGLFont::renderText(int x, int y, const QString& text, int maxWidth, bool background)
{
 if (text.isEmpty()) {
	return 0;
 }
 int heightSum = 0;
 if (x < 0) {
	x = 0;
 }
 if (y < 0) {
	y = 0;
 }
 QStringList lines = text.split('\n');
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
 if (text.isEmpty()) {
	return 0;
 }
 int w = width(text);
 if (w >= maxWidth) {
	w = maxWidth;
 }
 QByteArray tmp = text.toLatin1();
 const int len = tmp.length();
 GLubyte* string = (GLubyte*)tmp.constData();
 // we must never ever use more height than height(..) claims we do
 int maxHeight = height(text, maxWidth);
 if (background) {
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
	glEnable(GL_BLEND);
#warning fixme: texturemanager
#if 0
	boTextureManager->disableTexturing();
#endif
	glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	glRecti(x, y - maxHeight, x + w, y);
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
					<< "oops - invalid height! h=" << h
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

