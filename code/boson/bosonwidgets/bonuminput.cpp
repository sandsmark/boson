/*
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include <qspinbox.h>
#include <qlayout.h>
#include <qslider.h>
#include <qlabel.h>

#include <kdialog.h>

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

		mLabel = 0;
	}
	QVBoxLayout* mTopLayout;
	QHBoxLayout* mMainLayout;

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

		mSliderLayout = 0;
	}
	QSpinBox* mSpin;
	QSlider* mSlider;

	QHBoxLayout* mSliderLayout;
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

 d->mSliderLayout = new QHBoxLayout(mainLayout());

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
		connect(d->mSlider, SIGNAL(valueChanged(int)), d->mSpin, SLOT(setValue(int)));
		d->mSliderLayout->addWidget(d->mSlider);
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

void BoIntNumInput::setValue(int v)
{
 d->mSpin->setValue(v);
 // slider gets changed by slotSpinValueChanged()
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

