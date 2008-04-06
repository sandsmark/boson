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

// note the copyright above: this is LGPL!
#ifndef BOUFONUMINPUT_H
#define BOUFONUMINPUT_H

#include "boufowidget.h"

class BoUfoSlider;
class BoUfoLineEdit;
class BoUfoLabel;

class BoUfoNumInput : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString label READ label WRITE setLabel);
	Q_PROPERTY(double minimumValue READ doubleMinimumValue WRITE setDoubleMinimumValue);
	Q_PROPERTY(double maximumValue READ doubleMaximumValue WRITE setDoubleMaximumValue);
	Q_PROPERTY(double stepSize READ doubleStepSize WRITE setDoubleStepSize);
	Q_PROPERTY(double value READ doubleValue WRITE setDoubleValue);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoNumInput();

	BoUfoSlider* slider() const
	{
		return mSlider;
	}
	BoUfoLineEdit* lineEdit() const
	{
		return mLineEdit;
	}

	float value() const;
	void setRange(float min, float max);
	float minimumValue() const;
	float maximumValue() const;
	void setStepSize(float);
	float stepSize() const;

	void setLabel(const QString& label, int a = Qt::AlignLeft | Qt::AlignTop);
	QString label() const;

	virtual void setOpaque(bool o);

	// AB: these are for Qts property system which doesn't like floats:
	double doubleValue() const { return (double)value(); }
	double doubleMinimumValue() const { return (double)minimumValue(); }
	double doubleMaximumValue() const { return (double)maximumValue(); }
	double doubleStepSize() const { return (double)stepSize(); }
	void setDoubleValue(double v) { setValue((float)v); }
	void setDoubleMinimumValue(double v) { slotSetMinValue((float)v); }
	void setDoubleMaximumValue(double v) { slotSetMaxValue((float)v); }
	void setDoubleStepSize(double v) { setStepSize((float)v); }

public slots:
	void setValue(float);
	void slotSetMaxValue(float);
	void slotSetMinValue(float);

signals:
	void signalValueChanged(float);

private slots:
	void slotSliderChanged(float);
	void slotTextEntered(const QString&);

private:
	void init();

private:
	BoUfoLabel* mLabel;
	BoUfoSlider* mSlider;
	BoUfoLineEdit* mLineEdit;
};
#endif
