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

#ifndef NO_OPENGL

#include "bosonglfont.h"

#include <GL/glx.h>

#include <qfont.h>

BosonGLFont::BosonGLFont(const QString& family)
{
 mFont = QFont(family);
 mFontMetrics = new QFontMetrics(mFont);

 // FIXME: i18n() support!
 mFontDisplayList = glGenLists(256);
 glXUseXFont(mFont.handle(), 0, 256, mFontDisplayList);
}

BosonGLFont::~BosonGLFont()
{
 delete mFontMetrics;
}

#endif // !NO_OPENGL
