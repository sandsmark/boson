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

#ifndef BOSONGLFONT_H
#define BOSONGLFONT_H

#include <bogl.h>

#include <qfontmetrics.h>

class BoFont;

class BoFontInfo {
public:
	BoFontInfo();
	BoFontInfo(const BoFontInfo& f)
	{
		*this = f;
	}

	BoFontInfo& operator=(const BoFontInfo& f)
	{
		mName = f.name();
		mBold = f.bold();
		mItalic = f.italic();
		mPointSize = f.pointSize();
		mTextured = f.textured();
		mFixedPitch = f.fixed();
		mUnderline = f.underline();
		mStrikeOut = f.strikeOut();
		return *this;
	}

	bool isEqual(const BoFontInfo& f) const
	{
		if (bold() != f.bold()) {
			return false;
		}
		if (italic() != f.italic()) {
			return false;
		}
		if (pointSize() != f.pointSize()) {
			return false;
		}
		if (textured() != f.textured()) {
			return false;
		}
		if (fixed() != f.fixed()) {
			return false;
		}
		if (underline() != f.underline()) {
			return false;
		}
		if (strikeOut() != f.strikeOut()) {
			return false;
		}
		if (name() != f.name()) {
			return false;
		}
		return true;
	}

	bool operator==(const BoFontInfo& f)
	{
		return isEqual(f);
	}

	QString toString() const;
	bool fromString(const QString& s);

	void setTextured(bool t) { mTextured = t; }
	bool textured() const { return mTextured; }

	/**
	 * @param name The family for GLX fonts, the filename (absolute) for
	 * textured fonts.
	 **/
	void setName(const QString& name) { mName = name; }
	const QString& name() const { return mName; }

	/**
	 * @return just like @ref name, but well suited to be displayed in a GUI
	 * (e.g. QString::null will be "Default")
	 **/
	QString guiName() const;

	void setPointSize(int p) { mPointSize = p; }
	int pointSize() const { return mPointSize; }

	void setBold(bool b) { mBold = b; }
	bool bold() const { return mBold; }

	void setItalic(bool i) { mItalic = i; }
	bool italic() const { return mItalic; }

	bool fixed() const { return mFixedPitch; }
	bool underline() const { return mUnderline; }
	bool strikeOut() const { return mStrikeOut; }

private:
	QString mName;
	bool mBold;
	bool mItalic;
	int mPointSize;
	bool mTextured;
	bool mFixedPitch;
	bool mUnderline;
	bool mStrikeOut;
};

/**
 * We store a font in this class with a few other values, like width and height.
 * We can use these cached values in order to prevent loading the metrics in
 * paintGL()
 * You can use the @ref displayList of BosonGLFont to easily render text in
 * a GL widget. An example use:
 * Call in BosonGLWidget::initializeGL():
 * <pre>
 * BosonGLFont* glFont = new BosonGLFont("fixed");
 * </pre>
 * and then in you BosonGLWidget::paintGL():
 * <pre>
 * QString text = QString::fromLatin1();
 * glListBase(bosonGLFont->displayList());
 * glCallLists(text.length(), GL_UNSIGNED_BYTE, (GLubyte*)text.latin1());
 * </pre>
 * @short Implementation of fonts for OpenGL using glXUseXFont
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGLFont
{
public:
	/**
	 * Create an NULL font. Do not use this before calling @ref loadFont !
	 **/
	BosonGLFont();

	/**
	 * Generate a display list containing the specified (GLX) font.
	 *
	 * Note that non-latin1 characters might be buggy!
	 *
	 * Also note that you must call @ref BosonGLWidget::makeCurrent
	 * before constructing a BosonGLFont
	 *
	 * This constructor is kinda obsolete and supports GLX fonts only.
	 * You should prefer the other constructor(s) instead.
	 **/
	BosonGLFont(const QString& family);

	/**
	 * Create a font that is described by @p font.
	 *
	 * See also @ref loadFont
	 **/
	BosonGLFont(const BoFontInfo& font);
	~BosonGLFont();

	/**
	 * Load a font described by @p font. If the font could not be loaded
	 * this tries to load a default GLX font (even if you wanted a textured
	 * font!). See @ref fontInfo to find out which font got actually loaded.
	 *
	 * Note that you must call @ref BoContext::makeCurrent or @ref
	 * BosonGLWidget::makeCurrent <em>before</em> trying to load the font!
	 *
	 * @return TRUE If a font was loaded (either the requested font or a
	 * fallback font), FALSE if no font could get loaded.
	 **/
	bool loadFont(const BoFontInfo& font);

	const BoFontInfo& fontInfo() const;

	/**
	 * Start rendering a font. Call this once before you render the fonts -
	 * it will do necessary stuff, such as setting the display list base and
	 * binding the texture.
	 **/
	void begin();

	int height() const;
	int width(const QString& text);

	/**
	 * @return The width of @p string, which must not contain any newlines.
	 **/
	int lineWidth(const GLubyte* string, int len) const;

	/**
	 * @return The width of the widest char in this font
	 **/
	int widestChar() const;

	/**
	 * @return The height of the text as it will be rendered by @ref
	 * renderText (including line wraps)
	 **/
	int height(const QString& text, int maxWidth);

	/**
	 * Warning: this function will change the glRasterPos()! (not
	 * necessarily to (x,y) !)
	 * @param x The left position of the text
	 * @param y The position directly <em>above</em> the text. This function
	 * won't render text above y, but below only. This is in OpenGL
	 * window-coordinates, that means that 0 is bottom.
	 * @param maxWidth The maximal amount of space the text is allowed to
	 * take. Text that is wider than this is automatically wrapped. If you
	 * don't have special requirements for your text you should simply use
	 * @ref BosonGLWidget::width - @p x
	 * @param background If TRUE this will add an alpha blended background
	 * of the text so that it is even visible if the background color of the
	 * screen is the same as the text color.
	 * @return The height that was needed to render the text. See also @ref
	 * height
	 **/
	int renderText(int x, int y, const QString& text, int maxWidth, bool background = true);

	/**
	 * @internal
	 * This is public only for libufo support. Don't use it in normal boson
	 * code! Use @ref renderText instead!
	 **/
	int renderStringInternal(int x, int y, const GLubyte* string, unsigned int len);


protected:
	bool loadGLXFont(const BoFontInfo& font);
	bool loadTXFFont(const BoFontInfo& font);

	/**
	 * Make a line that is no longer than @width (opengl coordinates, not
	 * characters) for @p font from @p string.
	 * @param len The length of the @p string.
	 * @return The number of characters that can be rendered without
	 * exceeding @p width.
	 **/
	int makeLine(const GLubyte* string, int len, int width) const;

	/**
	 * @return A position in @p string where wrapping should occur. This
	 * tries to find spaces in order to avoid wrapping at a word.
	 *
	 * @param string The string to wrap. It should not contain newlines
	 * ('\n').
	 **/
	int wrapAtPos(const GLubyte* string, int length) const;


	int renderLine(int x, int y, const QString& text, int maxWidth, bool background = true);

private:
	BoFont* mFont;
	BoFontInfo mFontInfo;
};

#endif // BOSONGLFONT_H

