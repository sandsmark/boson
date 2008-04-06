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

#include "boufotextedit.h"
#include "boufotextedit.moc"

BoUfoTextEdit::BoUfoTextEdit() : BoUfoWidget()
{
 init();
}

void BoUfoTextEdit::init()
{
 setLayoutClass(UHBoxLayout);
 mTextEdit = new ufo::UTextEdit();
 ufoWidget()->add(mTextEdit);
}

void BoUfoTextEdit::setEditable(bool e)
{
 mTextEdit->setEditable(e);
}

bool BoUfoTextEdit::isEditable() const
{
 return mTextEdit->isEditable();
}

void BoUfoTextEdit::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mTextEdit->setOpaque(o);
}

void BoUfoTextEdit::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mTextEdit->setMinimumSize(s);
}

void BoUfoTextEdit::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mTextEdit->setPreferredSize(s);
}

void BoUfoTextEdit::setText(const QString& text)
{
 if (text.isNull()) {
	mTextEdit->setText("");
 } else {
	QByteArray tmp = text.toAscii();
	mTextEdit->setText(std::string(tmp.constData(), tmp.length()));
 }
}

QString BoUfoTextEdit::text() const
{
 QString text = mTextEdit->getText().c_str();
 return text;
}


