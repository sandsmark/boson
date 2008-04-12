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
#ifndef BOUFOACTIONEXT_H
#define BOUFOACTIONEXT_H

#include "boufo/boufoaction.h"

// this file contains very boson-game specific extensions to boufoaction.*
// -> in boufo/ we should in general not include any ../ files, but here we can
//    without problems.


/**
 * @short Specialized @ref BoUfoToggleAction to configure a @ref BosonConfig
 * option
 *
 * This class modifies a boolean value of @ref BosonConfig. It uses @ref
 * BosonConfig::boolValue and @ref BosonConfig::setBoolValue to read and write the
 * values.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoConfigToggleAction : public BoUfoToggleAction
{
	Q_OBJECT
public:
	/**
	 * @param key The @ref BosonConfig key that is used for @ref
	 * BosonConfig::setBoolValue and @ref BosonConfig::boolValue. You can
	 * use @ref QString() here, then @p name is used for the key. You
	 * have to make sure that such a key actually exists and that it is a
	 * boolean config option!
	 **/
	BoUfoConfigToggleAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const char* name, QString key = QString());
	~BoUfoConfigToggleAction();

	virtual void setChecked(bool c);

private slots:
	void slotValueToggled(bool);

private:
	QString mKey;
};


#endif

