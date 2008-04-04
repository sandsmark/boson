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

#ifndef BOVECTORINPUT_H
#define BOVECTORINPUT_H

#include <qwidget.h>

class QHBoxLayout;
class QVBoxLayout;
class bofixed;
template<class T> class BoVector3;
template<class T> class BoVector4;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector3<float> BoVector3Float;
typedef BoVector4<bofixed> BoVector4Fixed;
typedef BoVector4<float> BoVector4Float;

class BoVector3InputPrivate;
class BoVector3Input : public QWidget
{
	Q_OBJECT
public:
	BoVector3Input(QWidget* parent = 0, const char* name = 0);
	~BoVector3Input();

	void setLabel(const QString& label, int a = AlignLeft | AlignTop);
	QString label() const;

	virtual void setRange(float min, float max, float step = 0.1f);
	float minValue() const;
	float maxValue() const;


	void setValue3(const BoVector3Float&);
	BoVector3Float value3() const;

signals:
	void signalValueChanged(const BoVector3Float&);

protected slots:
	virtual void slotValueChanged(float);

protected:
	QHBoxLayout* mainLayout() const;

private:
	BoVector3InputPrivate* d;
};

class BoVector4InputPrivate;
class BoVector4Input : public BoVector3Input
{
	Q_OBJECT
public:
	BoVector4Input(QWidget* parent = 0, const char* name = 0);
	~BoVector4Input();

	virtual void setRange(float min, float max, float step = 0.1f);

	void setValue4(const BoVector4Float&);
	BoVector4Float value4() const;

signals:
	void signalValueChanged(const BoVector4Float&);

protected slots:
	virtual void slotValueChanged(float);

private:
	BoVector4InputPrivate* d;
};

#endif
