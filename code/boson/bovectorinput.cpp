/*
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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

// note the copyright above: this is LGPL!

#include "bovectorinput.h"
#include "bovectorinput.moc"

#include "../bomemory/bodummymemory.h"
#include "bonuminput.h"
#include "bo3dtools.h"

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <kdialog.h>

class BoVector3InputPrivate
{
public:
	BoVector3InputPrivate()
	{
		mLabel = 0;
		mTopLayout = 0;
		mMainLayout = 0;

		mX = 0;
		mY = 0;
		mZ = 0;
	}
	QLabel* mLabel;
	Q3VBoxLayout* mTopLayout;
	Q3HBoxLayout* mMainLayout;

	BoFloatNumInput* mX;
	BoFloatNumInput* mY;
	BoFloatNumInput* mZ;
};

BoVector3Input::BoVector3Input(QWidget* parent, const char* name) : QWidget(parent, name)
{
 d = new BoVector3InputPrivate;

 d->mTopLayout = new Q3VBoxLayout(this);
 d->mMainLayout = new Q3HBoxLayout(d->mTopLayout);

 d->mX = new BoFloatNumInput(this);
 d->mY = new BoFloatNumInput(this);
 d->mZ = new BoFloatNumInput(this);
 connect(d->mX, SIGNAL(signalValueChanged(float)), this, SLOT(slotValueChanged(float)));
 connect(d->mY, SIGNAL(signalValueChanged(float)), this, SLOT(slotValueChanged(float)));
 connect(d->mZ, SIGNAL(signalValueChanged(float)), this, SLOT(slotValueChanged(float)));
 d->mMainLayout->addWidget(d->mX);
 d->mMainLayout->addWidget(d->mY);
 d->mMainLayout->addWidget(d->mZ);
}

BoVector3Input::~BoVector3Input()
{
 delete d;
}

void BoVector3Input::setLabel(const QString& label, int a)
{
 delete d->mLabel;
 d->mLabel = 0;
 if (label.isEmpty()) {
	return;
 }
 d->mLabel = new QLabel(label, this);
 d->mLabel->setAlignment((a & (~(Qt::AlignTop|Qt::AlignBottom|Qt::AlignVCenter))) | Qt::AlignVCenter);

 // if no vertical alignment set, use Top alignment
 if (!(a & (Qt::AlignTop|Qt::AlignBottom|Qt::AlignVCenter))) {
	a |= Qt::AlignTop;
 }
 if (a & Qt::AlignTop) {
	d->mTopLayout->insertWidget(0, d->mLabel);
 } else if (a & Qt::AlignBottom) {
	d->mTopLayout->addWidget(d->mLabel);
 } else {
	d->mMainLayout->insertWidget(0, d->mLabel);
 }
}

QString BoVector3Input::label() const
{
 if (d->mLabel) {
	return d->mLabel->text();
 }
 return QString();
}

// TODO: add a slider?
void BoVector3Input::setRange(float min, float max, float step)
{
 d->mX->setRange(min, max, step, false);
 d->mY->setRange(min, max, step, false);
 d->mZ->setRange(min, max, step, false);
}

float BoVector3Input::minValue() const
{
 return d->mX->minValue();
}

float BoVector3Input::maxValue() const
{
 return d->mX->maxValue();
}

void BoVector3Input::setValue3(const BoVector3Float& v)
{
 d->mX->setValue(v.x());
 d->mY->setValue(v.y());
 d->mZ->setValue(v.z());
}

BoVector3Float BoVector3Input::value3() const
{
 return BoVector3Float(d->mX->value(), d->mY->value(), d->mZ->value());
}

void BoVector3Input::slotValueChanged(float)
{
 emit signalValueChanged(value3());
}


Q3HBoxLayout* BoVector3Input::mainLayout() const
{
 return d->mMainLayout;
}



class BoVector4InputPrivate
{
public:
	BoVector4InputPrivate()
	{
		mW = 0;
	}
	BoFloatNumInput* mW;
};

BoVector4Input::BoVector4Input(QWidget* parent, const char* name) : BoVector3Input(parent, name)
{
 d = new BoVector4InputPrivate;
 d->mW = new BoFloatNumInput(this);
 connect(d->mW, SIGNAL(signalValueChanged(float)), this, SLOT(slotValueChanged(float)));
 mainLayout()->addWidget(d->mW);
}

BoVector4Input::~BoVector4Input()
{
 delete d;
}

void BoVector4Input::setValue4(const BoVector4Float& v)
{
 BoVector3Input::setValue3(BoVector3Float(v.x(), v.y(), v.z()));
 d->mW->setValue(v.w());
}

BoVector4Float BoVector4Input::value4() const
{
 BoVector3Float v = value3();
 return BoVector4Float(v.x(), v.y(), v.z(), d->mW->value());
}

void BoVector4Input::setRange(float min, float max, float step)
{
 BoVector3Input::setRange(min, max, step);
 d->mW->setRange(min, max, step, false);
}

void BoVector4Input::slotValueChanged(float f)
{
 BoVector3Input::slotValueChanged(f);
 emit signalValueChanged(value4());
}

