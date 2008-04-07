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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "configoptionwidgets.h"
#include "configoptionwidgets.moc"

#include "../../bomemory/bodummymemory.h"
#include "bosonconfig.h"
#include "bodebug.h"

#include <knuminput.h>

#include <qlayout.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <Q3VBoxLayout>


ConfigOptionWidget::ConfigOptionWidget(const QString& configKey, QWidget* parent)
	: QWidget(parent)
{
 mKey = configKey;
}

ConfigOptionWidget::~ConfigOptionWidget()
{
 boDebug(210) << k_funcinfo << endl;
}



ConfigOptionWidgetInt::ConfigOptionWidgetInt(const QString& configKey, QWidget* parent)
	: ConfigOptionWidget(configKey, parent)
{
 Q3VBoxLayout* layout = new Q3VBoxLayout(this);
 mNumInput = new KIntNumInput(this);
 connect(mNumInput, SIGNAL(valueChanged(int)), this, SIGNAL(signalValueChanged()));
 layout->addWidget(mNumInput);

 load();
}

ConfigOptionWidgetInt::~ConfigOptionWidgetInt()
{
}

void ConfigOptionWidgetInt::setLabel(const QString& label)
{
 mNumInput->setLabel(label);
}

QString ConfigOptionWidgetInt::label() const
{
 return mNumInput->label();
}

void ConfigOptionWidgetInt::setValue(int v)
{
 mNumInput->setValue(v);
}

int ConfigOptionWidgetInt::value() const
{
 return mNumInput->value();
}

void ConfigOptionWidgetInt::setRange(int min, int max, int step)
{
 mNumInput->setRange(min, max, step);
}

int ConfigOptionWidgetInt::minValue() const
{
 return mNumInput->minimum();
}

int ConfigOptionWidgetInt::maxValue() const
{
 return mNumInput->maximum();
}

void ConfigOptionWidgetInt::loadFromConfigScript(const BosonConfigScript* script)
{
 const BoConfigEntry* value = script->valueForKey(configKey());
 if (!value) {
	return;
 }
 if (value->type() != BoConfigEntry::Int) {
	boError() << k_funcinfo << "key " << configKey() << " is there, but has unexpected type " << value->type() << endl;
	return;
 }
 const BoConfigIntEntry* v = (const BoConfigIntEntry*)value;
 setValue(v->value());
}

void ConfigOptionWidgetInt::load()
{
 setValue(boConfig->intValue(configKey()));
}

void ConfigOptionWidgetInt::apply()
{
 boConfig->setIntValue(configKey(), value());
}

void ConfigOptionWidgetInt::loadDefault()
{
 setValue(boConfig->intDefaultValue(configKey()));
}

ConfigOptionWidgetUInt::ConfigOptionWidgetUInt(const QString& configKey, QWidget* parent)
	: ConfigOptionWidget(configKey, parent)
{
 Q3VBoxLayout* layout = new Q3VBoxLayout(this);
 mNumInput = new KIntNumInput(this);
 connect(mNumInput, SIGNAL(valueChanged(int)), this, SIGNAL(signalValueChanged()));
 layout->addWidget(mNumInput);

 load();
}

ConfigOptionWidgetUInt::~ConfigOptionWidgetUInt()
{
}

void ConfigOptionWidgetUInt::setLabel(const QString& label)
{
 mNumInput->setLabel(label);
}

QString ConfigOptionWidgetUInt::label() const
{
 return mNumInput->label();
}

void ConfigOptionWidgetUInt::setValue(unsigned int v)
{
 mNumInput->setValue(v);
}

unsigned int ConfigOptionWidgetUInt::value() const
{
 int v = mNumInput->value();
 if (v < 0) {
	return 0;
 }
 return v;
}

void ConfigOptionWidgetUInt::setRange(unsigned int min, unsigned int max, int step)
{
 mNumInput->setRange(min, max, step);
}

int ConfigOptionWidgetUInt::minValue() const
{
 return mNumInput->minimum();
}

int ConfigOptionWidgetUInt::maxValue() const
{
 return mNumInput->maximum();
}

void ConfigOptionWidgetUInt::loadFromConfigScript(const BosonConfigScript* script)
{
 const BoConfigEntry* value = script->valueForKey(configKey());
 if (!value) {
	return;
 }
 if (value->type() != BoConfigEntry::UInt) {
	boError() << k_funcinfo << "key " << configKey() << " is there, but has unexpected type " << value->type() << endl;
	return;
 }
 const BoConfigUIntEntry* v = (const BoConfigUIntEntry*)value;
 setValue(v->value());
}

