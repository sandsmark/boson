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
#ifndef BOPLUGINMANAGER_H
#define BOPLUGINMANAGER_H

#include <qobject.h>
#include <config.h> // USE_BO_PLUGINS

class BoPluginManagerPrivate;
class KLibFactory;

/**
 * @internal
 * Do not use this macro yourself. Use BOPLUGIN_MANAGER instead.
 **/
#define BOPLUGIN_MANAGER_LIBNAME(className, libName) \
	QString className::libname() const { return #libName; }

/**
 * @internal
 * Do not use this macro yourself. Use BOPLUGIN_MANAGER instead.
 **/
#if USE_BO_PLUGINS
#define BOPLUGIN_MANAGER_INITWITHOUTLIBRARY(className, libName) \
	KLibFactory* className::initWithoutLibrary() { return 0; }
#else
#define BOPLUGIN_MANAGER_INITWITHOUTLIBRARY(className, libName) \
	extern "C" { void* init_##libName(); }  \
	KLibFactory* className::initWithoutLibrary() \
		{ \
			return (KLibFactory*)init_##libName(); \
		}
#endif

/**
 * This macro should be in the .cpp file of the plugin manager of your plugin,
 * that is of the class that derives from @ref BoPluginManager.
 *
 * It implements @ref BoPluginManager::libname and @ref
 * BoPluginManager::initWithoutLibrary.
 * @param className The class name of the plugin manager (for example @ref
 *        BoMeshRendererManager)
 * @param libName The name of the library (without .so suffix) that provides the
 *        plugin. For example libbomeshrendererplugin
 **/
#define BOPLUGIN_MANAGER(className, libName) \
		BOPLUGIN_MANAGER_LIBNAME(className, libName) \
		BOPLUGIN_MANAGER_INITWITHOUTLIBRARY(className, libName)

/**
 * Every boson plugin must provide at least two plain C functions, which are the
 * interface to the plugin.
 * <li>init_libname() where "libname" is the name of the library, without the
 *     .la (but including the "lib"). This function must return a pointer to the
 *     factory of the plugin, which must be derived from @ref KLibFactory.
 * <li>version_libname() where "libname" is equal to the one above. This returns
 *     simply BOSON_VERSION from boversion.h. This function is used by the plugin
 *     loader to find out whether the plugin can be used.
 *
 * Both of these functions are automatically added by this macro. Make sure that
 * you #include <boversion.h> ! (or tell me how to do that in a macro).
 *
 * Place this macro into the .cpp file of the factory of your plugin.
 **/
// AB: probably add a #include <boversion.h> to the file defining this macro
#define BO_EXPORT_PLUGIN_FACTORY( libname, factory ) \
	extern "C" { \
		void* init_##libname() { return new factory; } \
		int version_##libname() { return BOSON_VERSION; } \
	}




/**
 * If you want to create a new class in a plugin, for example a new
 * groundrenderer or a new meshrenderer:
 * @li Write the class in the plugin directory (e.g. boson/meshrenderer/) and
 *     add it to the Makefile.am
 * @li Add it to the factory class of that plugin (e.g.
 *     boson/meshrenderer/bomeshrendererfactory.cpp)
 * @li Make sure that your new class derives from the base class that every
 *     class in that plugin needs to derive from. This class is the interface to
 *     the application and therefore is different for every plugin (for example
 *     @ref BoMeshRenderer for the meshrenderer plugin). Note that this class is
 *     declared in a file outside the plugin directory.
 *
 * If you want to create a new plugin:
 * (note that these docs were written long after I wrote the last plugin. it
 * might be possible that I miss some things here)
 * @li Create a base class for your plugin. All methods that the application
 *     should be able to call in your plugin must be declared in this class
 *     (pure virtual usually). This is the interface class that the application
 *     will use.
 * @li Create a manager class (such as @ref BoMeshRendererManager) deriving from
 *     @ref BoPluginManager and implement the required methods. Add the
 *     BOPLUGIN_MANAGER macro to the .cpp file.
 * @li Create a new directory and add it to the Makefile.am.
 * @li Create a a factory class deriving from @ref KLibFactory. Add the
 *     BO_EXPORT_PLUGIN_FACTORY macro to the .cpp file.
 * @li AB: note sure here: create a class deriving from @ref BoPluginInformation
 *     and name it BoPluginInformation_libname, where libname is the name of
 *     your library.
 * @li That's it. Query your manager class for a new plugin and you'll get it.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoPluginManager
{
public:
	BoPluginManager();
	virtual ~BoPluginManager();

	/**
	 * @param unusable If non-null this is set to TRUE, when reloading
	 * failed and the library is unusable now, or to FALSE if it still can
	 * be used. If reloading succeeded, this is always set to FALSE.
	 **/
	bool reloadPlugin(bool* unusable);

	/**
	 * @return A list of available classes. The list contains the available
	 * classnames.
	 **/
	QStringList availablePlugins();

	/**
	 * @param className The name of the plugin , or @ref QString::null for
	 * the first plguin found
	 **/
	bool makePluginCurrent(const QString& className);

	void unsetCurrentPlugin();

	/**
	 * Check for @ref currentPlugin being NULL and try to load a default
	 * plugin when it is NULL.
	 * @return TRUE when we have a current plugin, FALSE if no current
	 * plugin is set and no default plugin could get loaded.
	 **/
	bool checkCurrentPlugin();

	/**
	 * @return The @ref QObject::className of the @ref currentPlugin or
	 * @ref QString::null if none is set. This name can be used in @ref
	 * makePluginCurrent.
	 **/
	QString currentPluginName() const;

	/**
	 * @return The currently used plugin . See @ref makePluginCurrent
	 **/
	QObject* currentPlugin() const
	{
		return mCurrentPlugin;
	}

protected:
	bool loadLibrary();
	bool unloadLibrary();
	QObject* createPlugin(const QString& name);
	bool makePluginCurrent(QObject* plugin);

	/**
	 * @return The config key which is used to lookup the default plugin
	 * class.
	 **/
	virtual QString configKey() const = 0;

	/**
	 * @return The name of the library without .so suffix. Could be used as
	 * parameter to @ref QLibrary.
	 **/
	virtual QString libname() const = 0;

	/**
	 * This method calls the init function of the plugin that is normally
	 * called right after loading the library. This method calls the
	 * function directly, that is without loading the library and resolving
	 * the symbol, therefore the library must be linked directly to the
	 * program.
	 *
	 * This method is a no-op if the program is compiled with plugin
	 * support.
	 **/
	virtual KLibFactory* initWithoutLibrary() = 0;

	virtual void initializePlugin() = 0;
	virtual void deinitializePlugin() = 0;

private:
	BoPluginManagerPrivate* d;

	QObject* mCurrentPlugin;
};

/**
 * A simple class providing information about what classes a plugin provides.
 * You need to derive from this class for your plugin and list all
 * available plugin classnames in @ref plugins.
 *
 * Note that the name of your derived class MUST be
 * BoPluginInformation_plugin, where "plugin" is the name of your library
 * file (e.g. "libbomeshrendererplugin")
 **/
class BoPluginInformation : public QObject
{
	Q_OBJECT
public:
	BoPluginInformation() : QObject()
	{
	}
	~BoPluginInformation()
	{
	}
	virtual QStringList plugins() const = 0;
};

#endif
