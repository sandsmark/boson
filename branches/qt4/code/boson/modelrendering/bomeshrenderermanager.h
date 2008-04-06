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
#ifndef BOMESHRENDERERMANAGER_H
#define BOMESHRENDERERMANAGER_H

#include "../bopluginmanager.h"

class BosonModel;
class BoMesh;
class BoMeshLOD;
class BoMeshRenderer;

class BoMeshRendererManagerPrivate;

/**
 * This class takes care of @ref BoMeshRenderer. It loads the plugin(s) for
 * meshrenderers and it sets the currently active renderer.
 *
 * The important methods are @ref makeRendererCurrent (which does what it is
 * named like), @ref currentRenderer (the model rendering code is required to
 * use this) and @ref addModel (@ref BosonModel must call this once a model
 * completed loading).
 * @short Managing of @ref BoMeshRenderer classes
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMeshRendererManager : public BoPluginManager
{
public:
	~BoMeshRendererManager();

	static void initStatic();
	static void deleteStatic();

	/**
	 * @return The BoMeshRendererManager object.
	 **/
	static BoMeshRendererManager* manager();

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

	void unsetCurrentRenderer()
	{
		unsetCurrentPlugin();
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
	BoMeshRenderer* currentRenderer() const
	{
		return (BoMeshRenderer*)currentPlugin();
	}

	/**
	 * Add a (completely loaded!) model to the mesh renderer. The
	 * meshrenderer will probably need to do additional initializations, so
	 * it is very important that the model has been loaded completely at
	 * this point.
	 **/
	void addModel(BosonModel*);

	/**
	 * Must be called on destruction of every model that called @ref
	 * addModel. Calling this for models that never called @ref addModel is
	 * a valid NOP, i.e. will do nothing.
	 **/
	void removeModel(BosonModel*);

	QString currentStatisticsData() const;

protected:
	BoMeshRenderer* createRenderer(const QString& name);

	virtual QString configKey() const;
	virtual QString libname() const;
	virtual KPluginFactory* initWithoutLibrary();

	virtual void initializePlugin();
	virtual void deinitializePlugin();

private:
	BoMeshRendererManager();

private:
	BoMeshRendererManagerPrivate* d;
	static BoMeshRendererManager* mManager;
};

#endif
