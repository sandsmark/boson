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

#include "bonuminput.h"
#include "bonuminput.moc"

#include "../../bomemory/bodummymemory.h"

#include <qspinbox.h>
#include <qlayout.h>
#include <qslider.h>
#include <qlabel.h>
#include <QDoubleSpinBox>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <kdialog.h>
#include <knuminput.h>

#include <config.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif


class BoNumInputPrivate
{
public:
	BoNumInputPrivate()
	{
		mTopLayout = 0;
		mMainLayout = 0;
		mSliderLayout = 0;

		mLabel = 0;
	}
	Q3VBoxLayout* mTopLayout;
	Q3HBoxLayout* mMainLayout;
	Q3HBoxLayout* mSliderLayout;

	QLabel* mLabel;
};

BoNumInput::BoNumInput(QWidget* parent, const char* name) : QWidget(parent, name)
{
 init();
}

BoNumInput::~BoNumInput()
{
 delete d;
}

void BoNumInput::init()
{
 d = new BoNumInputPrivate;

 d->mTopLayout = new Q3VBoxLayout(this, 0, KDialog::spacingHint(), "bonuminput_toplayout");
 d->mMainLayout = new Q3HBoxLayout(d->mTopLayout);
 d->mSliderLayout = new Q3HBoxLayout(d->mMainLayout);
}

void BoNumInput::setLabel(const QString& text, int a)
{
 delete d->mLabel;
 d->mLabel = 0;
 if (text.isEmpty()) {
	return;
 }
 d->mLabel = new QLabel(text, this);
 d->mLabel->setAlignment((a & (~(Qt::AlignTop|Qt::AlignBottom|Qt::AlignVCenter))) | Qt::AlignVCenter);

 // if no vertical alignment set, use Top alignment
 if(!(a & (Qt::AlignTop|Qt::AlignBottom|Qt::AlignVCenter))) {
	a |= Qt::AlignTop;
 }

 if (a & Qt::AlignTop) {
	topLayout()->insertWidget(0, d->mLabel);
 } else if (a & Qt::AlignBottom) {
	topLayout()->addWidget(d->mLabel);
 } else {
	mainLayout()->insertWidget(0, d->mLabel);
 }
}

QString BoNumInput::label() const
{
 if (d->mLabel) {
	return d->mLabel->text();
 }
 return QString();
}

Q3HBoxLayout* BoNumInput::mainLayout() const
{
 return d->mMainLayout;
}

Q3HBoxLayout* BoNumInput::sliderLayout() const
{
 return d->mSliderLayout;
}

Q3VBoxLayout* BoNumInput::topLayout() const
{
 return d->mTopLayout;
}


class BoIntNumInputPrivate
{
public:
	BoIntNumInputPrivate()
	{
		mSpin = 0;
		mSlider = 0;
	}
	QSpinBox* mSpin;
	QSlider* mSlider;
};

BoIntNumInput::BoIntNumInput(QWidget* parent, const char* name) : BoNumInput(parent, name)
{
 init();
}

BoIntNumInput::~BoIntNumInput()
{
 delete d;
}

void BoIntNumInput::init()
{
 d = new BoIntNumInputPrivate;
 d->mSpin = new QSpinBox(INT_MIN, INT_MAX, 1, this, "bointnuminput_spinbox");
 connect(d->mSpin, SIGNAL(valueChanged(int)), this, SLOT(slotSpinValueChanged(int)));
 mainLayout()->addWidget(d->mSpin);

 setFocusProxy(d->mSpin);
}

void BoIntNumInput::slotSpinValueChanged(int v)
{
 if (d->mSlider) {
	d->mSlider->setValue(v);
 }
 emit signalValueChanged(value());
}

void BoIntNumInput::setRange(int min, int max, int step, bool slider)
{
 min = qMin(min, max);
 max = qMax(min, max);
 d->mSpin->setMinValue(min);
 d->mSpin->setMaxValue(max);
 d->mSpin->setLineStep(step);

 step = d->mSpin->singleStep(); // in case it wasn't fully valid

 if (slider) {
	if (!d->mSlider) {
		d->mSlider = new QSlider(Qt::Horizontal, this, "bointnuminput_slider");
		d->mSlider->setTickmarks(QSlider::TicksBelow);
		connect(d->mSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderMoved(int)));
		sliderLayout()->addWidget(d->mSlider);
	}
	d->mSlider->setRange(min, max);

	int major = (max-min) / 10; // see KIntNumInput for a better version! this might overflow ints!
	if (major == 0) {
		major = step;
	}
	d->mSlider->setSteps(step, major);
	d->mSlider->setTickInterval(major);
 } else {
	delete d->mSlider;
	d->mSlider = 0;
 }
}

