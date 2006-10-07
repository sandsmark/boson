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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
	BoMeshRendererFactory(QObject* parent = 0, const char* name = 0);
	~BoMeshRendererFactory();

protected:
	virtual QObject* createObject(QObject* parent = 0, const char* name = 0,
			const char* className = "QObject",
			const QStringList &args = QStringList());

private:
	static KInstance* mInstance;
};

class BoPluginInformation_libbomeshrendererplugin : public BoPluginInformation
{
	Q_OBJECT
public:
	BoPluginInformation_libbomeshrendererplugin() : BoPluginInformation()
	{
	}
	~BoPluginInformation_libbomeshrendererplugin()
	{
	}

	virtual QStringList plugins() const;

};

#endif