void ConfigOptionWidgetUInt::load()
{
 setValue(boConfig->uintValue(configKey()));
}

void ConfigOptionWidgetUInt::apply()
{
 boConfig->setUIntValue(configKey(), value());
}

void ConfigOptionWidgetUInt::loadDefault()
{
 setValue(boConfig->uintDefaultValue(configKey()));
}


ConfigOptionWidgetDouble::ConfigOptionWidgetDouble(const QString& configKey, QWidget* parent)
	: ConfigOptionWidget(configKey, parent)
{
 Q3VBoxLayout* layout = new Q3VBoxLayout(this);
 mNumInput = new KDoubleNumInput(this);
 connect(mNumInput, SIGNAL(valueChanged(double)), this, SIGNAL(signalValueChanged()));
 layout->addWidget(mNumInput);

 load();
}

ConfigOptionWidgetDouble::~ConfigOptionWidgetDouble()
{
}

void ConfigOptionWidgetDouble::setLabel(const QString& label)
{
 mNumInput->setLabel(label);
}

QString ConfigOptionWidgetDouble::label() const
{
 return mNumInput->label();
}

void ConfigOptionWidgetDouble::setValue(double v)
{
 mNumInput->setValue(v);
}

double ConfigOptionWidgetDouble::value() const
{
 return mNumInput->value();
}

void ConfigOptionWidgetDouble::setRange(double min, double max, double step)
{
 mNumInput->setRange(min, max, step);
}

double ConfigOptionWidgetDouble::minValue() const
{
 return mNumInput->minimum();
}

double ConfigOptionWidgetDouble::maxValue() const
{
 return mNumInput->maximum();
}

void ConfigOptionWidgetDouble::loadFromConfigScript(const BosonConfigScript* script)
{
 const BoConfigEntry* value = script->valueForKey(configKey());
 if (!value) {
	return;
 }
 if (value->type() != BoConfigEntry::Double) {
	boError() << k_funcinfo << "key " << configKey() << " is there, but has unexpected type " << value->type() << endl;
	return;
 }
 const BoConfigDoubleEntry* v = (const BoConfigDoubleEntry*)value;
 setValue(v->value());
}

void ConfigOptionWidgetDouble::load()
{
 setValue(boConfig->doubleValue(configKey()));
}

void ConfigOptionWidgetDouble::apply()
{
 boConfig->setDoubleValue(configKey(), value());
}

void ConfigOptionWidgetDouble::loadDefault()
{
 setValue(boConfig->doubleDefaultValue(configKey()));
}



ConfigOptionWidgetBool::ConfigOptionWidgetBool(const QString& configKey, QWidget* parent)
	: ConfigOptionWidget(configKey, parent)
{
 Q3VBoxLayout* layout = new Q3VBoxLayout(this);
 mCheckBox = new QCheckBox(this);
 layout->addWidget(mCheckBox);

 connect(mCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(signalValueChanged()));
 connect(mCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(signalValueChanged(bool)));

 load();
}

ConfigOptionWidgetBool::~ConfigOptionWidgetBool()
{
}

void ConfigOptionWidgetBool::setLabel(const QString& label)
{
 mCheckBox->setText(label);
}

QString ConfigOptionWidgetBool::label() const
{
 return mCheckBox->text();
}

void ConfigOptionWidgetBool::setValue(bool c)
{
 mCheckBox->setChecked(c);
}

bool ConfigOptionWidgetBool::value() const
{
 return mCheckBox->isChecked();
}

void ConfigOptionWidgetBool::loadFromConfigScript(const BosonConfigScript* script)
{
 const BoConfigEntry* value = script->valueForKey(configKey());
 if (!value) {
	return;
 }
 if (value->type() != BoConfigEntry::Bool) {
	boError() << k_funcinfo << "key " << configKey() << " is there, but has unexpected type " << value->type() << endl;
	return;
 }
 const BoConfigBoolEntry* v = (const BoConfigBoolEntry*)value;
 setValue(v->value());
}

void ConfigOptionWidgetBool::load()
{
 setValue(boConfig->boolValue(configKey()));
}

void ConfigOptionWidgetBool::apply()
{
 boConfig->setBoolValue(configKey(), value());
}

void ConfigOptionWidgetBool::loadDefault()
{
 setValue(boConfig->boolDefaultValue(configKey()));
}