void BoIntNumInput::setValue(int v, bool emitSignal)
{
 if (!emitSignal) {
	blockSignals(true);
 }
 d->mSpin->setValue(v);
 // slider gets changed by slotSpinValueChanged()

 if (!emitSignal) {
	blockSignals(false);
 }
}

int BoIntNumInput::value() const
{
 return d->mSpin->value();
}

int BoIntNumInput::minValue() const
{
 return d->mSpin->minValue();
}

int BoIntNumInput::maxValue() const
{
 return d->mSpin->maxValue();
}

bool BoIntNumInput::showSlider() const
{
 return d->mSlider;
}

void BoIntNumInput::slotSliderMoved(int v)
{
 d->mSpin->setValue(v);
}



class BoFloatNumInputPrivate
{
public:
	BoFloatNumInputPrivate()
	{
		mSpin = 0;
		mSlider = 0;
	}
	QDoubleSpinBox* mSpin;
	QSlider* mSlider;
};

BoFloatNumInput::BoFloatNumInput(QWidget* parent, const char* name) : BoNumInput(parent, name)
{
 init();
}

BoFloatNumInput::~BoFloatNumInput()
{
 delete d;
}

void BoFloatNumInput::init()
{
 d = new BoFloatNumInputPrivate;
 d->mSpin = new QDoubleSpinBox(this);
 d->mSpin->setRange(0.0, 9999.0);
 d->mSpin->setSingleStep(0.1);
 d->mSpin->setValue(0.0);
 d->mSpin->setDecimals(2);
 connect(d->mSpin, SIGNAL(valueChanged(int)), this, SLOT(slotSpinValueChanged(int)));
 mainLayout()->addWidget(d->mSpin);

 setFocusProxy(d->mSpin);
}

void BoFloatNumInput::slotSpinValueChanged(int v)
{
 if (d->mSlider) {
	d->mSlider->setValue(v);
 }
 emit signalValueChanged(value());
}

void BoFloatNumInput::setRange(float min, float max, float step, bool slider)
{
 if (d->mSlider) {
	d->mSlider->blockSignals(true);
 }
 min = qMin(min, max);
 max = qMax(min, max);
 d->mSpin->setRange(min, max);
 d->mSpin->setSingleStep(step);

 step = d->mSpin->singleStep(); // in case it wasn't fully valid

 if (slider) {
	int smin = d->mSpin->minimum();
	int smax = d->mSpin->maximum();
	int svalue = d->mSpin->value();
	int sstep = d->mSpin->singleStep();
	if (!d->mSlider) {
		d->mSlider = new QSlider(Qt::Horizontal, this, "bofloatnuminput_slider");
		d->mSlider->setTickmarks(QSlider::TicksBelow);
		connect(d->mSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderMoved(int)));
		sliderLayout()->addWidget(d->mSlider);
	}
	d->mSlider->setRange(smin, smax);
	d->mSlider->setLineStep(sstep);
	d->mSlider->setValue(svalue);

	int major = (smax-smin) / 10; // see KIntNumInput for a better version! this might overflow ints!
	if (major == 0) {
		major = sstep;
	}
	d->mSlider->setTickInterval(major);
 } else {
	delete d->mSlider;
	d->mSlider = 0;
 }
 if (d->mSlider) {
	d->mSlider->blockSignals(false);
 }
}

void BoFloatNumInput::slotSliderMoved(int v)
{
 d->mSpin->setValue(mapSliderToSpin(v));
}

void BoFloatNumInput::setValue(float v, bool emitSignal)
{
 if (!emitSignal) {
	blockSignals(true);
 }
 d->mSpin->setValue(v);
 // slider gets changed by slotSpinValueChanged()

 if (!emitSignal) {
	blockSignals(false);
 }
}

float BoFloatNumInput::value() const
{
 return (float)d->mSpin->value();
}

float BoFloatNumInput::minValue() const
{
 return (float)d->mSpin->minimum();
}

float BoFloatNumInput::maxValue() const
{
 return (float)d->mSpin->maximum();
}

bool BoFloatNumInput::showSlider() const
{
 return d->mSlider;
}

double BoFloatNumInput::mapSliderToSpin(int val) const
{
 // map [slidemin,slidemax] to [spinmin,spinmax]
 double spinmin = d->mSpin->minimum();
 double spinmax = d->mSpin->maximum();
 double slidemin = d->mSlider->minValue(); // cast int to double to avoid
 double slidemax = d->mSlider->maxValue(); // overflow in rel denominator
 double rel = ( double(val) - slidemin ) / ( slidemax - slidemin );
 return spinmin + rel * ( spinmax - spinmin );
}

