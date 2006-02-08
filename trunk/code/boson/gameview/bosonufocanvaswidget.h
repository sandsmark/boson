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
#ifndef BOSONUFOCANVASWIDGET_H
#define BOSONUFOCANVASWIDGET_H

#include "../boufo/boufo.h"
#include "../bo3dtools.h"

class BosonCanvas;
class PlayerIO;
class Unit;
class UnitProperties;
class BoGameCamera;
class BosonShot;
class BosonShotFragment;
class BosonItem;
class BosonEffect;
class BosonWeapon;
class BosonItemRenderer;
class BosonItemContainer;
template<class T> class QPtrList;

class BosonItemEffects
{
public:
	BosonItemEffects(BosonItem* item);
	~BosonItemEffects();

	const BosonItem* item() const
	{
		return mItem;
	}
	void setEffects(const QPtrList<BosonEffect>& effects, QPtrList<BosonEffect>* takeOwnership);
	void addEffect(BosonEffect* e, QPtrList<BosonEffect>* takeOwnership);
	void clearEffects();
	void removeEffect(BosonEffect* e);
	const QPtrList<BosonEffect>& effects() const;

	void updateEffectsPosition();
	void updateEffectsRotation();

private:
	QPtrList<BosonEffect>* mEffects;
	BosonItem* mItem;
};


class BosonUfoCanvasWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoCanvasWidget : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	BosonUfoCanvasWidget();
	virtual ~BosonUfoCanvasWidget();

	/**
	 * Called once after game starting to initialize the items that are
	 * already in the game.
	 **/
	bool initializeItems();

	void setGameGLMatrices(const BoGLMatrices*);
	void setCamera(BoGameCamera* c);
	void setLocalPlayerIO(PlayerIO* io);
	void setCanvas(const BosonCanvas* canvas);

	virtual void paintWidget();

	/**
	 * @return BosonCanvasRenderer::emulatePickItems
	 **/
	QValueList<BosonItem*> emulatePickItems(const QRect& pickRect) const;

	/**
	 * This method emulates the behaviour of OpenGL "picking", i.e.
	 * GL_SELECT mode, however it does not actually use OpenGL.
	 *
	 * @return The (world) coordinates of the ground at the widget
	 * coordinates @p pickWidgetPos. (-1,-1,-1) if no ground is at that
	 * position.
	 **/
	BoVector3Float emulatePickGroundPos(const QPoint& pickWidgetPos) const;

	void quitGame();

	void cameraChanged();

	bool loadEffectsFromXML(const QDomElement& root);
	bool saveEffectsAsXML(QDomElement& root) const;

	void createEffect(unsigned int id, const BoVector3Fixed& pos, bofixed zrot);
	void createAttachedEffect(int unitid, unsigned int effectid, BoVector3Fixed offset, bofixed zrot);
	void advanceEffects(float elapsed);

public slots:
	void slotAddItemContainerData(BosonItemContainer* c);
	void slotRemoveItemContainerData(BosonItemContainer* c);

	void slotAdvance(unsigned int advanceCallsCount, bool advanceFlag);
	void slotShotFired(BosonShot* shot, BosonWeapon* weapon);
	void slotShotHit(BosonShot* shot);
	void slotUnitDestroyed(Unit* unit);
	void slotFacilityConstructed(Unit* unit);
	void slotFragmentCreated(BosonShotFragment* fragment);

protected:
	void addEffect(BosonEffect* effect);
	void addEffects(const QPtrList<BosonEffect>& effects);
	void animateItems(unsigned int advanceCallsCount);
	void setParticlesDirty(bool);
	void addFacilityConstructedEffects(Unit* facility);
	void initItemEffects();

	BosonItemRenderer* createItemRendererFor(const BosonItemContainer* c);

private:
	BosonUfoCanvasWidgetPrivate* d;
};


#endif

