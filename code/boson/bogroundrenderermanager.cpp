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
	}

	PlayerIO* mLocalPlayerIO;
	const float* mViewFrustum;
	const BoMatrix* mModelviewMatrix;
	const BoMatrix* mProjectionMatrix;
	const int* mViewport;
};

BoGroundRendererManager::BoGroundRendererManager() : BoPluginManager()
{
 d = new BoGroundRendererManagerPrivate;
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
 if (ret && currentRenderer()) {
	QString renderer = currentRenderer()->className();
	boConfig->setStringValue("GroundRendererClass", renderer);
	return ret;
 }
 boWarning() << k_funcinfo << "unknown id " << id << " using default renderer" << endl;
 return makeRendererCurrent(QString::null);
}

QStringList BoGroundRendererManager::availableRenderers()
{
 return availablePlugins();
}

BoGroundRenderer* BoGroundRendererManager::createRenderer(const QString& name)
{
 return (BoGroundRenderer*)createPlugin(name);
}

QString BoGroundRendererManager::libname() const
{
 return "libbogroundrendererplugin";
}

QString BoGroundRendererManager::configKey() const
{
 // "GroundRenderer" is already in use
 return QString::fromLatin1("GroundRendererClass");
}

void BoGroundRendererManager::initializePlugin()
{
 BO_CHECK_NULL_RET(currentRenderer());
 currentRenderer()->setLocalPlayerIO(d->mLocalPlayerIO);
 currentRenderer()->setViewFrustum(d->mViewFrustum);
 currentRenderer()->setMatrices(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewport);
}

void BoGroundRendererManager::deinitializePlugin()
{
}

void BoGroundRendererManager::setLocalPlayerIO(PlayerIO* io)
{
 d->mLocalPlayerIO = io;
 if (currentRenderer()) {
	currentRenderer()->setLocalPlayerIO(d->mLocalPlayerIO);
 }
}

void BoGroundRendererManager::setViewFrustum(const float* f)
{
 d->mViewFrustum = f;
 if (currentRenderer()) {
	currentRenderer()->setViewFrustum(f);
 }
}

void BoGroundRendererManager::setMatrices(const BoMatrix* m, const BoMatrix* p, const int* v)
{
 d->mModelviewMatrix = m;
 d->mProjectionMatrix = p;
 d->mViewport = v;
 if (currentRenderer()) {
	currentRenderer()->setMatrices(m, p, v);
 }
}

QString BoGroundRendererManager::currentStatisticsData() const
{
 BoGroundRenderer* current = currentRenderer();
 if (!current) {
	return QString::null;
 }
 return current->statisticsData();
}

