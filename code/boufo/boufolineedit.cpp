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

#include "boufolineedit.h"
#include "boufolineedit.moc"

#include <bodebug.h>


BoUfoLineEdit::BoUfoLineEdit() : BoUfoWidget()
{
 init();
}

void BoUfoLineEdit::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mLineEdit->setMinimumSize(s);
}

void BoUfoLineEdit::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mLineEdit->setPreferredSize(s);
}

void BoUfoLineEdit::init()
{
 setLayoutClass(UHBoxLayout);
 mLineEdit = new ufo::ULineEdit();
 ufoWidget()->add(mLineEdit);

 CONNECT_UFO_TO_QT(BoUfoLineEdit, mLineEdit, Activated);
}

void BoUfoLineEdit::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mLineEdit->setOpaque(o);
}

void BoUfoLineEdit::setEditable(bool e)
{
 mLineEdit->setEditable(e);
}

bool BoUfoLineEdit::isEditable() const
{
 return mLineEdit->isEditable();
}

void BoUfoLineEdit::uslotActivated(ufo::UActionEvent*)
{
 emit signalActivated();
 emit signalActivated(text());
}

void BoUfoLineEdit::setText(const QString& text)
{
 if (text.isNull()) {
	mLineEdit->setText("");
 } else {
	QByteArray tmp = text.toAscii();
	mLineEdit->setText(std::string(tmp.constData(), tmp.length()));
 }
}

QString BoUfoLineEdit::text() const
{
 QString text = mLineEdit->getText().c_str();
 return text;
}


