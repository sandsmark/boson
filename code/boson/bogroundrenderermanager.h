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
#ifndef BOGROUNDRENDERERMANAGER_H
#define BOGROUNDRENDERERMANAGER_H

#include <qobject.h>

class BoGroundRenderer;
class PlayerIO;
class BoMatrix;

class BoGroundRendererManagerPrivate;

/**
 * @short Managing of @ref BoGroundRenderer classes
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGroundRendererManager
{
public:
	~BoGroundRendererManager();

	static void initStatic();

	/**
	 * @param unusable If non-null this is set to TRUE, when reloading
	 * failed and the library is unusable now, or to FALSE if it still can
	 * be used. If reloading succeeded, this is always set to FALSE.
	 **/
	bool reloadPlugin(bool* unusable);

	/**
	 * @return The BoGroundRendererManager object.
	 **/
	static BoGroundRendererManager* manager()
	{
		initStatic();
		return mManager;
	}

	/**
	 * @return A list of available renderers. The list contains the names
	 * (i.e. the classnames) of the renderers.
	 **/
	QStringList availableRenderers();

	/**
	 * @param className The name of the renderer, or @ref QString::null for
	 * the first renderer found
	 **/
	bool makeRendererCurrent(const QString& className);
	bool makeRendererIdCurrent(int id); // obsolete

	/**
	 * Check for @ref currentRenderer being NULL and try to load a default
	 * renderer when it is NULL.
	 * @return TRUE when we have a current renderer, FALSE if no current
	 * renderer is set and no default renderer could get loaded.
	 **/
	static bool checkCurrentRenderer();

	/**
	 * @return The @ref QObject::className of the @ref currentRenderer or
	 * @ref QString::null if none is set. This name can be used in @ref
	 * makeRendererCurrent.
	 **/
	QString currentRendererName() const;

	/**
	 * @return The currently used renderer. See @ref makeRendererCurrent
	 **/
	BoGroundRenderer* currentRenderer() const
	{
		return mCurrentRenderer;
	}

	void setLocalPlayerIO(PlayerIO*);
	void setViewFrustum(const float*);
	void setMatrices(const BoMatrix* modelviewMatrix, const BoMatrix* projectionMatrix, const int* viewport);

	void unsetCurrentRenderer();

protected:
	bool loadLibrary();
	bool unloadLibrary();
	BoGroundRenderer* createRenderer(const QString& name);
	bool makeRendererCurrent(BoGroundRenderer* renderer);

private:
	BoGroundRendererManager();

private:
	BoGroundRendererManagerPrivate* d;
	static BoGroundRendererManager* mManager;

	BoGroundRenderer* mCurrentRenderer;
};

#endif
