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
#include "bogroundrenderermanager.h"

#include "bogroundrenderer.h"
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
#include <qlibrary.h>

#include <stdlib.h>
#include <dlfcn.h>

BoGroundRendererManager* BoGroundRendererManager::mManager = 0;
static KStaticDeleter<BoGroundRendererManager> sd;

class BoGroundRendererManagerPrivate
{
public:
	BoGroundRendererManagerPrivate()
	{
		mLocalPlayerIO = 0;
		mViewFrustum = 0;
		mModelviewMatrix = 0;
		mProjectionMatrix = 0;
		mViewport = 0;

		mLibrary = 0;
		mLibraryFactory = 0;
	}

	PlayerIO* mLocalPlayerIO;
	const float* mViewFrustum;
	const BoMatrix* mModelviewMatrix;
	const BoMatrix* mProjectionMatrix;
	const int* mViewport;

	QLibrary* mLibrary;
	KLibFactory* mLibraryFactory;
};

BoGroundRendererManager::BoGroundRendererManager()
{
 d = new BoGroundRendererManagerPrivate;
 mCurrentRenderer = 0;
}

BoGroundRendererManager::~BoGroundRendererManager()
{
 unloadLibrary();
 delete d;
}

void BoGroundRendererManager::initStatic()
{
 if (mManager) {
	return;
 }
 mManager = new BoGroundRendererManager;
 sd.setObject(mManager);
}

bool BoGroundRendererManager::makeRendererIdCurrent(int id)
{
 bool ret = false;
 switch (id) {
	case BoGroundRenderer::Fast:
		ret = makeRendererCurrent("BoFastGroundRenderer");
		break;
	case BoGroundRenderer::Default:
		ret = makeRendererCurrent("BoDefaultGroundRenderer");
		break;
	default:
		break;
 }
 if (ret && mCurrentRenderer) {
	QString renderer = mCurrentRenderer->className();
	boConfig->setStringValue("GroundRendererClass", renderer);
	return ret;
 }
 boWarning() << k_funcinfo << "unknown id " << id << " using default renderer" << endl;
 return makeRendererCurrent(QString::null);
}

bool BoGroundRendererManager::makeRendererCurrent(const QString& name)
{
 QStringList list = availableRenderers();
 if (list.isEmpty()) {
	boError() << k_funcinfo << "no groundrenderer available" << endl;
	return false;
 }
 if (name.isEmpty()) {
	if (list.first().isEmpty()) {
		boError() << k_funcinfo << "first groundrenderer found is invalid!" << endl;
		return false;
	}
	QString renderer = boConfig->stringValue("GroundRendererClass");
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

 BoGroundRenderer* renderer = createRenderer(name);
 if (renderer) {
	boDebug() << k_funcinfo << "created renderer" << endl;
	return makeRendererCurrent(renderer);
 } else {
	boError() << k_funcinfo << "Error loading renderer " << name << endl;
 }
 return false;
}



QStringList BoGroundRendererManager::availableRenderers()
{
 QStringList list;
 loadLibrary();
 if (!d->mLibraryFactory) {
	return list;
 }

 BoGroundRendererInformation* info = (BoGroundRendererInformation*)d->mLibraryFactory->create(0, 0, "BoGroundRendererInformation");
 if (!info) {
	// should never happen, as we check for it at loading
	boError() << k_funcinfo << "no information object?!" << endl;
	return list;
 }
 list = info->groundRenderers();
 delete info;

 return list;
}

QString BoGroundRendererManager::currentRendererName() const
{
 if (!currentRenderer()) {
	return QString::null;
 }
 return currentRenderer()->className();
}

BoGroundRenderer* BoGroundRendererManager::createRenderer(const QString& name)
{
 boDebug() << k_funcinfo << name << endl;
 if (!loadLibrary()) {
	return 0;
 }
 BO_CHECK_NULL_RET0(d->mLibraryFactory);
 return (BoGroundRenderer*)d->mLibraryFactory->create(0, 0, name);
}

bool BoGroundRendererManager::loadLibrary()
{
 if (d->mLibrary) {
	if (d->mLibrary->isLoaded()) {
		return true;
	}
	return false;
 }
 QString lib = "libbogroundrendererplugin";
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
		QCString info_name = QCString("BoGroundRendererInformation");
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

bool BoGroundRendererManager::unloadLibrary()
{
 boDebug() << k_funcinfo << "unsetting old renderer" << endl;
 unsetCurrentRenderer();
 boDebug() << k_funcinfo << "deleting factory" << endl;
 delete d->mLibraryFactory;
 d->mLibraryFactory = 0;
 if (d->mLibrary) {
	d->mLibrary->unload();
 }
 delete d->mLibrary;
 d->mLibrary = 0;
 return true;
}

bool BoGroundRendererManager::makeRendererCurrent(BoGroundRenderer* renderer)
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
 mCurrentRenderer = renderer;

 mCurrentRenderer->setLocalPlayerIO(d->mLocalPlayerIO);
 mCurrentRenderer->setViewFrustum(d->mViewFrustum);
 mCurrentRenderer->setMatrices(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewport);

 return true;
}

void BoGroundRendererManager::unsetCurrentRenderer()
{
 if (!mCurrentRenderer) {
	// nothing to do
	return;
 }
 boDebug() << k_funcinfo << endl;
 delete mCurrentRenderer;
 mCurrentRenderer = 0;
}

bool BoGroundRendererManager::checkCurrentRenderer()
{
 BoGroundRendererManager* manager = BoGroundRendererManager::manager();
 if (!manager) {
	BO_NULL_ERROR(manager);
	return false;
 }
 if (!manager->currentRenderer()) {
	boDebug() << k_funcinfo << "getting a default groundrenderer" << endl;
		// pick a default renderer
	bool ret = manager->makeRendererCurrent(QString::null);
	if (ret) {
		boDebug() << k_funcinfo << "default groundrenderer loaded" << endl;
	}
	return ret;
 }
 return true;
}

bool BoGroundRendererManager::reloadPlugin(bool* unusable)
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

void BoGroundRendererManager::setLocalPlayerIO(PlayerIO* io)
{
 d->mLocalPlayerIO = io;
 if (mCurrentRenderer) {
	mCurrentRenderer->setLocalPlayerIO(d->mLocalPlayerIO);
 }
}

void BoGroundRendererManager::setViewFrustum(const float* f)
{
 d->mViewFrustum = f;
 if (mCurrentRenderer) {
	mCurrentRenderer->setViewFrustum(f);
 }
}

void BoGroundRendererManager::setMatrices(const BoMatrix* m, const BoMatrix* p, const int* v)
{
 d->mModelviewMatrix = m;
 d->mProjectionMatrix = p;
 d->mViewport = v;
 if (mCurrentRenderer) {
	mCurrentRenderer->setMatrices(m, p, v);
 }
}

