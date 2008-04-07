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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONGAMEVIEWPLUGINFACTORY_H
#define BOSONGAMEVIEWPLUGINFACTORY_H

#include <klibloader.h>
#include "../../bopluginmanager.h"

/**
 * This allows creation of a gameview plugin. The gameview plugin is intended to
 * provide a single plugin class only (unlike mesh and ground renderers).
 **/
class BosonGameViewPluginFactory : public KPluginFactory
{
	Q_OBJECT
public:
	BosonGameViewPluginFactory(QObject* parent = 0);
	~BosonGameViewPluginFactory();

protected:
	virtual QObject* createObject(QObject* parent, const char* className,
			const QStringList &args);

};

class BoPluginInformation_libbosongameviewplugin : public BoPluginInformation
{
	Q_OBJECT
public:
	BoPluginInformation_libbosongameviewplugin() : BoPluginInformation()
	{
	}
	~BoPluginInformation_libbosongameviewplugin()
	{
	}

	virtual QStringList plugins() const;

};

#endif
