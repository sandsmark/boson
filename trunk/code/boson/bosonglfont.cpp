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
 return metrics()->width(text);
}
