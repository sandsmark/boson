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

#include "boufolabel.h"
#include "boufolabel.moc"

#include "boufoimage.h"
#include "boufodrawable.h"
#include "boufomanager.h"
#include "boufofontinfo.h"
#include "boufomanager.h"
#include "ufoext/ubolabel.h"
#include "ufoext/ubodrawableicon.h"
#include <bodebug.h>

#include <kglobal.h>
#include <kstandarddirs.h>

#include <qimage.h>

QColor BoUfoLabel::mDefaultForegroundColor = Qt::black;

BoUfoLabel::BoUfoLabel()
	: BoUfoWidget(new ufo::UBoLabel())
{
 init();
}

BoUfoLabel::BoUfoLabel(const QString& text)
	: BoUfoWidget(new ufo::UBoLabel())
{
 init();
 setText(text);
}

void BoUfoLabel::init()
{
 setLayoutClass(UHBoxLayout);
 mLabel = (ufo::UBoLabel*)ufoWidget();
 mLabel->setOpaque(false);
 setForegroundColor(defaultForegroundColor());
 setKeyEventsEnabled(false);
}

void BoUfoLabel::setVerticalAlignment(VerticalAlignment a)
{
 BoUfoWidget::setVerticalAlignment(a);
 mLabel->setVerticalAlignment(ufoWidget()->getVerticalAlignment());
}

void BoUfoLabel::setHorizontalAlignment(HorizontalAlignment a)
{
 BoUfoWidget::setHorizontalAlignment(a);
 mLabel->setHorizontalAlignment(ufoWidget()->getHorizontalAlignment());
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
 if (img.isNull()) {
	mLabel->setIcon(0);
 } else {
	mLabel->setIcon(new ufo::UBoDrawableIcon(img.image()));
 }
}

void BoUfoLabel::setIcon(const BoUfoDrawable& drawable)
{
 if (!drawable.drawable()) {
	mLabel->setIcon(0);
 } else {
	mLabel->setIcon(new ufo::UBoDrawableIcon(drawable.drawable()));
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
	} else if (BoUfoManager::currentUfoManager()) {
		QString dataDir = BoUfoManager::currentUfoManager()->dataDir();
		if (!dataDir.isEmpty()) {
			file = dataDir + "/" + file_;
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

