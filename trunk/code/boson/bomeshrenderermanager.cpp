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

BoMeshRendererManager* BoMeshRendererManager::mManager = 0;
static KStaticDeleter<BoMeshRendererManager> sd;

class BoMeshRendererManagerPrivate
{
public:
	BoMeshRendererManagerPrivate()
	{
		mLibrary = 0;
		mLibraryFactory = 0;
	}

	QPtrList<BosonModel> mAllModels;

	QLibrary* mLibrary;
	KLibFactory* mLibraryFactory;
};

BoMeshRendererManager::BoMeshRendererManager()
{
 d = new BoMeshRendererManagerPrivate;
 mCurrentRenderer = 0;
}

BoMeshRendererManager::~BoMeshRendererManager()
{
 unloadLibrary();
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

bool BoMeshRendererManager::makeRendererCurrent(const QString& name)
{
 QStringList list = availableRenderers();
 if (list.isEmpty()) {
	boError() << k_funcinfo << "no meshrenderer available" << endl;
	return false;
 }
 if (name.isEmpty()) {
	if (list.first().isEmpty()) {
		boError() << k_funcinfo << "first meshrenderer found is invalid!" << endl;
		return false;
	}
	QString renderer = boConfig->stringValue("MeshRenderer");
	if (!list.contains(renderer)) {
		boWarning() << k_funcinfo << "boConfig requested renderer " << renderer << " but it was not available" << endl;
		renderer = list.first();
	}
	return makeRendererCurrent(renderer);
 }

 if (!availableRenderers().contains(name)) {
	boError() << k_funcinfo << "renderer " << name << " not available" << endl;
	return false;
 }

 BoMeshRenderer* renderer = createRenderer(name);
 if (renderer) {
	boDebug() << k_funcinfo << "created renderer" << endl;
	return makeRendererCurrent(renderer);
 } else {
	boError() << k_funcinfo << "Error loading renderer " << name << endl;
 }
 return false;
}



QStringList BoMeshRendererManager::availableRenderers()
{
 QStringList list;
 loadLibrary();
 if (!d->mLibraryFactory) {
	return list;
 }

 BoMeshRendererInformation* info = (BoMeshRendererInformation*)d->mLibraryFactory->create(0, 0, "BoMeshRendererInformation");
 if (!info) {
	// should never happen, as we check for it at loading
	boError() << k_funcinfo << "no information object?!" << endl;
	return list;
 }
 list = info->meshRenderers();
 delete info;

 return list;
}

QString BoMeshRendererManager::currentRendererName() const
{
 if (!currentRenderer()) {
	return QString::null;
 }
 return currentRenderer()->className();
}

BoMeshRenderer* BoMeshRendererManager::createRenderer(const QString& name)
{
 boDebug() << k_funcinfo << name << endl;
 if (!loadLibrary()) {
	return 0;
 }
 BO_CHECK_NULL_RET0(d->mLibraryFactory);
 return (BoMeshRenderer*)d->mLibraryFactory->create(0, 0, name);
}

bool BoMeshRendererManager::loadLibrary()
{
 if (d->mLibrary) {
	if (d->mLibrary->isLoaded()) {
		return true;
	}
	return false;
 }
 QString lib = "libbomeshrendererplugin";
 QString file  = KGlobal::dirs()->findResource("lib", QString("kde3/plugins/boson/%1.so").arg(lib));

 QString error;
 bool ret = true;
 typedef KLibFactory* (*init_function)();
 typedef int (*version_function)();
 init_function init_func = 0;
 version_function version_func = 0;

 if (file.isEmpty()) {
	error = i18n("Unable to find a file for this plugin");
	boError() << k_funcinfo << error << endl;
	ret = false;
 } else {
	d->mLibrary = new QLibrary(file);
 }
 if (ret) {
	ret = d->mLibrary->load();
	if (!ret) {
		error = i18n("Library loading failed");
		char* e = dlerror();
		if (e) {
			error = QString("%1 - reported error: %2").arg(error).arg(e);
		}
	}
 }

 if (ret) {
	boDebug() << k_funcinfo << "library " << lib << " loaded. resolving symbols" << endl;

	if (ret) {
		QCString init_name = QCString("init_") + lib.latin1();
		init_func = (init_function)d->mLibrary->resolve(init_name);
		if (!init_func) {
			ret = false;
			error = i18n("Could not resolve %1").arg(init_name);
			boError() << k_funcinfo << error << endl;
		}
	}
	if (ret) {
		typedef void (*FunctionType)();
		QCString version_name = QCString("version_") + lib.latin1();
		version_func = (version_function)d->mLibrary->resolve(version_name);
		if (!version_func) {
			ret = false;
			error = i18n("Could not resolve %1").arg(version_name);
			boError() << k_funcinfo << error << endl;
		}
	}
 }

 if (ret) {
	boDebug() << k_funcinfo << "symbols resolved. checking version" << endl;
	int version = version_func();
	if (version != BOSON_VERSION) {
		error = i18n("Version mismatch: plugin compiled for %1, you are running %2").arg(version).arg(BOSON_VERSION);
		boError() << k_funcinfo << error << endl;
		ret = false;
	}
 }
 if (ret) {
	boDebug() << k_funcinfo << "initializing plugin" << endl;
	d->mLibraryFactory = init_func();
	if (!d->mLibraryFactory) {
		error = i18n("Could not load factory (init returned NULL)");
		boError() << k_funcinfo << error << endl;
		ret = false;
	}
 }
 if (ret) {
	boDebug() << k_funcinfo << "searching for information object" << endl;
		QCString info_name = QCString("BoMeshRendererInformation");
		QObject* info = d->mLibraryFactory->create(0, 0, info_name);
		if (!info) {
			error = i18n("Could not find the information object. searched for: %1").arg(info_name);
			boError() << k_funcinfo << error << endl;
			ret = false;
		} else {
			delete info;
		}
 }
 if (ret) {
	boDebug() << k_funcinfo << "library should be ready to use now" << endl;
 } else {
	boError() << k_funcinfo << "library loading failed. fatal error." << endl;
	KMessageBox::sorry(0, i18n("Mesh renderer plugin could not be loaded - check your installation!\nFailed plugin: %1\nTried file: %2\nError: %3").arg(lib).arg(file).arg(error));
	unloadLibrary();
	exit(1);
 }
 return ret;
}

bool BoMeshRendererManager::unloadLibrary()
{
 boDebug() << k_funcinfo << "unsetting old renderer" << endl;
 unsetCurrentRenderer();
 boDebug() << k_funcinfo << "deleting factory" << endl;
 delete d->mLibraryFactory;
 d->mLibraryFactory = 0;
 bool ret = true;
 if (d->mLibrary) {
	if (!d->mLibrary->unload()) {
		boError() << k_funcinfo << "unloading lib failed!" << endl;
		ret = false;
	}
 }
 delete d->mLibrary;
 d->mLibrary = 0;
 return ret;
}

bool BoMeshRendererManager::makeRendererCurrent(BoMeshRenderer* renderer)
{
 if (!renderer) {
	return false;
 }
 if (renderer == mCurrentRenderer) {
	return true;
 }
 if (mCurrentRenderer) {
	boDebug() << k_funcinfo << "unsetting old renderer" << endl;
	unsetCurrentRenderer();
	boDebug() << k_funcinfo << "old renderer unset" << endl;
 }
 renderer->setModel(0);
 mCurrentRenderer = renderer;

 // the model data must be updated for the new renderer
 QPtrListIterator<BosonModel> it(d->mAllModels);
 while (it.current()) {
	renderer->initializeData(it.current());
	++it;
 }
 return true;
}

void BoMeshRendererManager::unsetCurrentRenderer()
{
 if (!mCurrentRenderer) {
	// nothing to do
	return;
 }
 boDebug() << k_funcinfo << endl;
 QPtrListIterator<BosonModel> it(d->mAllModels);
 while (it.current()) {
	mCurrentRenderer->deinitializeData(it.current());
	++it;
 }
 delete mCurrentRenderer;
 mCurrentRenderer = 0;
}

bool BoMeshRendererManager::checkCurrentRenderer()
{
 BoMeshRendererManager* manager = BoMeshRendererManager::manager();
 if (!manager) {
	BO_NULL_ERROR(manager);
	return false;
 }
 if (!manager->currentRenderer()) {
	boDebug() << k_funcinfo << "getting a default meshrenderer" << endl;
		// pick a default renderer
	bool ret = manager->makeRendererCurrent(QString::null);
	if (ret) {
		boDebug() << k_funcinfo << "default meshrenderer loaded" << endl;
	}
	return ret;
 }
 return true;
}

bool BoMeshRendererManager::reloadPlugin(bool* unusable)
{
 if (!unloadLibrary()) {
	boError() << k_funcinfo << "unloading failed" << endl;
	if (unusable) {
		*unusable = true;
	}
	return false;
 }
 if (!loadLibrary()) {
	boError() << k_funcinfo << "library loading failed" << endl;
	if (unusable) {
		*unusable = true;
	}
	return false;
 }
 if (unusable) {
	*unusable = false;
 }
 return checkCurrentRenderer();
}

