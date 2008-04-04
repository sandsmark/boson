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

#ifndef BONUMINPUT_H
#define BONUMINPUT_H

#include <qwidget.h>

class QHBoxLayout;
class QVBoxLayout;

class BoNumInputPrivate;
class BoNumInput : public QWidget
{
	Q_OBJECT
public:
	BoNumInput(QWidget* parent = 0, const char* name = 0);
	~BoNumInput();

	void setLabel(const QString& label, int a = AlignLeft | AlignTop);
	QString label() const;

	void setSteps(int minor, int major);

	virtual bool showSlider() const = 0;

protected:
	/**
	 * @return The main layout, containing the spinbox and (if existing)
	 * the slider.
	 **/
	QHBoxLayout* mainLayout() const;

	/**
	 * @return The layout containing the @ref QSlider, if existing.
	 * Otherwise the layout is empty.
	 **/
	QHBoxLayout* sliderLayout() const;

	/**
	 * @return The top layout. Inside it you'll find the @ref mainLayout.
	 * The label (see @ref setLabel) can be at top of this layout, at it's
	 * bottom or in @ref mainLayout, depending on the alignment flags.
	 **/
	QVBoxLayout* topLayout() const;

private:
	void init();

private:
	BoNumInputPrivate* d;
};


class BoIntNumInputPrivate;
class BoIntNumInput : public BoNumInput
{
	Q_OBJECT
public:
	BoIntNumInput(QWidget* parent = 0, const char* name = 0);
	~BoIntNumInput();

	void setRange(int min, int max, int step = 1, bool showSlider = true);
	int minValue() const;
	int maxValue() const;

	int value() const;

	virtual bool showSlider() const;

public slots:
	/**
	 * @param emitSignal Use FALSE to avoid @ref signalValueChanged being
	 * emitted when you call this. By default it gets emitted, just as @ref
	 * QSpinBox does.
	 **/
	void setValue(int, bool emitSignal = true);

signals:
	void signalValueChanged(int);

protected slots:
	void slotSpinValueChanged(int);
	void slotSliderMoved(int);

private:
	void init();

private:
	BoIntNumInputPrivate* d;
};

class BoFloatNumInputPrivate;
class BoFloatNumInput : public BoNumInput
{
	Q_OBJECT
public:
	BoFloatNumInput(QWidget* parent = 0, const char* name = 0);
	~BoFloatNumInput();

	void setRange(float min, float max, float step = 0.1f, bool showSlider = true);
	float minValue() const;
	float maxValue() const;

	float value() const;

	virtual bool showSlider() const;

public slots:
	/**
	 * @param emitSignal Use FALSE to avoid @ref signalValueChanged being
	 * emitted when you call this. By default it gets emitted, just as @ref
	 * QSpinBox does.
	 **/
	void setValue(float, bool emitSignal = true);

signals:
	void signalValueChanged(float);

protected slots:
	void slotSpinValueChanged(int);
	void slotSliderMoved(int);

protected:
	double mapSliderToSpin(int) const;

private:
	void init();

private:
	BoFloatNumInputPrivate* d;
};

#endif
