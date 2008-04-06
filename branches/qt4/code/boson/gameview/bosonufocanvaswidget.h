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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONUFOCANVASWIDGET_H
#define BOSONUFOCANVASWIDGET_H

#include "../boufo/boufo.h"
#include "../bo3dtools.h"
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>

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
template<class T> class Q3PtrList;

class BosonItemEffects
{
public:
	BosonItemEffects(BosonItem* item);
	~BosonItemEffects();

	const BosonItem* item() const
	{
		return mItem;
	}
	void setEffects(const Q3PtrList<BosonEffect>& effects, Q3PtrList<BosonEffect>* takeOwnership);
	void addEffect(BosonEffect* e, Q3PtrList<BosonEffect>* takeOwnership);
	void clearEffects();
	void removeEffect(BosonEffect* e);
	const Q3PtrList<BosonEffect>& effects() const;

	void updateEffectsPosition();
	void updateEffectsRotation();

private:
	Q3PtrList<BosonEffect>* mEffects;
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

	Q3ValueList<BosonItem*> itemsAtWidgetRect(const QRect& widgetRect) const;

	/**
	 * Like @ref itemsAtWidgetRect but returns @ref Unit objects only.
	 **/
	Q3ValueList<Unit*> unitsAtWidgetRect(const QRect& widgetRect) const;

	/**
	 * Like @ref unitsAtWidgetRect with a rect of width=height=1, but this
	 * method selects a single unit from the list.
	 *
	 * This can be used to find a single unit that is under the mouse or so.
	 **/
	Unit* unitAtWidgetPos(const QPoint& widgetPos) const;

	/**
	 * @return BosonCanvasRenderer::emulatePickItems
	 **/
	Q3ValueList<BosonItem*> emulatePickItems(const QRect& pickRect) const;

	/**
	 * This method emulates the behaviour of OpenGL "picking", i.e.
	 * GL_SELECT mode, however it does not actually use OpenGL.
	 *
	 * @param worldPos The result, i.e. the world coordinates of the ground
	 * at the widgetcoordinates @p pickWidgetPos is placed here if this
	 * method is successful. Will remain unmodified otherwise.
	 *
	 * @return TRUE on success (the result is in @p worldPos), otherwise
	 * FALSE.
	 **/
	bool emulatePickGroundPos(const QPoint& pickWidgetPos, BoVector3Float* worldPos) const;

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
	void addEffects(const Q3PtrList<BosonEffect>& effects);
	void animateItems(unsigned int advanceCallsCount);
	void setParticlesDirty(bool);
	void addFacilityConstructedEffects(Unit* facility);
	void initItemEffects();

	BosonItemRenderer* createItemRendererFor(const BosonItemContainer* c);

private:
	BosonUfoCanvasWidgetPrivate* d;
};


#endif

