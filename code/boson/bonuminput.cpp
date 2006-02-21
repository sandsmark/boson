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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!

#include "bonuminput.h"
#include "bonuminput.moc"

#include "../../bomemory/bodummymemory.h"

#include <qspinbox.h>
#include <qlayout.h>
#include <qslider.h>
#include <qlabel.h>

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
	QVBoxLayout* mTopLayout;
	QHBoxLayout* mMainLayout;
	QHBoxLayout* mSliderLayout;

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

 d->mTopLayout = new QVBoxLayout(this, 0, KDialog::spacingHint(), "bonuminput_toplayout");
 d->mMainLayout = new QHBoxLayout(d->mTopLayout);
 d->mSliderLayout = new QHBoxLayout(d->mMainLayout);
}

void BoNumInput::setLabel(const QString& text, int a)
{
 delete d->mLabel;
 d->mLabel = 0;
 if (text.isEmpty()) {
	return;
 }
 d->mLabel = new QLabel(text, this);
 d->mLabel->setAlignment((a & (~(AlignTop|AlignBottom|AlignVCenter))) | AlignVCenter);

 // if no vertical alignment set, use Top alignment
 if(!(a & (AlignTop|AlignBottom|AlignVCenter))) {
	a |= AlignTop;
 }

 if (a & AlignTop) {
	topLayout()->insertWidget(0, d->mLabel);
 } else if (a & AlignBottom) {
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
 return QString::null;
}

QHBoxLayout* BoNumInput::mainLayout() const
{
 return d->mMainLayout;
}

QHBoxLayout* BoNumInput::sliderLayout() const
{
 return d->mSliderLayout;
}

QVBoxLayout* BoNumInput::topLayout() const
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
 min = QMIN(min, max);
 max = QMAX(min, max);
 d->mSpin->setMinValue(min);
 d->mSpin->setMaxValue(max);
 d->mSpin->setLineStep(step);

 step = d->mSpin->lineStep(); // in case it wasn't fully valid

 if (slider) {
	if (!d->mSlider) {
		d->mSlider = new QSlider(QSlider::Horizontal, this, "bointnuminput_slider");
		d->mSlider->setTickmarks(QSlider::Below);
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
	KDoubleSpinBox* mSpin;
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
 d->mSpin = new KDoubleSpinBox(0.0, 9999.0, 0.1, 0.0, 2, this, "bofloatnuminput_spinbox");
 QSpinBox* spin = d->mSpin;
 connect(spin, SIGNAL(valueChanged(int)), this, SLOT(slotSpinValueChanged(int)));
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
 min = QMIN(min, max);
 max = QMAX(min, max);
 d->mSpin->setRange(min, max, step, d->mSpin->precision());

 step = d->mSpin->lineStep(); // in case it wasn't fully valid

 if (slider) {
	// upcast to base type to get min/maxValue in int form
	QSpinBox* spin = d->mSpin;
	int smin = spin->minValue();
	int smax = spin->maxValue();
	int svalue = spin->value();
	int sstep = spin->lineStep();
	if (!d->mSlider) {
		d->mSlider = new QSlider(QSlider::Horizontal, this, "bofloatnuminput_slider");
		d->mSlider->setTickmarks(QSlider::Below);
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
 return (float)d->mSpin->minValue();
}

float BoFloatNumInput::maxValue() const
{
 return (float)d->mSpin->maxValue();
}

bool BoFloatNumInput::showSlider() const
{
 return d->mSlider;
}

double BoFloatNumInput::mapSliderToSpin(int val) const
{
 // map [slidemin,slidemax] to [spinmin,spinmax]
 double spinmin = d->mSpin->minValue();
 double spinmax = d->mSpin->maxValue();
 double slidemin = d->mSlider->minValue(); // cast int to double to avoid
 double slidemax = d->mSlider->maxValue(); // overflow in rel denominator
 double rel = ( double(val) - slidemin ) / ( slidemax - slidemin );
 return spinmin + rel * ( spinmax - spinmin );
}

