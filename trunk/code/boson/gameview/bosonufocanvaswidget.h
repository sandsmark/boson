/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
template<class T> class QPtrList;

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

	void setGameGLMatrices(const BoGLMatrices*);
	void setCamera(BoGameCamera* c);
	void setLocalPlayerIO(PlayerIO* io);
	void setCanvas(const BosonCanvas* canvas);

	virtual void paintWidget();

	void quitGame();

	void cameraChanged();

	bool loadEffectsFromXML(const QDomElement& root);
	bool saveEffectsAsXML(QDomElement& root) const;

public slots:
	void slotAdvance(unsigned int advanceCallsCount, bool advanceFlag);
	void slotItemAdded(BosonItem*);
	void slotItemRemoved(BosonItem*);
	void slotShotFired(BosonShot* shot, BosonWeapon* weapon);
	void slotShotHit(BosonShot* shot);
	void slotUnitDestroyed(Unit* unit);
	void slotFacilityConstructed(Unit* unit);
	void slotFragmentCreated(BosonShotFragment* fragment);

protected:
	void addEffect(BosonEffect* effect);
	void addEffects(const QPtrList<BosonEffect>& effects);
	void animateItems(unsigned int advanceCallsCount);
	void advanceEffects(float elapsed);
	void setParticlesDirty(bool);
	void addFacilityConstructedEffects(Unit* facility);

private:
	BosonUfoCanvasWidgetPrivate* d;
};


#endif

