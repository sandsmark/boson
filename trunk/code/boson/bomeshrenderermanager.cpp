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
#include "bomeshrenderermanager.h"

#include "bomeshrenderer.h"
#include "bosonmodel.h"
#include "bosonconfig.h"
#include "boversion.h"

#include <bodebug.h>

#include <kapplication.h>
#include <kstaticdeleter.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klibloader.h>

#include <qstringlist.h>
#include <qptrlist.h>
#include <qlibrary.h>

#include <stdlib.h>
#include <dlfcn.h>

BOPLUGIN_MANAGER(BoMeshRendererManager, libbomeshrendererplugin)

BoMeshRendererManager* BoMeshRendererManager::mManager = 0;
static KStaticDeleter<BoMeshRendererManager> sd;

class BoMeshRendererManagerPrivate
{
public:
	BoMeshRendererManagerPrivate()
	{
	}

	QPtrList<BosonModel> mAllModels;
};

BoMeshRendererManager::BoMeshRendererManager() : BoPluginManager()
{
 d = new BoMeshRendererManagerPrivate;
}

BoMeshRendererManager::~BoMeshRendererManager()
{
 boDebug() << k_funcinfo << endl;
 unloadLibrary();
 if (d->mAllModels.count() != 0) {
	boWarning() << k_funcinfo << "model list not empty" << endl;
 }
 delete d;
}

void BoMeshRendererManager::addModel(BosonModel* model)
{
 BO_CHECK_NULL_RET(model);
 if (d->mAllModels.containsRef(model)) {
	boWarning() << k_funcinfo << "model already here" << endl;
	return;
 }
 d->mAllModels.append(model);
 if (currentRenderer()) {
	currentRenderer()->initializeData(model);
 }
}

void BoMeshRendererManager::removeModel(BosonModel* model)
{
 BO_CHECK_NULL_RET(model);
 d->mAllModels.removeRef(model);
}

void BoMeshRendererManager::initStatic()
{
 if (mManager) {
	return;
 }
 mManager = new BoMeshRendererManager;
 sd.setObject(mManager);
}

QStringList BoMeshRendererManager::availableRenderers()
{
 return availablePlugins();
}

BoMeshRenderer* BoMeshRendererManager::createRenderer(const QString& name)
{
 return (BoMeshRenderer*)createPlugin(name);
}

QString BoMeshRendererManager::configKey() const
{
 return QString::fromLatin1("MeshRenderer");
}

void BoMeshRendererManager::initializePlugin()
{
 BO_CHECK_NULL_RET(currentRenderer());
 currentRenderer()->setModel(0);

 // the model data must be updated for the new renderer
 QPtrListIterator<BosonModel> it(d->mAllModels);
 while (it.current()) {
	currentRenderer()->initializeData(it.current());
	++it;
 }
}

void BoMeshRendererManager::deinitializePlugin()
{
 BO_CHECK_NULL_RET(currentRenderer());
 boDebug() << k_funcinfo << endl;
 QPtrListIterator<BosonModel> it(d->mAllModels);
 while (it.current()) {
	currentRenderer()->deinitializeData(it.current());
	++it;
 }
}

QString BoMeshRendererManager::currentStatisticsData() const
{
 BoMeshRenderer* current = currentRenderer();
 if (!current) {
	return QString::null;
 }
 return current->statisticsData();
}

