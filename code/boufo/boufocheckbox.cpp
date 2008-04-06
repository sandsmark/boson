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

#include "boufocheckbox.h"
#include "boufocheckbox.moc"

#include "boufolabel.h"
#include <bodebug.h>

BoUfoCheckBox::BoUfoCheckBox()
	: BoUfoWidget(new ufo::UCheckBox())
{
 init();
}

BoUfoCheckBox::BoUfoCheckBox(const QString& text, bool checked)
	: BoUfoWidget(new ufo::UCheckBox())
{
 init();
 setText(text);
 setChecked(checked);
}

void BoUfoCheckBox::init()
{
 setLayoutClass(UHBoxLayout);
 mCheckBox = (ufo::UCheckBox*)ufoWidget();
 // AB: at least the background of the label must be transparent. unfortunately,
 // libufo uses UButton for the checkbox, so the actual checkbox and its label
 // are the same widget
 mCheckBox->setOpaque(false);

 setForegroundColor(BoUfoLabel::defaultForegroundColor());

 CONNECT_UFO_TO_QT(BoUfoCheckBox, mCheckBox, Activated);
 CONNECT_UFO_TO_QT(BoUfoCheckBox, mCheckBox, Highlighted);

 setMouseEventsEnabled(true, true);
}

void BoUfoCheckBox::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mCheckBox->setOpaque(o);
}

void BoUfoCheckBox::uslotActivated(ufo::UActionEvent*)
{
 emit signalActivated();
 emit signalToggled(checked());
}

void BoUfoCheckBox::uslotHighlighted(ufo::UActionEvent*)
{
 emit signalHighlighted();
}

void BoUfoCheckBox::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mCheckBox->setMinimumSize(s);
}

void BoUfoCheckBox::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mCheckBox->setPreferredSize(s);
}

void BoUfoCheckBox::setText(const QString& text)
{
 if (text.isNull()) {
	mCheckBox->setText("");
 } else {
	QByteArray tmp = text.toAscii();
	mCheckBox->setText(std::string(tmp.constData(), tmp.length()));
 }
}

QString BoUfoCheckBox::text() const
{
 QString text = mCheckBox->getText().c_str();
 return text;
}

void BoUfoCheckBox::setChecked(bool c)
{
 mCheckBox->setSelected(c);
}

bool BoUfoCheckBox::checked() const
{
 return mCheckBox->isSelected();
}

