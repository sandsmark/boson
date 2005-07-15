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

#include "boufolabel.h"
#include "boufolabel.moc"

#include "boufoimage.h"
#include "boufomanager.h"
#include "boufofontinfo.h"
#include <bodebug.h>

#include <kglobal.h>
#include <kstandarddirs.h>

#include <qimage.h>

QColor BoUfoLabel::mDefaultForegroundColor = Qt::black;

BoUfoLabel::BoUfoLabel() : BoUfoWidget()
{
 init();
}

BoUfoLabel::BoUfoLabel(const QString& text) : BoUfoWidget()
{
 init();
 setText(text);
}

void BoUfoLabel::init()
{
 setLayoutClass(UHBoxLayout);
 mLabel = new ufo::ULabel();
 widget()->add(mLabel);
 mLabel->setOpaque(false);
 setForegroundColor(defaultForegroundColor());
}

void BoUfoLabel::setVerticalAlignment(VerticalAlignment a)
{
 BoUfoWidget::setVerticalAlignment(a);
 mLabel->setVerticalAlignment(widget()->getVerticalAlignment());
}

void BoUfoLabel::setHorizontalAlignment(HorizontalAlignment a)
{
 BoUfoWidget::setHorizontalAlignment(a);
 mLabel->setHorizontalAlignment(widget()->getHorizontalAlignment());
}

void BoUfoLabel::setDefaultForegroundColor(const QColor& c)
{
 mDefaultForegroundColor = c;
}

const QColor& BoUfoLabel::defaultForegroundColor()
{
 return mDefaultForegroundColor;
}

void BoUfoLabel::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mLabel->setOpaque(o);
}

void BoUfoLabel::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mLabel->setMinimumSize(s);
}

void BoUfoLabel::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mLabel->setPreferredSize(s);
}

void BoUfoLabel::setText(const QString& text)
{
 if (text.isNull()) {
	mLabel->setText("");
 } else {
	mLabel->setText(text.latin1());
 }
}

QString BoUfoLabel::text() const
{
 QString text = mLabel->getText().c_str();
 return text;
}

void BoUfoLabel::setIcon(const BoUfoImage& img)
{
 if (!img.image()) {
	mLabel->setIcon(0);
 } else {
	mLabel->setIcon(new ufo::UImageIcon(img.image()));
 }
}

void BoUfoLabel::setIconFile(const QString& file_)
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

QString BoUfoLabel::iconFile() const
{
 return mIconFile;
}

void BoUfoLabel::setFont(BoUfoManager* m, const BoUfoFontInfo& info)
{
 ufo::UFont* font = info.ufoFont(m);

#warning FIXME: memory leak
 // this should be done by ufo::UWidget::setFont() and that one should also
 // unreference it.
 // it is necessary because otherwise the application crashes when rendering due
 // to ufo::UGraphics::setFont().
 font->reference();

#warning FIXME: setFont() is broken
#if 0
 mLabel->setFont(font);
#else
 boError() << k_funcinfo << "is broken" << endl;
#endif
}


