/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef CONFIGOPTIONWIDGETS_H
#define CONFIGOPTIONWIDGETS_H

#include <qwidget.h>
#include <qstring.h>

class QWidget;
class KIntNumInput;
class KDoubleNumInput;
class QCheckBox;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class ConfigOptionWidget : public QWidget
{
	Q_OBJECT
public:
	ConfigOptionWidget(const QString& configKey, QWidget* parent = 0);
	virtual ~ConfigOptionWidget();

	virtual void setLabel(const QString& label) = 0;
	virtual QString label() const = 0;

	virtual void load() = 0;
	virtual void apply() = 0;
	virtual void loadDefault() = 0;

	const QString& configKey() const
	{
		return mKey;
	}

signals:
	void signalValueChanged();

private:
	QString mKey;
};

class ConfigOptionWidgetInt : public ConfigOptionWidget
{
	Q_OBJECT
public:
	ConfigOptionWidgetInt(const QString& configKey, QWidget* parent = 0);
	~ConfigOptionWidgetInt();

	virtual void setLabel(const QString& label);
	virtual QString label() const;

	void setValue(int value);
	int value() const;
	void setRange(int min, int max, int step = 1);
	int minValue() const;
	int maxValue() const;

	virtual void load();
	virtual void apply();
	virtual void loadDefault();

private:
	KIntNumInput* mNumInput;
};

class ConfigOptionWidgetUInt : public ConfigOptionWidget
{
	Q_OBJECT
public:
	ConfigOptionWidgetUInt(const QString& configKey, QWidget* parent = 0);
	~ConfigOptionWidgetUInt();

	virtual void setLabel(const QString& label);
	virtual QString label() const;

	void setValue(unsigned int value);
	unsigned int value() const;
	void setRange(unsigned int min, unsigned int max, int step = 1);
	int minValue() const;
	int maxValue() const;

	virtual void load();
	virtual void apply();
	virtual void loadDefault();

private:
	KIntNumInput* mNumInput;
};

class ConfigOptionWidgetDouble : public ConfigOptionWidget
{
	Q_OBJECT
public:
	ConfigOptionWidgetDouble(const QString& configKey, QWidget* parent = 0);
	~ConfigOptionWidgetDouble();

	virtual void setLabel(const QString& label);
	virtual QString label() const;

	void setValue(double value);
	double value() const;
	void setRange(double min, double max, double step = 1.0);
	double minValue() const;
	double maxValue() const;

	virtual void load();
	virtual void apply();
	virtual void loadDefault();

private:
	KDoubleNumInput* mNumInput;
};

class ConfigOptionWidgetBool : public ConfigOptionWidget
{
	Q_OBJECT
public:
	ConfigOptionWidgetBool(const QString& configKey, QWidget* parent = 0);
	~ConfigOptionWidgetBool();

	virtual void setLabel(const QString& label);
	virtual QString label() const;

	void setValue(bool c);
	void setChecked(bool c)
	{
		setValue(c);
	}
	bool value() const;
	bool isChecked() const
	{
		return value();
	}

	virtual void load();
	virtual void apply();
	virtual void loadDefault();

signals:
	void signalValueChanged(bool);

private:
	QCheckBox* mCheckBox;
};

#endif
