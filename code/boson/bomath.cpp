/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

// note the copyright above: this is LGPL!

#include "bomath.h"

#include <stdio.h>

#include <qdatastream.h> // I don't like this very much here, but it is necessary

void floatToBin(float _f)
{
 printf("%f: ", _f);
 int f = *((int*)&_f);
 for (int i = 31; i >= 0; i--) {
	if (f & (0x1 << i)) {
		printf("1");
	} else {
		printf("0");
	}
 }
 int sign = (f & (0x1 << 31)) ? 1 : 0;
 int e = 0;
 float m = 0;
 for (int i = 30; i >= 23; i--) {
	e <<= 1;
	if (f & (0x1 << i)) {
		e += 1;
	}
 }
 for (int i = 0; i < 23; i++) {
	m /= 2.0f;
	if (f & (0x1 << i)) {
		m += (0.5f);
	}
 }

 // ieee stuff. mantissa always has a leading 1, exponent has a bias of 127
 m += 1.0f;
 e -= 127;
 printf(" - sign=%d, e=%d, m=%f", sign, e, m);
 printf("\n");
}

QDataStream& operator<<(QDataStream& stream, const bofixed& f)
{
 stream << (Q_INT32)f.rawInt();
 return stream;
}

QDataStream& operator>>(QDataStream& stream, bofixed& f)
{
 Q_INT32 v;
 stream >> v;
 f.setFromRawInt(v);
 return stream;
}

