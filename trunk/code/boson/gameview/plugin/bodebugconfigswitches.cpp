/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bodebugconfigswitches.h"
#include "bodebugconfigswitches.moc"

#include "bodebug.h"
#include "boufocheckbox.h"
#include "../../bosonconfig.h"

#include <qsignalmapper.h>

class BoDebugConfigSwitchesPrivate
{
public:
	BoDebugConfigSwitchesPrivate()
	{
		mBooleanSignalMapper = 0;
	}
	QSignalMapper* mBooleanSignalMapper;
	QMap<QString, BoUfoCheckBox*> mBooleanSwitches;
};

BoDebugConfigSwitches::BoDebugConfigSwitches()
	: BoUfoWidget()
{
 d = new BoDebugConfigSwitchesPrivate();
 d->mBooleanSignalMapper = new QSignalMapper(this);
 connect(d->mBooleanSignalMapper, SIGNAL(mapped(const QString&)),
		this, SLOT(slotChangeBooleanSwitch(const QString&)));

 setLayoutClass(BoUfoWidget::UVBoxLayout);
}

BoDebugConfigSwitches::~BoDebugConfigSwitches()
{
 delete d;
}

void BoDebugConfigSwitches::setTemplate(SwitchTemplate t)
{
 switch (t) {
	case Rendering:
		addBooleanConfigureSwitch("UseLight");
		addBooleanConfigureSwitch("UseGroundShaders");
		addBooleanConfigureSwitch("UseUnitShaders");
		addBooleanConfigureSwitch("UseLOD");
		addBooleanConfigureSwitch("UseVBO");
//		addBooleanConfigureSwitch("WaterShaders");
//		addBooleanConfigureSwitch("TextureCompression");
		addBooleanConfigureSwitch("TextureFOW");
		addBooleanConfigureSwitch("debug_render_ground");
		addBooleanConfigureSwitch("debug_render_items");
		addBooleanConfigureSwitch("debug_render_water");
		addBooleanConfigureSwitch("debug_render_particles");
		break;
	default:
		boWarning() << k_funcinfo << "unknown template " << (int)t << endl;
		break;
 }
}

void BoDebugConfigSwitches::addBooleanConfigureSwitch(const QString& key, const QString& _name)
{
 if (key.isEmpty()) {
	boError() << k_funcinfo << "empty key" << endl;
	return;
 }
 QString name = _name;
 if (name.isEmpty()) {
	name = key;
 }
 if (haveKey(key)) {
	boDebug() << k_funcinfo << "key " << key << " already there" << endl;
	return;
 }
 if (!boConfig->hasKey(key)) {
	boError() << k_funcinfo << "boconfig has no such key: " << key << endl;
	return;
 }
 BoConfigEntry* entry = boConfig->value(key);
 if (!entry) {
	BO_NULL_ERROR(entry);
	return;
 }
 if (entry->type() != BoConfigEntry::Bool) {
	boError() << k_funcinfo << "config entry " << key << " is not of type Bool" << endl;
	return;
 }

 BoUfoCheckBox* check = new BoUfoCheckBox();
 check->setText(name);
 check->setChecked(boConfig->boolValue(key));
 addWidget(check);
 d->mBooleanSignalMapper->setMapping(check, key);
 connect(check, SIGNAL(signalActivated()), d->mBooleanSignalMapper, SLOT(map()));
 d->mBooleanSwitches.insert(key, check);
}

bool BoDebugConfigSwitches::haveKey(const QString& key) const
{
 if (d->mBooleanSwitches.contains(key)) {
	return true;
 }
 return false;
}

void BoDebugConfigSwitches::removeBooleanConfigureSwitch(const QString& key)
{
 if (!d->mBooleanSwitches.contains(key)) {
	return;
 }
 BoUfoCheckBox* check = d->mBooleanSwitches[key];
 d->mBooleanSignalMapper->removeMappings(check);
 d->mBooleanSwitches.remove(key);
 removeWidget(check);
}

void BoDebugConfigSwitches::clear()
{
 QValueList<QString> keys = d->mBooleanSwitches.keys();
 for (QValueList<QString>::iterator it = keys.begin(); it != keys.end(); ++it) {
	removeBooleanConfigureSwitch(*it);
 }
}

void BoDebugConfigSwitches::slotUpdate()
{
 for (QMap<QString, BoUfoCheckBox*>::iterator it = d->mBooleanSwitches.begin(); it != d->mBooleanSwitches.end(); ++it) {
	BoUfoCheckBox* check = it.data();
	check->setChecked(boConfig->boolValue(it.key()));
 }
}

void BoDebugConfigSwitches::slotChangeBooleanSwitch(const QString& key)
{
 if (!d->mBooleanSwitches.contains(key)) {
	return;
 }
 bool v = d->mBooleanSwitches[key]->checked();
 boConfig->setBoolValue(key, v);
}

