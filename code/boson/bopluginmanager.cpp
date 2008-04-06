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
#include "bopluginmanager.h"
#include "bopluginmanager.moc"

#include "../bomemory/bodummymemory.h"
#include "bosonconfig.h"
#include "boversion.h"

#include <bodebug.h>
#include <config.h> // USE_BO_PLUGINS

#include <kapplication.h>
#include <k3staticdeleter.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klibloader.h>

#include <qstringlist.h>
#include <q3ptrlist.h>
#include <qlibrary.h>
//Added by qt3to4:
#include <Q3CString>

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
	boError(800) << k_funcinfo << "no classes available" << endl;
	return false;
 }
 if (name.isEmpty()) {
	if (list.first().isEmpty()) {
		boError(800) << k_funcinfo << "first class found is invalid!" << endl;
		return false;
	}
	QString plugin = boConfig->stringValue(configKey());
	if (!list.contains(plugin)) {
		boWarning(800) << k_funcinfo << "boConfig requested plugin " << plugin << " but it was not available" << endl;
		plugin = list.first();
	}
	return makePluginCurrent(plugin);
 }

 if (!availablePlugins().contains(name)) {
	boError(800) << k_funcinfo << "class " << name << " not available" << endl;
	return false;
 }
 if (currentPluginName() == name) {
	return true;
 }

 QObject* plugin = createPlugin(name);
 if (plugin) {
	boDebug(800) << k_funcinfo << "created plugin" << endl;
	return makePluginCurrent(plugin);
 } else {
	boError(800) << k_funcinfo << "Error loading plugin " << name << endl;
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
	boError(800) << k_funcinfo << "no information object?!" << endl;
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
 boDebug(800) << k_funcinfo << name << endl;
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
#if USE_BO_PLUGINS
 if (d->mLibraryFactory) {
	return true;
 }
#endif
 QString lib = libname();
 QString file;

 QString error;
 bool ret = true;
 typedef KLibFactory* (*init_function)();
 typedef int (*version_function)();

#if USE_BO_PLUGINS
 init_function init_func = 0;
 version_function version_func = 0;
 file = KGlobal::dirs()->findResource("lib", QString("kde3/plugins/boson/%1.so").arg(lib));
 if (file.isEmpty()) {
	error = i18n("Unable to find a file for this plugin");
	boError(800) << k_funcinfo << error << endl;
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
			error = QString("%1 - reported error:\n%2").arg(error).arg(e);
		}
	}
 }

 if (ret) {
	boDebug(800) << k_funcinfo << "library " << lib << " loaded. resolving symbols" << endl;

	if (ret) {
		Q3CString init_name = Q3CString("init_") + lib.latin1();
		init_func = (init_function)d->mLibrary->resolve(init_name);
		if (!init_func) {
			ret = false;
			error = i18n("Could not resolve %1").arg(init_name);
			boError(800) << k_funcinfo << error << endl;
		}
	}
	if (ret) {
		typedef void (*FunctionType)();
		Q3CString version_name = Q3CString("version_") + lib.latin1();
		version_func = (version_function)d->mLibrary->resolve(version_name);
		if (!version_func) {
			ret = false;
			error = i18n("Could not resolve %1").arg(version_name);
			boError(800) << k_funcinfo << error << endl;
		}
	}
 }

 if (ret) {
	boDebug(800) << k_funcinfo << "symbols resolved. checking version" << endl;
	int version = version_func();
	if (version != BOSON_VERSION) {
		error = i18n("Version mismatch: plugin compiled for %1, you are running %2").arg(version).arg(BOSON_VERSION);
		boError(800) << k_funcinfo << error << endl;
		ret = false;
	}
 }
#else
 file = "(no plugin)";
#endif // USE_BO_PLUGINS
 if (ret) {
	boDebug(800) << k_funcinfo << "initializing plugin" << endl;
#if USE_BO_PLUGINS
	d->mLibraryFactory = init_func();
#else
	d->mLibraryFactory = initWithoutLibrary();
#endif
	if (!d->mLibraryFactory) {
		error = i18n("Could not load factory (init returned NULL)");
		boError(800) << k_funcinfo << error << endl;
		ret = false;
	}
 }
 if (ret) {
	boDebug(800) << k_funcinfo << "searching for information object" << endl;
	Q3CString info_name = Q3CString("BoPluginInformation");
	QObject* info = d->mLibraryFactory->create(0, 0, info_name);
	if (!info) {
		error = i18n("Could not find the information object. searched for: %1").arg(info_name);
		boError(800) << k_funcinfo << error << endl;
		ret = false;
	} else {
		delete info;
	}
 }
 if (ret) {
	boDebug(800) << k_funcinfo << "library should be ready to use now" << endl;
 } else {
	boError(800) << k_funcinfo << "library loading failed. fatal error." << endl;
	KMessageBox::sorry(0, i18n("Plugin could not be loaded - check your installation!\nFailed plugin: %1\nTried file: %2\nError: %3").arg(lib).arg(file).arg(error));
	unloadLibrary();
	exit(1);
 }
 return ret;
}

bool BoPluginManager::unloadLibrary()
{
 boDebug(800) << k_funcinfo << "unsetting old plugin" << endl;
 unsetCurrentPlugin();
 boDebug(800) << k_funcinfo << "deleting factory" << endl;
 delete d->mLibraryFactory;
 d->mLibraryFactory = 0;
 d->mLibraryPlugins.clear();
 bool ret = true;
 if (d->mLibrary) {
	if (!d->mLibrary->unload()) {
		boError(800) << k_funcinfo << "unloading lib failed!" << endl;
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
	boDebug(800) << k_funcinfo << "unsetting old plugin" << endl;
	unsetCurrentPlugin();
	boDebug(800) << k_funcinfo << "old plugin unset" << endl;
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
 boDebug(800) << k_funcinfo << endl;
 deinitializePlugin();
 delete mCurrentPlugin;
 mCurrentPlugin = 0;
}

bool BoPluginManager::checkCurrentPlugin()
{
 if (!currentPlugin()) {
	boDebug(800) << k_funcinfo << "getting a default plugin" << endl;
	// pick a default plugin
	bool ret = makePluginCurrent(QString::null);
	if (ret) {
		boDebug(800) << k_funcinfo << "default plugin loaded" << endl;
	}
	return ret;
 }
 return true;
}

bool BoPluginManager::reloadPlugin(bool* unusable)
{
 if (!unloadLibrary()) {
	boError(800) << k_funcinfo << "unloading failed" << endl;
	if (unusable) {
		*unusable = true;
	}
	return false;
 }
 if (!loadLibrary()) {
	boError(800) << k_funcinfo << "library loading failed" << endl;
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

