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
#include "bosongameviewpluginmanager.h"

#include "../bomemory/bodummymemory.h"
#include "bosongameviewpluginbase.h"
#include "boversion.h"
#include <bodebug.h>

#include <kapplication.h>
#include <klocale.h>
#include <klibloader.h>

#include <qstringlist.h>

#include <stdlib.h>
#include <dlfcn.h>

BOPLUGIN_MANAGER(BosonGameViewPluginManager, libbosongameviewplugin)

BosonGameViewPluginManager* BosonGameViewPluginManager::mManager = 0;

class BosonGameViewPluginManagerPrivate
{
public:
	BosonGameViewPluginManagerPrivate()
	{
	}
};

BosonGameViewPluginManager::BosonGameViewPluginManager() : BoPluginManager()
{
 d = new BosonGameViewPluginManagerPrivate;
}

BosonGameViewPluginManager::~BosonGameViewPluginManager()
{
 boDebug() << k_funcinfo << endl;
 unloadLibrary();
 delete d;
}

void BosonGameViewPluginManager::initStatic()
{
 if (mManager) {
	return;
 }
 boDebug() << k_funcinfo << endl;
 mManager = new BosonGameViewPluginManager;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonGameViewPluginManager::deleteStatic()
{
 boDebug() << k_funcinfo << endl;
 delete mManager;
 mManager = 0;
 boDebug() << k_funcinfo << "done" << endl;
}

BosonGameViewPluginManager* BosonGameViewPluginManager::manager()
{
 if (!mManager) {
	boError() << k_funcinfo << "requested manager, but initStatic() has not yet been called. We will most likely crash!" << endl;
	QString bt = boBacktrace();
	if (!bt.isEmpty()) {
		boError() << "backtrace: " << bt << endl;
	}
	return 0;
 }
 return mManager;
}

QString BosonGameViewPluginManager::configKey() const
{
 return QString::fromLatin1("GameViewPlugin");
}

void BosonGameViewPluginManager::initializePlugin()
{
}

void BosonGameViewPluginManager::deinitializePlugin()
{
}



