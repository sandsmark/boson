/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOUNITDECORATIONRENDERER_H
#define BOUNITDECORATIONRENDERER_H

class BosonItem;
class Unit;
class BoGameCamera;
class BosonShot;

class BoUnitDecorationRendererPrivate;
class BoUnitDecorationRenderer
{
public:
	enum Backend {
		BackendDefault = 0,
		BackendDebug = 1
	};
public:
	BoUnitDecorationRenderer();
	~BoUnitDecorationRenderer();

	void setCamera(BoGameCamera* camera);
	BoGameCamera* camera() const;

	void setBackend(Backend backend);

	void renderDecoration(BosonItem* item);


private:
	BoUnitDecorationRendererPrivate* d;
};

/**
 * @short Base class for all @ref BoUnitDecorationRenderer backends
 *
 * Used internally by @ref BoUnitDecorationRenderer only.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUnitDecorationRendererBackend
{
public:
	BoUnitDecorationRendererBackend();
	~BoUnitDecorationRendererBackend();

#if 0
	void setCamera(BoGameCamera* c)
	{
		mCamera = c;
	}
	BoGameCamera* camera() const
	{
		return mCamera;
	}
#endif

public:
	virtual void renderUnitDecoration(Unit* unit) = 0;
	virtual void renderShotDecoration(BosonShot* shot) = 0;

private:
#if 0
	BoGameCamera* mCamera;
#endif
};

class BoUnitDecorationRendererBackendDebug : public BoUnitDecorationRendererBackend
{
public:
	BoUnitDecorationRendererBackendDebug();
	~BoUnitDecorationRendererBackendDebug();

public:
	virtual void renderUnitDecoration(Unit* unit);
	virtual void renderShotDecoration(BosonShot* shot);

};

class BoUnitDecorationRendererBackendDefault : public BoUnitDecorationRendererBackend
{
public:
	BoUnitDecorationRendererBackendDefault();
	~BoUnitDecorationRendererBackendDefault();

public:
	virtual void renderUnitDecoration(Unit* unit);
	virtual void renderShotDecoration(BosonShot* shot);

};

#endif

