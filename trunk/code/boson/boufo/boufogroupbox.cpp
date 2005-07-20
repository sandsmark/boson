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

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufogroupbox.h"
#include "boufogroupbox.moc"

#include <bodebug.h>

BoUfoGroupBox::BoUfoGroupBox()
	: BoUfoWidget(new ufo::UGroupBox())
{
 mGroupBox = (ufo::UGroupBox*)widget();
}

BoUfoGroupBox::BoUfoGroupBox(const QString& title)
	: BoUfoWidget(new ufo::UGroupBox())
{
 mGroupBox = (ufo::UGroupBox*)widget();
 setTitle(title);
}

void BoUfoGroupBox::setTitle(const QString& title)
{
 if (title.isNull()) {
	mGroupBox->setTitle("");
 } else {
	mGroupBox->setTitle(title.latin1());
 }
}

QString BoUfoGroupBox::title() const
{
 QString title = mGroupBox->getTitle().c_str();
 return title;
}

