/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONCANVASRENDERER_H
#define BOSONCANVASRENDERER_H

#include "../defines.h"
#include "../bo3dtools.h"

class BosonCanvas;
class BosonCursor;
class BoSelection;
class Player;
class PlayerIO;
class Unit;
class UnitProperties;
class BoGameCamera;
class BoAutoGameCamera;
class BoItemList;
class BosonItem;
class BoPixmapRenderer;
class BoLight;
class BoFontInfo;
class BosonScript;
class BoVisibleEffects;
class BosonMap;
class BosonEffect;
class BoSpecificAction;
class BoGLMatrices;
class BoRenderItem;

class KGameChat;
class KGameIO;
class QDomElement;
template<class T> class QPtrList;
template<class T> class QValueVector;
template<class T> class QValueList;

class BosonCanvasRendererPrivate;
/**
 * @short This class renders everything that is actually "part of the game".
 *
 * The canvas renderer basically renders the canvas. That means it renders the
 * ground and everything on it (items/units, effects, ...), i.e. everything that
 * is "part of the game".
 *
 * It does <em>not</em> render anything that is used to control the game -
 * cmdframe, labels, cursor, ... These things are handled elsewhere.
 *
 * Note that this class is rather considered to be a part of the @ref
 * BosonGameView, not of @ref BosonCanvas!
 *
 * @author Andreas Beckermann <b_mann@gmx.de
 **/
class BosonCanvasRenderer
{
public:
	BosonCanvasRenderer();
	~BosonCanvasRenderer();

	void setGameGLMatrices(const BoGLMatrices*);
	void setCamera(BoGameCamera* camera);
	void setLocalPlayerIO(PlayerIO* io);

	void setParticlesDirty(bool dirty);

	void reset();
	void initGL();
	void paintGL(const BosonCanvas* canvas);
	unsigned int renderedItems() const;
	unsigned int renderedCells() const;
	unsigned int renderedParticles() const;
	int textureBindsCells() const;
	int textureBindsItems() const;
	int textureBindsWater() const;
	int textureBindsParticles() const;

	BoGameCamera* camera() const;
	PlayerIO* localPlayerIO() const;
	const BoFrustum& viewFrustum() const;

protected:
	void renderGround(const BosonMap*);
	void renderItems(const BoItemList* allCanvasItems);
	void renderSelections(const BoItemList* selectedItems);
	void renderWater();
	void renderFog(BoVisibleEffects&);
	void renderParticles(BoVisibleEffects&);
	void renderBulletTrailEffects(BoVisibleEffects& visible);
	void renderFadeEffects(BoVisibleEffects& visible);
	void renderPathLines(const BosonCanvas* canvas, QValueList<QPoint>& path, bool isFlying, float _z);
	void createRenderItemList(QValueVector<BoRenderItem>* renderItemList, const BoItemList* allItems);
	void createSelectionsList(BoItemList* selections, const QValueVector<BoRenderItem>* relevantItems);
	void createVisibleEffectsList(BoVisibleEffects*, const QPtrList<BosonEffect>& allEffects, unsigned int mapWidth, unsigned int mapHeight);

	void renderBoundingBox(const BosonItem* item);
	void renderBoundingBox(const BoVector3Float& c1, const BoVector3Float& c2);

private:
	BosonCanvasRendererPrivate* d;
};

#endif

