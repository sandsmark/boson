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
#include "bopluginmanager.h"
#include "bopluginmanager.moc"

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

class BoPluginManagerPrivate
{
public:
	BoPluginManagerPrivate()
	{
		mLibrary = 0;
		mLibraryFactory = 0;
	}

	QLibrary* mLibrary;
	KLibFactory* mLibraryFactory;
	QStringList mLibraryPlugins; // availablePlugins()
};

BoPluginManager::BoPluginManager()
{
 d = new BoPluginManagerPrivate;
 mCurrentPlugin = 0;
}

BoPluginManager::~BoPluginManager()
{
 unloadLibrary();
 delete d;
}

bool BoPluginManager::makePluginCurrent(const QString& name)
{
 QStringList list = availablePlugins();
 if (list.isEmpty()) {
	boError() << k_funcinfo << "no classes available" << endl;
	return false;
 }
 if (name.isEmpty()) {
	if (list.first().isEmpty()) {
		boError() << k_funcinfo << "first class found is invalid!" << endl;
		return false;
	}
	QString plugin = boConfig->stringValue(configKey());
	if (!list.contains(plugin)) {
		boWarning() << k_funcinfo << "boConfig requested plugin " << plugin << " but it was not available" << endl;
		plugin = list.first();
	}
	return makePluginCurrent(plugin);
 }

 if (!availablePlugins().contains(name)) {
	boError() << k_funcinfo << "class " << name << " not available" << endl;
	return false;
 }
 if (currentPluginName() == name) {
	return true;
 }

 QObject* plugin = createPlugin(name);
 if (plugin) {
	boDebug() << k_funcinfo << "created plugin" << endl;
	return makePluginCurrent(plugin);
 } else {
	boError() << k_funcinfo << "Error loading plugin " << name << endl;
 }
 return false;
}



QStringList BoPluginManager::availablePlugins()
{
 QStringList list;
 loadLibrary();
 if (!d->mLibraryFactory) {
	return list;
 }
 if (!d->mLibraryPlugins.isEmpty()) {
	return d->mLibraryPlugins;
 }

 BoPluginInformation* info = (BoPluginInformation*)d->mLibraryFactory->create(0, 0, "BoPluginInformation");
 if (!info) {
	// should never happen, as we check for it at loading
	boError() << k_funcinfo << "no information object?!" << endl;
	return list;
 }
 list = info->plugins();
 delete info;

 d->mLibraryPlugins = list;
 return list;
}

QString BoPluginManager::currentPluginName() const
{
 if (!currentPlugin()) {
	return QString::null;
 }
 return currentPlugin()->className();
}

QObject* BoPluginManager::createPlugin(const QString& name)
{
 boDebug() << k_funcinfo << name << endl;
 if (!loadLibrary()) {
	return 0;
 }
 BO_CHECK_NULL_RET0(d->mLibraryFactory);
 return d->mLibraryFactory->create(0, 0, name);
}

bool BoPluginManager::loadLibrary()
{
 if (d->mLibrary) {
	if (d->mLibrary->isLoaded()) {
		return true;
	}
	return false;
 }
 QString lib = libname();
 QString file = KGlobal::dirs()->findResource("lib", QString("kde3/plugins/boson/%1.so").arg(lib));

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
		const char* e = dlerror();
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
		QCString info_name = QCString("BoPluginInformation");
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
	KMessageBox::sorry(0, i18n("Plugin could not be loaded - check your installation!\nFailed plugin: %1\nTried file: %2\nError: %3").arg(lib).arg(file).arg(error));
	unloadLibrary();
	exit(1);
 }
 return ret;
}

bool BoPluginManager::unloadLibrary()
{
 boDebug() << k_funcinfo << "unsetting old plugin" << endl;
 unsetCurrentPlugin();
 boDebug() << k_funcinfo << "deleting factory" << endl;
 delete d->mLibraryFactory;
 d->mLibraryFactory = 0;
 d->mLibraryPlugins.clear();
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

bool BoPluginManager::makePluginCurrent(QObject* plugin)
{
 if (!plugin) {
	return false;
 }
 if (plugin == mCurrentPlugin) {
	return true;
 }
 if (mCurrentPlugin) {
	boDebug() << k_funcinfo << "unsetting old plugin" << endl;
	unsetCurrentPlugin();
	boDebug() << k_funcinfo << "old plugin unset" << endl;
 }
 mCurrentPlugin = plugin;

 initializePlugin();

 boConfig->setStringValue(configKey(), currentPluginName());
 return true;
}

void BoPluginManager::unsetCurrentPlugin()
{
 if (!mCurrentPlugin) {
	// nothing to do
	return;
 }
 boDebug() << k_funcinfo << endl;
 deinitializePlugin();
 delete mCurrentPlugin;
 mCurrentPlugin = 0;
}

bool BoPluginManager::checkCurrentPlugin()
{
 if (!currentPlugin()) {
	boDebug() << k_funcinfo << "getting a default plugin" << endl;
		// pick a default plugin
	bool ret = makePluginCurrent(QString::null);
	if (ret) {
		boDebug() << k_funcinfo << "default plugin loaded" << endl;
	}
	return ret;
 }
 return true;
}

bool BoPluginManager::reloadPlugin(bool* unusable)
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
 return checkCurrentPlugin();
}

