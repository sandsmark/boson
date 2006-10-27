/*
    This file is part of the Boson game
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2006 Rivo Laks (rivolaks@hot.ee)

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
#include "../global.h"

#include <qobject.h>
#include <qptrlist.h>

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
class BosonItemRenderer;
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
class BosonItemContainer;

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
class BosonCanvasRenderer : public QObject
{
	Q_OBJECT
public:
	BosonCanvasRenderer();
	~BosonCanvasRenderer();

	void setGameGLMatrices(const BoGLMatrices*);
	void setCamera(BoGameCamera* camera);
	void setLocalPlayerIO(PlayerIO* io);
	void setCanvas(const BosonCanvas* canvas);

	void setParticlesDirty(bool dirty);

	void reset();
	void initGL();

	/**
	 * Render the currently visible items, effects, ground, ... i.e. the
	 * currently visible part of the canvas.
	 *
	 * The method assumes that the projection has been set already. Same
	 * about the camera settings.
	 *
	 * @param allItems A list of all BosonItemContainer objects existing in
	 * the game.
	 * @param effects A list of all BosonEffect objects existing in the
	 * game.
	 **/
	void paintGL(const QPtrList<BosonItemContainer>& allItems, const QPtrList<BosonEffect>& effects);

	/**
	 * Uses the list of currently visible items to emulate OpenGL "picking"
	 * (i.e. GL_SELECT mode). The items in the specified @p pickRect on the
	 * screen (i.e. @p pickRect is in widget coordinates) are returned.
	 *
	 * @return A list containing all items that were found to be visible in
	 * the specified rect. This method may return (due to the use of
	 * bounding volumes) items that are "nearly" visible, but not actually.
	 * This is intended: it makes sure that the user does not have to click
	 * _exactly_ on a pixel of the item, buit can click "close to it", too.
	 **/
	QValueList<BosonItem*> emulatePickItems(const QRect& pickRect) const;

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

public slots:
	void slotItemRemoved(BosonItem* item);

protected slots:
	void slotAddFeedbackAttack(const QPtrList<Unit>& attacker, const Unit* unit);
	void slotAddFeedbackMoveTo(const QPtrList<Unit>& units, const BoVector2Fixed& pos, bool withAttack);
	void slotWidgetResized();

protected:
	void renderGround(const BosonMap*, RenderFlags flags = Default);
	void renderItems(RenderFlags flags = Default);
	void renderUnitIcons();
	void renderSelections(const BoItemList* selectedItems);
	void renderWater();
	void renderFog(BoVisibleEffects&);
	void renderParticles(BoVisibleEffects&);
	void renderBulletTrailEffects(BoVisibleEffects& visible);
	void renderFadeEffects(BoVisibleEffects& visible, bool enableShaderEffects);
	void renderPathLines(const BosonCanvas* canvas, QValueList<QPoint>& path, bool isFlying, float _z);
	void createRenderItemList(QValueVector<BoRenderItem>* renderItemList, QValueList<Unit*>* radarContactList, const QPtrList<BosonItemContainer>& allItems);
	void createSelectionsList(BoItemList* selections, const QValueVector<BoRenderItem>* relevantItems);
	void createVisibleEffectsList(BoVisibleEffects*, const QPtrList<BosonEffect>& allEffects, unsigned int mapWidth, unsigned int mapHeight);

	void renderShadowMap(const BosonCanvas* canvas);
	void activateShadowMap();
	void deactivateShadowMap();
	void extractViewFrustum(BoVector3Float* points, const BoFrustum& viewFrustum);
	bool mustRenderToTexture(BoVisibleEffects& visible);
	/**
	 * @return TRUE on success, FALSE if an error occurred. If FALSE is
	 * returned, no GL state change has been made.
	 **/
	bool startRenderingToTexture();
	void stopRenderingToTexture();

	void renderBoundingBox(const BosonItem* item);
	void renderBoundingBox(const BoVector3Float& c1, const BoVector3Float& c2);

private:
	BosonCanvasRendererPrivate* d;
};

#endif

