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

#include "bo3dtools.h"

#include <kdebug.h>

void BoMatrix::loadMatrix(const GLfloat* m)
{
 for (int i = 0; i < 16; i++) {
   mData[i] = m[i];
 }
}

void BoMatrix::loadMatrix(GLenum matrix)
{
 switch (matrix) {
   case GL_MODELVIEW_MATRIX:
   case GL_PROJECTION_MATRIX:
   case GL_TEXTURE_MATRIX:
     break;
   default:
     kdError() << k_funcinfo << "Invalid matrix enum " << (int)matrix << endl;
 }
 glGetFloatv(matrix, mData);
}

void BoMatrix::transform(BoVector3* vector, BoVector3* input)
{
 // v = m * i, m is a 4x4 OpenGL matrix, r and v are both a 3x1 column vector.
 // the forth element is unused in boson and therefore we use silently 0.
#define M(row, col) mData[col * 4 + row] // AB: shamelessy stolen from mesa's  math subdir
#define v(element) vector->mData[element]
#define i(element) input->mData[element]
 v(0) = M(0, 0) * i(0) + M(0, 1) * i(1) + M(0, 2) * i(2) + M(0, 3);
 v(1) = M(1, 0) * i(0) + M(1, 1) * i(1) + M(1, 2) * i(2) + M(1, 3);
 v(2) = M(2, 0) * i(0) + M(2, 1) * i(1) + M(2, 2) * i(2) + M(2, 3);
#undef i
#undef v
#undef M
}

void BoMatrix::transform(BoVector4* vector, BoVector4* input)
{
 // v = m * i, m is a 4x4 OpenGL matrix, r and v are both a 3x1 column vector.
 // the forth element is unused in boson and therefore we use silently 0.
#define M(row, col) mData[col * 4 + row] // AB: shamelessy stolen from mesa's  math subdir
#define v(element) vector->mData[element]
#define i(element) input->mData[element]
 v(0) = M(0, 0) * i(0) + M(0, 1) * i(1) + M(0, 2) * i(2) + M(0, 3);
 v(1) = M(1, 0) * i(0) + M(1, 1) * i(1) + M(1, 2) * i(2) + M(1, 3);
 v(2) = M(2, 0) * i(0) + M(2, 1) * i(1) + M(2, 2) * i(2) + M(2, 3);
 v(3) = M(3, 0) * i(0) + M(3, 1) * i(1) + M(3, 2) * i(2) + M(3, 3);
#undef i
#undef v
#undef M
}

void BoMatrix::debugMatrix(const GLfloat* m)
{
 kdDebug() << k_funcinfo << endl;
 for (int i = 0; i < 4; i++) {
   kdDebug() << QString("%1 %2 %3 %4").arg(m[i]).arg(m[i + 4]).arg(m[i + 8]).arg(m[i + 12]) << endl;
 }
 kdDebug() << k_funcinfo << "done" << endl;
}


/*
 * vim:et sw=2
 */
