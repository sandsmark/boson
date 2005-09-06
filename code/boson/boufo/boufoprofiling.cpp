/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "boufoprofiling.h"

BoUfoProfiling* BoUfoProfiling::mProfiling = 0;
int BoUfoProfiling::mRefCount = 0;

void BoUfoProfiling::setProfiling(BoUfoProfiling* p)
{
 delete mProfiling;
 mProfiling = p;
}

void BoUfoProfiling::reference()
{
 mRefCount++;
}

void BoUfoProfiling::unreference()
{
 mRefCount--;
 if (mRefCount < 0) {
	// Oops - should never happen
	mRefCount = 0;
 }
 if (mRefCount == 0) {
	delete mProfiling;
	mProfiling = 0;
 }
}

