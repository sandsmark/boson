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
#ifndef BOGROUNDRENDERERMANAGER_H
#define BOGROUNDRENDERERMANAGER_H

#include "bopluginmanager.h"

class BoGroundRenderer;
class PlayerIO;
class BoMatrix;
class BoFrustum;

class BoGroundRendererManagerPrivate;

/**
 * @short Managing of @ref BoGroundRenderer classes
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGroundRendererManager : public BoPluginManager
{
public:
	~BoGroundRendererManager();

	static void initStatic();
	static void deleteStatic();

	/**
	 * @return The BoGroundRendererManager object.
	 **/
	static BoGroundRendererManager* manager();

	/**
	 * @return A list of available renderers. The list contains the names
	 * (i.e. the classnames) of the renderers.
	 **/
	QStringList availableRenderers();

	/**
	 * @param className The name of the renderer, or @ref QString::null for
	 * the first renderer found
	 **/
	bool makeRendererCurrent(const QString& className)
	{
		return makePluginCurrent(className);
	}

	/**
	 * Check for @ref currentRenderer being NULL and try to load a default
	 * renderer when it is NULL.
	 * @return TRUE when we have a current renderer, FALSE if no current
	 * renderer is set and no default renderer could get loaded.
	 **/
	static bool checkCurrentRenderer()
	{
		if (!manager()) {
			return false;
		}
		return manager()->checkCurrentPlugin();
	}

	/**
	 * @return The @ref QObject::className of the @ref currentRenderer or
	 * @ref QString::null if none is set. This name can be used in @ref
	 * makeRendererCurrent.
	 **/
	inline QString currentRendererName() const
	{
		return currentPluginName();
	}

	/**
	 * @return The currently used renderer. See @ref makeRendererCurrent
	 **/
	BoGroundRenderer* currentRenderer() const
	{
		return (BoGroundRenderer*)currentPlugin();
	}

	void setLocalPlayerIO(PlayerIO*);
	void setViewFrustum(const BoFrustum*);
	void setMatrices(const BoMatrix* modelviewMatrix, const BoMatrix* projectionMatrix, const int* viewport);

	void unsetCurrentRenderer()
	{
		unsetCurrentPlugin();
	}

	QString currentStatisticsData() const;

protected:
	BoGroundRenderer* createRenderer(const QString& name);

	virtual QString configKey() const;
	virtual QString libname() const;
	virtual KLibFactory* initWithoutLibrary();

	virtual void initializePlugin();
	virtual void deinitializePlugin();

private:
	BoGroundRendererManager();

private:
	BoGroundRendererManagerPrivate* d;
	static BoGroundRendererManager* mManager;
};

#endif
