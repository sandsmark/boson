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

#ifndef BOSONGLFONT_H
#define BOSONGLFONT_H

#include <GL/gl.h>

#include <qfontmetrics.h>

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
	 * Generate a display list containing the specified font.
	 *
	 * Note that non-latin1 characters might be buggy!
	 *
	 * Also note that you must call @ref BosonGLWidget::makeCurrent to make
	 * before constructing a BosonGLFont
	 **/
	BosonGLFont(const QString& family);
	~BosonGLFont();
	inline QFontMetrics* metrics() const { return mFontMetrics; }
	inline int height() const { return mFontMetrics->height(); }
	int width(const QString& text);

	/**
	 * @return The height of the text as it will be rendered by @ref
	 * renderText (including line wraps)
	 **/
	int height(const QString& text, int maxWidth);

	/**
	 * @return The base of the OpenGL display lists.
	 **/
	inline GLuint displayList() const { return mFontDisplayList; }

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

protected:
	/**
	 * Make a line that is no longer than @width (opengl coordinates, not
	 * characters) for @p font from @p string.
	 * @param len The length of the @p string.
	 * @return The number of characters that can be rendered without
	 * exceeding @p width.
	 **/
	int makeLine(const GLubyte* string, int len, int width) const;

	/**
	 * @return The position in @p string where to wrap. Currently this
	 * implementation will wrap at length-1, i.e. the last char.
	 **/
	int wrapAtPos(const GLubyte* string, int length) const;

	int renderLine(int x, int y, const QString& text, int maxWidth, bool background = true);

private:
	QFont mFont;
	QFontMetrics* mFontMetrics;
	GLuint mFontDisplayList; 
};

#endif // BOSONGLFONT_H

