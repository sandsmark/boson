/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufomatrix.h"
#include "boufomatrix.moc"

#include "boufolabel.h"
#include <bodebug.h>

BoUfoMatrix::BoUfoMatrix() : BoUfoWidget()
{
 init();
}

BoUfoMatrix::~BoUfoMatrix()
{
 // AB: remember NOT to delete the mMatrix[i] elements. libufo does so. we just
 // need to delete the array containing the pointers.
 delete[] mMatrix;
}

void BoUfoMatrix::init()
{
 setLayoutClass(UVBoxLayout);

 BoUfoHBox* rows[4];
 for (int i = 0; i < 4; i++) {
	rows[i] = new BoUfoHBox();
	addWidget(rows[i]);
 }

 mMatrix = new BoUfoLabel*[16];
 for (int i = 0; i < 16; i++) {
	mMatrix[i] = new BoUfoLabel();
	rows[i % 4]->addWidget(mMatrix[i]);
 }
}

void BoUfoMatrix::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 for (int i = 0; i < 16; i++) {
	mMatrix[i]->setOpaque(o);
 }
}

void BoUfoMatrix::setMatrix(const float* m)
{
 for (int i = 0; i < 16; i++) {
	mMatrix[i]->setText(QString::number(m[i]));
 }
}

