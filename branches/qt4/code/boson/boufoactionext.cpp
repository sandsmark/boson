/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "boufoactionext.h"
#include "boufoactionext.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "bosonconfig.h"

BoUfoConfigToggleAction::BoUfoConfigToggleAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const char* name, QString key)
	: BoUfoToggleAction(text, cut, receiver, slot, parent, name)
{
 if (key.isEmpty()) {
	key = name;
	if (key == "unnamed") {
		boWarning() << k_funcinfo << "using key \"unnamed\" - is this really intended??" << endl;
	}
 }
 mKey = key;

 if (boConfig->hasKey(key)) {
	connect(this, SIGNAL(signalToggled(bool)), this, SLOT(slotValueToggled(bool)));
	setChecked(boConfig->boolValue(key));
 } else {
	boError() << k_funcinfo << "no such key " << key << endl;
 }
}

BoUfoConfigToggleAction::~BoUfoConfigToggleAction()
{
}


void BoUfoConfigToggleAction::setChecked(bool c)
{
 BoUfoToggleAction::setChecked(c);
 slotValueToggled(isChecked());
}

void BoUfoConfigToggleAction::slotValueToggled(bool v)
{
 if (!boConfig->hasKey(mKey)) {
	boError() << k_funcinfo << "boConfig has no such key: " << mKey << endl;
	return;
 }
 boConfig->setBoolValue(mKey, v);
}

