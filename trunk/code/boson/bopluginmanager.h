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
#ifndef BOPLUGINMANAGER_H
#define BOPLUGINMANAGER_H

#include <qobject.h>

class BoPluginManagerPrivate;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
//template<class Type> class BoPluginManager
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

	virtual QString libname() const = 0;

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
