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

#ifndef NO_OPENGL

#include <GL/gl.h>

#include <qfontmetrics.h>

/**
 * We store a font in this class with a few other values, like width and height.
 * We can use these cached values in order to prevent loading the metrics in
 * paintGL()
 * You can use the @ref displayList of BosonGLFont to easily render text in
 * a QGL widget. An example use:
 * Call in QGLWidget::initializeGL():
 * <pre>
 * BosonGLFont* glFont = new BosonGLFont("fixed");
 * </pre>
 * and then in you QGLWidget::paintGL():
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
	 * Also note that you must call @ref QGLWidget::makeCurrent to make
	 * before constructing a BosonGLFont
	 **/
	BosonGLFont(const QString& family);
	~BosonGLFont();
	inline QFontMetrics* metrics() const { return mFontMetrics; }
	inline int height() const { return mFontMetrics->height(); }

	/**
	 * @return The base of the OpenGL display lists.
	 **/
	inline GLuint displayList() const { return mFontDisplayList; }

private:
	QFont mFont;
	QFontMetrics* mFontMetrics;
	GLuint mFontDisplayList; 
};

#endif // !NO_OPENGL

#endif // BOSONGLFONT_H
