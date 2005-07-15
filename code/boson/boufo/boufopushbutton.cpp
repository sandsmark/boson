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

#include "boufopushbutton.h"
#include "boufopushbutton.moc"

#include "boufoimage.h"
#include <bodebug.h>

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qimage.h>

#include <math.h>

BoUfoPushButton::BoUfoPushButton()
	: BoUfoWidget(new ufo::UButton())
{
 init();
}

BoUfoPushButton::BoUfoPushButton(const QString& text)
	: BoUfoWidget(new ufo::UButton())
{
 init();
 setText(text);
}

void BoUfoPushButton::init()
{
 mButton = (ufo::UButton*)widget();

 CONNECT_UFO_TO_QT(BoUfoPushButton, mButton, Activated);
 CONNECT_UFO_TO_QT(BoUfoPushButton, mButton, Highlighted);

 // allow clicking the button + enable highlight effect
 setMouseEventsEnabled(true, true);
 setKeyEventsEnabled(true);

 // TODO:
 // it would be nice if we could use setOpaque(false), but still have a border
 // around the button.
 // additionally we should provide some kind of default color (like for labels)
 setOpaque(true);
}

void BoUfoPushButton::setVerticalAlignment(VerticalAlignment a)
{
 BoUfoWidget::setVerticalAlignment(a);
 mButton->setVerticalAlignment(widget()->getVerticalAlignment());
}

void BoUfoPushButton::setHorizontalAlignment(HorizontalAlignment a)
{
 BoUfoWidget::setHorizontalAlignment(a);
 mButton->setHorizontalAlignment(widget()->getHorizontalAlignment());
}

void BoUfoPushButton::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mButton->setOpaque(o);
}

void BoUfoPushButton::uslotActivated(ufo::UActionEvent*)
{
 emit signalActivated();
 emit signalClicked();
}

void BoUfoPushButton::uslotHighlighted(ufo::UActionEvent*)
{
 emit signalHighlighted();
}

void BoUfoPushButton::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mButton->setMinimumSize(s);
}

void BoUfoPushButton::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mButton->setPreferredSize(s);
}

void BoUfoPushButton::setText(const QString& text)
{
 if (text.isNull()) {
	mButton->setText("");
 } else {
	mButton->setText(text.latin1());
 }
}

QString BoUfoPushButton::text() const
{
 QString text = mButton->getText().c_str();
 return text;
}

void BoUfoPushButton::setIcon(const BoUfoImage& img)
{
 if (!img.image()) {
	mButton->setIcon(0);
 } else {
	mButton->setIcon(new ufo::UImageIcon(img.image()));
 }
}

void BoUfoPushButton::setIconFile(const QString& file_)
{
 QString file = file_;
 if (!file_.isEmpty()) {
	QImage img;
	if (KGlobal::_instance) { // NULL in boufodesigner
		file = locate("data", "boson/" + file_);
		if (file.isEmpty()) {
			boDebug() << k_funcinfo << "file " << file_ << " not found" << endl;
			file = file_;
		}
	}
	if (!img.load(file)) {
		boError() << k_funcinfo << file << " could not be loaded" << endl;
		return;
	}
	setIcon(img);
 } else {
	setIcon(BoUfoImage());
 }
 mIconFile = file;
}

QString BoUfoPushButton::iconFile() const
{
 return mIconFile;
}

void BoUfoPushButton::setToggleButton(bool t)
{
 mButton->setToggable(t);
}

bool BoUfoPushButton::isToggleButton() const
{
 return mButton->isToggable();
}

void BoUfoPushButton::setOn(bool on)
{
 mButton->setSelected(on);
}

bool BoUfoPushButton::isOn() const
{
 return mButton->isSelected();
}


