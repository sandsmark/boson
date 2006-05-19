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
#ifndef BODEBUGCONFIGSWITCHES_H
#define BODEBUGCONFIGSWITCHES_H

#include "../../boufo/boufo.h"

class BoDebugConfigSwitchesPrivate;
/**
 * @short Widget that helps at displaying configure switches.
 *
 * The widget is meant to display a certain set of configure switches (such as
 * "render ground", "render items", "enable light", "enable shadows", ...).
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoDebugConfigSwitches : public BoUfoWidget
{
	Q_OBJECT
public:
	/**
	 * Preconfigured templates, that display a certain set of configure
	 * switches
	 **/
	enum SwitchTemplate {
		Rendering = 1
	};

public:
	BoDebugConfigSwitches();
	~BoDebugConfigSwitches();

	void setTemplate(SwitchTemplate t);

	/**
	 * Add a checkbox for the config entry @p key. The @p name is used as
	 * text for the checkbox. If @p name is empty, @p key is used instead.
	 **/
	void addBooleanConfigureSwitch(const QString& key, const QString& name = QString::null);

	bool haveKey(const QString& key) const;

	void removeBooleanConfigureSwitch(const QString& key);
	void clear();

public slots:
	/**
	 * Update the displayed data according to the current @ref BosonConfig
	 * values
	 **/
	void slotUpdate();

protected slots:
	void slotChangeBooleanSwitch(const QString& key);

private:
	BoDebugConfigSwitchesPrivate* d;
};


#endif

