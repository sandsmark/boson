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
#ifndef BOMESHRENDERERFACTORY_H
#define BOMESHRENDERERFACTORY_H

#include <klibloader.h>
#include "../bomeshrenderer.h"
#include "../../bopluginmanager.h"

class BoMeshRendererFactory : public KLibFactory
{
	Q_OBJECT
public:
	BoMeshRendererFactory(QObject* parent = 0);
	~BoMeshRendererFactory();

protected:
	virtual QObject* create(const char* iface, QWidget* parentWidget, QObject* parent, const QVariantList& args, const QString& keyWord);
};

class BoPluginInformation_bomeshrendererplugin : public BoPluginInformation
{
	Q_OBJECT
public:
	BoPluginInformation_bomeshrendererplugin() : BoPluginInformation()
	{
	}
	~BoPluginInformation_bomeshrendererplugin()
	{
	}

	virtual QStringList plugins() const;

};

#endif
