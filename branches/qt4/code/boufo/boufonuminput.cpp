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

#include "boufonuminput.h"
#include "boufonuminput.moc"

#include "boufolabel.h"
#include "boufolineedit.h"
#include "boufoslider.h"

#include <bodebug.h>

BoUfoNumInput::BoUfoNumInput() : BoUfoWidget()
{
 init();
}

void BoUfoNumInput::init()
{
 setLayoutClass(UHBoxLayout);

 mLabel = new BoUfoLabel();

 mSlider = new BoUfoSlider();
 connect(mSlider, SIGNAL(signalFloatValueChanged(float)),
		this, SLOT(slotSliderChanged(float)));

 // TODO: allow only float inputs. there is a
 // UDocumentFactory::createDigitDocument(), but that one allows digits only,
 // i.e. only integers
 mLineEdit = new BoUfoLineEdit();
 // TODO: do not only accept input when return was pressed, but also after a
 // short timeout!
 connect(mLineEdit, SIGNAL(signalActivated(const QString&)),
		this, SLOT(slotTextEntered(const QString&)));

 // TODO: a spinbox would be nice (i.e. up/down arrows next to the lineedit)

 addWidget(mLabel);
 addWidget(mSlider);
 addWidget(mLineEdit);

 setRange(0.0f, 100.0f);
 setValue(50.0f);
}

void BoUfoNumInput::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mLabel->setOpaque(o);
 mSlider->setOpaque(o);
 mLineEdit->setOpaque(o);
}

void BoUfoNumInput::slotSliderChanged(float v)
{
 static bool recursive = false;
 if (recursive) {
	return;
 }
 recursive = true;
 setValue(v);
 recursive = false;

 emit signalValueChanged(v);
}

void BoUfoNumInput::slotTextEntered(const QString& text)
{
 bool ok;
 float v = text.toFloat(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "invalid text entered (not a float)" << endl;
	v = value();
 }
 setValue(v);

 emit signalValueChanged(v);
}

float BoUfoNumInput::value() const
{
 float v = mLineEdit->text().toDouble();
 v = qMax(v, minimumValue());
 v = qMin(v, maximumValue());
 return v;
}

float BoUfoNumInput::minimumValue() const
{
 return mSlider->minimumValue();
}

float BoUfoNumInput::maximumValue() const
{
 return mSlider->maximumValue();
}

float BoUfoNumInput::stepSize() const
{
 return mSlider->stepSize();
}

void BoUfoNumInput::setLabel(const QString& label, int a)
{
 mLabel->setText(label);

#warning TODO: a
 // alignment should be pretty easy using a BorderLayout
}

QString BoUfoNumInput::label() const
{
 return mLabel->text();
}

void BoUfoNumInput::setValue(float v)
{
 mSlider->setFloatValue(v);
 mLineEdit->setText(QString::number(mSlider->floatValue()));
}

void BoUfoNumInput::slotSetMaxValue(float max)
{
 double min = qMin(max, minimumValue());
 setRange(min, max);
}

void BoUfoNumInput::slotSetMinValue(float min)
{
 double max = qMax(min, maximumValue());
 setRange(min, max);
}

void BoUfoNumInput::setStepSize(float s)
{
 mSlider->setFloatRange(mSlider->minimumValue(), mSlider->maximumValue(), s);
}

void BoUfoNumInput::setRange(float min, float max)
{
 if (min > max) {
	boError() << k_funcinfo << "min > max" << endl;
	min = max;
 }
 mSlider->setFloatRange(min, max, mSlider->stepSize());

 if (value() < minimumValue()) {
	setValue(minimumValue());
 } else if (value() > maximumValue()) {
	setValue(maximumValue());
 }
}


