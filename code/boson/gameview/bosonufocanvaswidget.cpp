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

#include "bosonufocanvaswidget.h"
#include "bosonufocanvaswidget.moc"

#include "../bomemory/bodummymemory.h"
#include "../no_player.h"
#include "bosoncanvasrenderer.h"
#include "../bosoncanvas.h"
#include "../bosonmap.h"
#include "../bosonmodel.h"
#include "../speciestheme.h"
#include "../speciesdata.h"
#include "../bosongroundtheme.h"
#include "../playerio.h"
#include "../unitproperties.h"
#include "../bosonconfig.h"
#include "../bosonprofiling.h"
#include "../items/bosonshot.h"
#include "../items/bosonitemrenderer.h"
#include "../bosonweapon.h"
#include "../unit.h"
#include "../bosoneffectmanager.h"
#include "../bosoneffectproperties.h"
#include "../bosoneffect.h"
#include "../bowaterrenderer.h"
#include "../bocamera.h"
#include "../boitemlist.h"
#include "../bosonviewdata.h"
#include "bodebug.h"

#include <klocale.h>

#include <qtimer.h>
#include <qvaluelist.h>
#include <qptrdict.h>
#include <qdom.h>


BosonItemEffects::BosonItemEffects(BosonItem* item)
{
 mItem = item;
 mEffects = new QPtrList<BosonEffect>();
}

BosonItemEffects::~BosonItemEffects()
{
 clearEffects();
 delete mEffects;
}

const QPtrList<BosonEffect>& BosonItemEffects::effects() const
{
 return *mEffects;
}

void BosonItemEffects::setEffects(const QPtrList<BosonEffect>& effects, QPtrList<BosonEffect>* takeOwnership)
{
 clearEffects();
// boDebug() << k_funcinfo << effects.count() << endl;
 for (QPtrListIterator<BosonEffect> it(effects); it.current(); ++it) {
	addEffect(it.current(), takeOwnership);
 }
}

void BosonItemEffects::addEffect(BosonEffect* e, QPtrList<BosonEffect>* takeOwnership)
{
 e->setOwnerId(mItem->id());
 mEffects->append(e);
 if (takeOwnership) {
	takeOwnership->append(e);
 }
}

void BosonItemEffects::removeEffect(BosonEffect* e)
{
 mEffects->removeRef(e);
}

void BosonItemEffects::clearEffects()
{
 for (QPtrListIterator<BosonEffect> it(*mEffects); it.current(); ++it) {
	it.current()->setOwnerId(0);
 }
 mEffects->clear();
}

void BosonItemEffects::updateEffectsPosition()
{
 BoVector3Fixed pos(item()->x() + item()->width() / 2, item()->y() + item()->height() / 2, item()->z());
 pos.canvasToWorld();
 for (QPtrListIterator<BosonEffect> it(*mEffects); it.current(); ++it) {
	it.current()->setPosition(pos);
 }
 mItem->setEffectsPositionDirty(false);
}

void BosonItemEffects::updateEffectsRotation()
{
 BoVector3Fixed rotation(item()->xRotation(), item()->yRotation(), item()->rotation());
 for (QPtrListIterator<BosonEffect> it(*mEffects); it.current(); ++it) {
	it.current()->setRotation(rotation);
 }
 mItem->setEffectsRotationDirty(false);
}




class BosonUfoCanvasWidgetPrivate
{
public:
	BosonUfoCanvasWidgetPrivate()
	{
		mGameGLMatrices = 0;
		mCanvasRenderer = 0;
		mCamera = 0;
		mLocalPlayerIO = 0;
		mCanvas = 0;

		mEffectManager = 0;
	}
	const BoGLMatrices* mGameGLMatrices;
	BosonCanvasRenderer* mCanvasRenderer;
	BoGameCamera* mCamera;
	PlayerIO* mLocalPlayerIO;
	const BosonCanvas* mCanvas;

	QPtrList<BosonEffect> mEffects;

	BosonEffectManager* mEffectManager;
};

BosonUfoCanvasWidget::BosonUfoCanvasWidget()
		: BoUfoCustomWidget()
{
 setName("BosonUfoCanvasWidget");

 d = new BosonUfoCanvasWidgetPrivate();
 BosonEffectPropertiesManager::initStatic();
 d->mEffectManager = new BosonEffectManager();

 d->mCanvasRenderer = new BosonCanvasRenderer();
 d->mCanvasRenderer->initGL();

 if (!boViewData) {
	BO_NULL_ERROR(boViewData);
	return;
 }
 connect(boViewData, SIGNAL(signalItemContainerAdded(BosonItemContainer*)),
		this, SLOT(slotAddItemContainerData(BosonItemContainer*)));
 connect(boViewData, SIGNAL(signalItemContainerAboutToBeRemoved(BosonItemContainer*)),
		this, SLOT(slotRemoveItemContainerData(BosonItemContainer*)));
 if (boViewData->allItemContainers().count() > 0) {
	boError() << k_funcinfo << "itemcontainer list should be empty at this point" << endl;
 }
}

BosonUfoCanvasWidget::~BosonUfoCanvasWidget()
{
 boDebug() << k_funcinfo << endl;
 quitGame();
 delete d->mCanvasRenderer;
 delete d->mEffectManager;
 BosonEffectPropertiesManager::deleteStatic();
 delete d;

 boDebug() << k_funcinfo << "donee" << endl;
}

void BosonUfoCanvasWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
 d->mCanvasRenderer->setGameGLMatrices(d->mGameGLMatrices);
}

void BosonUfoCanvasWidget::setCamera(BoGameCamera* c)
{
 d->mCamera = c;
}

void BosonUfoCanvasWidget::setLocalPlayerIO(PlayerIO* io)
{
 d->mLocalPlayerIO = io;
 d->mCanvasRenderer->setLocalPlayerIO(d->mLocalPlayerIO);
}

void BosonUfoCanvasWidget::setCanvas(const BosonCanvas* canvas)
{
 if (d->mCanvas) {
	disconnect(d->mCanvas, 0, this, 0);
 }
 d->mCanvas = canvas;
 d->mCanvasRenderer->setCanvas(d->mCanvas);
 if (d->mCanvas) {
	connect(d->mCanvas, SIGNAL(signalShotFired(BosonShot*, BosonWeapon*)),
		this, SLOT(slotShotFired(BosonShot*, BosonWeapon*)));
	connect(d->mCanvas, SIGNAL(signalShotHit(BosonShot*)),
		this, SLOT(slotShotHit(BosonShot*)));
	connect(d->mCanvas, SIGNAL(signalUnitDestroyed(Unit*)),
		this, SLOT(slotUnitDestroyed(Unit*)));
	connect(d->mCanvas, SIGNAL(signalFragmentCreated(BosonShotFragment*)),
		this, SLOT(slotFragmentCreated(BosonShotFragment*)));

	initItemEffects();
 }
}

void BosonUfoCanvasWidget::initItemEffects()
{
 boDebug() << k_funcinfo << endl;
 if (d->mEffects.count() > 0) {
	boDebug() << k_funcinfo << "effects already initialized. most likely loaded from xml - we are loading a game, no initializing required." << endl;
	return;
 }
 for (BoItemList::ConstIterator it = d->mCanvas->allItems()->begin(); it != d->mCanvas->allItems()->end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (u->isFacility()) {
			Facility* f = (Facility*)u;
			if (f->isConstructionComplete()) {
				addFacilityConstructedEffects(u);
			}
		}
	}
 }
}

void BosonUfoCanvasWidget::setParticlesDirty(bool dirty)
{
 d->mCanvasRenderer->setParticlesDirty(dirty);
}

void BosonUfoCanvasWidget::quitGame()
{
 d->mCanvasRenderer->reset();

 while (!d->mEffects.isEmpty()) {
	BosonEffect* e = d->mEffects.take(0);
	if (e->ownerId() && d->mCanvas && boViewData) {
		BosonItem* owner = d->mCanvas->findItem(e->ownerId());
		if (owner) {
			BosonItemEffects* itemEffects = 0;
			BosonItemContainer* c = boViewData->itemContainer(owner);
			if (c) {
				itemEffects = c->effects();
			}
			if (itemEffects) {
				itemEffects->removeEffect(e);
			}
		}
	}
	delete e;
 }
}

void BosonUfoCanvasWidget::addEffect(BosonEffect* e)
{
 d->mEffects.append(e);
}

void BosonUfoCanvasWidget::addEffects(const QPtrList<BosonEffect>& effects)
{
 for (QPtrListIterator<BosonEffect> it(effects); it.current(); ++it) {
	addEffect(it.current());
 }
}

void BosonUfoCanvasWidget::slotAdvance(unsigned int advanceCallsCount, bool advanceFlag)
{
 Q_UNUSED(advanceFlag);
 PROFILE_METHOD
 setParticlesDirty(true);
 animateItems(advanceCallsCount);
 advanceEffects(0.05);

 boProfiling->push("Advance Water");
 boWaterRenderer->update(0.05);
 boProfiling->pop();
}

void BosonUfoCanvasWidget::animateItems(unsigned int advanceCallsCount)
{
 Q_UNUSED(advanceCallsCount);
 for (QPtrListIterator<BosonItemContainer> it(boViewData->allItemContainers()); it.current(); ++it) {
	BosonItemRenderer* r = it.current()->itemRenderer();
	if (r) {
		r->animate();
	}
 }
}

void BosonUfoCanvasWidget::advanceEffects(float elapsed)
{
 BO_CHECK_NULL_RET(d->mCanvas);
 QPtrList<BosonEffect> removeEffects;
 for (QPtrListIterator<BosonItemContainer> it(boViewData->allItemContainers()); it.current(); ++it) {
	BosonItemEffects* e = it.current()->effects();
	if (!e->item()) {
		continue;
	}
	if (e->effects().count() == 0) {
		continue;
	}
	if (e->item()->isEffectsPositionDirty()) {
		e->updateEffectsPosition();
	}
	if (e->item()->isEffectsRotationDirty()) {
		e->updateEffectsRotation();
	}
 }
 for (QPtrListIterator<BosonEffect> it(d->mEffects); it.current(); ++it) {
	BosonEffect* e = it.current();
	if (!e->hasStarted()) {
		e->update(elapsed);
	} else {
		e->markUpdate(elapsed);
	}
	if (!e->isActive()) {
		if (e->ownerId()) {
			// Remove the effect from owner
			BosonItem* owner = d->mCanvas->findItem(e->ownerId());
			if (owner) {
				BosonItemEffects* itemEffects = 0;
				BosonItemContainer* c = boViewData->itemContainer(owner);
				if (c) {
					itemEffects = c->effects();
				}
				if (itemEffects) {
					itemEffects->removeEffect(e);
				}
			}
		}
		removeEffects.append(e);
	}
 }
 d->mEffects.setAutoDelete(true);
 while (removeEffects.count() > 0) {
	BosonEffect* e = removeEffects.take(0);
	d->mEffects.removeRef(e);
 }
}

void BosonUfoCanvasWidget::cameraChanged()
{
 setParticlesDirty(true);
 BO_CHECK_NULL_RET(d->mCamera);
 QPtrListIterator<BosonEffect> it(d->mEffects);
 while (it.current()) {
	if (it.current()->type() == BosonEffect::ParticleEnvironmental) {
		it.current()->setPosition(d->mCamera->cameraPos().toFixed());
	}
	++it;
 }
}

bool BosonUfoCanvasWidget::loadEffectsFromXML(const QDomElement& root)
{
 if (!d->mCanvas) {
	BO_NULL_ERROR(d->mCanvas);
	return false;
 }
 bool ret = true;
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("Effect"));

 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement effect = list.item(i).toElement();
	bool ok = false;

	unsigned int propId = 0;
	unsigned int ownerId = 0;

	propId = effect.attribute(QString::fromLatin1("PropId")).toUInt(&ok);
	if (!propId || !ok) {
		boError() << k_funcinfo << "invalid number for PropId" << endl;
		ret = false;
		continue;
	}
	ownerId = effect.attribute(QString::fromLatin1("OwnerId")).toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid number for OwnerId" << endl;
		ret = false;
		continue;
	}

	// AB: ownerId is an item here
	// if that item is not present, then it belongs to a player that is not
	// being loaded in this game. we ignore the effect then.
	if (!d->mCanvas->findItem(ownerId)) {
		continue;
	}

	const BosonEffectProperties* prop = boEffectPropertiesManager->effectProperties(propId);
	if (!prop) {
		boError() << k_funcinfo << "Null effect properties with id " << propId << endl;
		ret = false;
		continue;
	}

	BoVector3Fixed pos, rot;
	if (!loadVector3FromXML(&pos, effect, "Position")) {
		ret = false;
		continue;
	}
	if (!loadVector3FromXML(&rot, effect, "Rotation")) {
		ret = false;
		continue;
	}

	BosonEffect* e = prop->newEffect(pos, rot);
	if(!e) {
		boError() << k_funcinfo << "NULL effect created! id: " << propId << "; owner: " << ownerId << endl;
		ret = false;
		continue;
	}
	if(!e->loadFromXML(effect)) {
		ret = false;
		delete e;
		continue;
	}
	addEffect(e);
	if (e->ownerId() != 0) {
		// Find effect's owner
		BosonItem* owner = d->mCanvas->findItem(e->ownerId());
		BosonItemEffects* itemEffects = 0;
		if (owner) {
			BosonItemContainer* c = boViewData->itemContainer(owner);
			if (c) {
				itemEffects = c->effects();
			}
		}
		if (itemEffects) {
			itemEffects->addEffect(e, 0);
		} else {
			boError() << k_funcinfo << "Can't find owner item with id " << e->ownerId() << " for effect!" << endl;
			e->makeObsolete();  // Maybe delete immediately?
			ret = false;
		}
	}
 }

 return ret;
}

bool BosonUfoCanvasWidget::saveEffectsAsXML(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();

 // Save effects
 QPtrListIterator<BosonEffect> effectIt(d->mEffects);
 while (effectIt.current()) {
	QDomElement e = doc.createElement(QString::fromLatin1("Effect"));
	effectIt.current()->saveAsXML(e);
	root.appendChild(e);
	++effectIt;
 }
 return true;
}

void BosonUfoCanvasWidget::slotShotFired(BosonShot* shot, BosonWeapon* weapon)
{
 BO_CHECK_NULL_RET(boViewData);
 BO_CHECK_NULL_RET(shot);
 BO_CHECK_NULL_RET(weapon);
 BO_CHECK_NULL_RET(weapon->properties());
 BO_CHECK_NULL_RET(weapon->unit());
 BoVector3Fixed pos(weapon->unit()->centerX(), weapon->unit()->centerY(), weapon->unit()->z());
 d->mEffectManager->loadWeaponType(weapon->properties());
 addEffects(d->mEffectManager->newShootEffects(weapon->properties(), pos, weapon->unit()->rotation()));

 BO_CHECK_NULL_RET(weapon->speciesTheme());
 BO_CHECK_NULL_RET(boViewData->speciesData(weapon->speciesTheme()));
 boViewData->speciesData(weapon->speciesTheme())->playSound(weapon->properties(), SoundWeaponShoot);
}

void BosonUfoCanvasWidget::slotShotHit(BosonShot* shot)
{
 BO_CHECK_NULL_RET(shot);
 BosonItemContainer* c = boViewData->itemContainer(shot);
 BO_CHECK_NULL_RET(c);

 BosonItemEffects* effects = c->effects();
 BoVector3Fixed pos(shot->x(), shot->y(), shot->z());

 if (effects && shot->properties()) {
	d->mEffectManager->loadWeaponType(shot->properties());
	switch (shot->type()) {
		case BosonShot::Bullet:
			effects->setEffects(d->mEffectManager->newFlyEffects(shot->properties(), pos, 0), &d->mEffects);
			break;
		case BosonShot::Rocket:
			break;
		case BosonShot::Explosion:
			break;
		case BosonShot::Mine:
			break;
		case BosonShot::Bomb:
			break;
		case BosonShot::Fragment:
		{
			BoVector3Fixed pos(shot->centerX(), shot->centerY(), shot->z());
			BosonShotFragment* fragment = (BosonShotFragment*)shot;
			const UnitProperties* prop = fragment->unitProperties();
			d->mEffectManager->loadUnitType(prop);
			addEffects(d->mEffectManager->newExplodingFragmentHitEffects(prop, pos));
			break;
		}
		case BosonShot::Missile:
			break;
		default:
			break;
	}
 }

 // Make shot's effects (e.g. smoke traces) obsolete
 if (effects && effects->effects().count() > 0) {
	QPtrListIterator<BosonEffect> it(effects->effects());
	while (it.current()) {
		it.current()->makeObsolete();
		++it;
	}
 }
 if (shot->properties()) {
	// Add hit effects
	d->mEffectManager->loadWeaponType(shot->properties());
	addEffects(d->mEffectManager->newHitEffects(shot->properties(), pos));
 }

 if (shot->properties() && shot->properties()->speciesTheme()) {
	BO_CHECK_NULL_RET(boViewData);
	BO_CHECK_NULL_RET(boViewData->speciesData(shot->properties()->speciesTheme()));
	boViewData->speciesData(shot->properties()->speciesTheme())->playSound(shot->properties(), SoundWeaponHit);
 }

 BO_CHECK_NULL_RET(effects);
 // this does NOT delete effects, but removes from the item only
 effects->clearEffects();
}

void BosonUfoCanvasWidget::slotUnitDestroyed(Unit* unit)
{
 BO_CHECK_NULL_RET(boViewData);
 BO_CHECK_NULL_RET(unit);
 BO_CHECK_NULL_RET(unit->speciesTheme());
 BO_CHECK_NULL_RET(boViewData->speciesData(unit->speciesTheme()));
 BosonItemContainer* c = boViewData->itemContainer(unit);
 BO_CHECK_NULL_RET(c);

 boViewData->speciesData(unit->speciesTheme())->playSound(unit, SoundReportDestroyed);
 BosonItemEffects* e = c->effects();
 if (e && e->effects().count() > 0) {
	// Make all unit's effects obsolete
	QPtrListIterator<BosonEffect> it(e->effects());
	for (; it.current(); ++it) {
		it.current()->makeObsolete();
		it.current()->setOwnerId(0);
	}
	e->clearEffects();

 }

 // Pos is center of unit
 BoVector3Fixed pos(unit->x() + unit->width() / 2, unit->y() + unit->height() / 2, unit->z());
 //pos += unit->unitProperties()->hitPoint();
 // Add destroyed effects
 d->mEffectManager->loadUnitType(unit->unitProperties());
 addEffects(d->mEffectManager->newDestroyedEffects(unit->unitProperties(), pos[0], pos[1], pos[2]));
}

void BosonUfoCanvasWidget::slotFragmentCreated(BosonShotFragment* fragment)
{
 BosonItemContainer* c = boViewData->itemContainer(fragment);
 BO_CHECK_NULL_RET(c);
 BosonItemEffects* effects = c->effects();
 BO_CHECK_NULL_RET(effects);
 BoVector3Fixed pos(fragment->x(), fragment->y(), fragment->z());
 d->mEffectManager->loadUnitType(fragment->unitProperties());
 effects->setEffects(d->mEffectManager->newExplodingFragmentFlyEffects(fragment->unitProperties(), pos), &d->mEffects);
}

void BosonUfoCanvasWidget::slotFacilityConstructed(Unit* unit)
{
 addFacilityConstructedEffects(unit);
}

void BosonUfoCanvasWidget::addFacilityConstructedEffects(Unit* unit)
{
 BO_CHECK_NULL_RET(unit);
 BosonItemContainer* c = boViewData->itemContainer(unit);
 BO_CHECK_NULL_RET(c);
 BosonItemEffects* effects = c->effects();
 BO_CHECK_NULL_RET(effects);
 float x = unit->x() + unit->width() / 2;
 float y = unit->y() + unit->height() / 2;
 float z = unit->z();
 d->mEffectManager->loadUnitType(unit->unitProperties());
 effects->setEffects(d->mEffectManager->newConstructedEffects(unit->unitProperties(), x, y, z), &d->mEffects);
}

void BosonUfoCanvasWidget::slotAddItemContainerData(BosonItemContainer* c)
{
 BO_CHECK_NULL_RET(c);

 BosonItem* item = c->item();
 BO_CHECK_NULL_RET(item);

 if (c->effects()) {
	boError() << k_funcinfo << "called twice" << endl;
	return;
 }

 BosonItemEffects* effects = new BosonItemEffects(item);
 c->setEffects(effects);

 BosonItemRenderer* itemRenderer = createItemRendererFor(c);
 if (!itemRenderer) {
	boWarning() << k_funcinfo << "unable to initialize item renderer for item " << item->id() << endl;
 }
 c->setItemRenderer(itemRenderer);

 float x = item->x() + item->width() / 2;
 float y = item->y() + item->height() / 2;
 float z = item->z();
 if (RTTI::isUnit(item->rtti())) {
	Unit* u = (Unit*)item;
	if (u->isMobile()) {
		d->mEffectManager->loadUnitType(u->unitProperties());
		effects->setEffects(d->mEffectManager->newConstructedEffects(u->unitProperties(), x, y, z), &d->mEffects);
	} else {
		// facilities need to be constructed first
	}
 } else if (RTTI::isShot(item->rtti())) {
	BosonShot* shot = (BosonShot*)item;
	switch (shot->type()) {
		case BosonShot::Bullet:
			break;
		case BosonShot::Rocket:
		{
			d->mEffectManager->loadWeaponType(shot->properties());
			BoVector3Fixed pos(shot->x(), shot->y(), shot->z());
			effects->setEffects(d->mEffectManager->newFlyEffects(shot->properties(), pos, 0.0), &d->mEffects);
			break;
		}
		case BosonShot::Explosion:
			break;
		case BosonShot::Mine:
			break;
		case BosonShot::Bomb:
			break;
		case BosonShot::Fragment:
			break;
		case BosonShot::Missile:
		{
			d->mEffectManager->loadWeaponType(shot->properties());
			BoVector3Fixed pos(shot->x(), shot->y(), shot->z());
			effects->setEffects(d->mEffectManager->newFlyEffects(shot->properties(), pos, 0.0), &d->mEffects);
			break;
		}
		default:
			break;
	}
 }
}

void BosonUfoCanvasWidget::slotRemoveItemContainerData(BosonItemContainer* c)
{
 BO_CHECK_NULL_RET(c);
 BosonItemEffects* effects = c->effects();
 BosonItemRenderer* itemRenderer = c->itemRenderer();
 if (effects) {
	effects->clearEffects();
 }
 c->setEffects(0);
 c->setItemRenderer(0);
 delete effects;
 delete itemRenderer;
}

void BosonUfoCanvasWidget::paintWidget()
{
 PROFILE_METHOD;
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the beginning of this method" << endl;
 }
 d->mCanvasRenderer->setCamera(d->mCamera);

 // Store the original libufo matrices and set our 3d matrices
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadMatrixf(d->mGameGLMatrices->projectionMatrix().data());
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadMatrixf(d->mGameGLMatrices->modelviewMatrix().data());

 glPushAttrib(GL_ALL_ATTRIB_BITS);
 glViewport(d->mGameGLMatrices->viewport()[0],
		d->mGameGLMatrices->viewport()[1],
		d->mGameGLMatrices->viewport()[2],
		d->mGameGLMatrices->viewport()[3]);

 d->mCanvasRenderer->paintGL(boViewData->allItemContainers(), d->mEffects);

 glPopAttrib();

  // Restore the original libufo matrices
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the end of this method" << endl;
 }
}


#include "../items/bosonshot.h" // for an explosion hack below
BosonItemRenderer* BosonUfoCanvasWidget::createItemRendererFor(const BosonItemContainer* c)
{
 BO_CHECK_NULL_RET0(c);
 BO_CHECK_NULL_RET0(c->item());
 BO_CHECK_NULL_RET0(c->item()->speciesTheme());
 BO_CHECK_NULL_RET0(boViewData);
 SpeciesData* speciesData = boViewData->speciesData(c->item()->speciesTheme());
 BO_CHECK_NULL_RET0(speciesData);
 if (c->itemRenderer()) {
	boWarning() << k_funcinfo << "called twice" << endl;
	return c->itemRenderer();
 }

 // TODO: a virtual bool providesModel() method would be handy here. by default
 // it would return true, but e.g. the explosion class would return false.
 bool providesModel = true;

 if (RTTI::isShot(c->item()->rtti())) {
	// AB: this is a hack. implement a providesModel() method instead.
	if (((BosonShot*)c->item())->type() == BosonShot::Explosion) {
		providesModel = false;
	}
 }

 if (boConfig->boolValue("ForceDisableModelLoading")) {
	providesModel = false;
 }

 // AB: note that we can of course use renderers other than the model renderer.
 // e.g. we might use some special renderer for bullets or so (i.e. not use any
 // model).
 // but as this is not required yet, a simple if (providesModel) is sufficient
 // here.
 BosonItemRenderer* itemRenderer = 0;
 if (providesModel) {
	itemRenderer = new BosonItemModelRenderer(c->item());
	BosonModel* model = 0;
	QString id = c->item()->getModelIdForItem();
	int index = -1;
	if (!id.isEmpty()) {
		index = id.find(':');
	}
	if (index >= 0) {
		QString type = id.left(index);
		QString file = id.right(id.length() - index - 1);
		if (type == "shot") {
			model = boViewData->speciesData(c->item()->speciesTheme())->objectModel(file);
		} else if (type == "unit") {
			bool ok;
			unsigned long int unitType = file.toULong(&ok);
			if (!ok) {
				boError() << k_funcinfo << file << " is not a number in id string " << id << endl;
			} else {
				model = boViewData->speciesData(c->item()->speciesTheme())->unitModel(unitType);
			}
		} else {
			boError() << k_funcinfo << "unrecognized type \"" << type << "\" of id string " << id << endl;
		}
	} else {
		boError() << k_funcinfo << "unrecognized format of id string: " << id << endl;
	}
	if (!itemRenderer->setModel(model)) {
		boWarning() << k_funcinfo << "itemRenderer()->setModel() failed" << endl;
		delete itemRenderer;
		itemRenderer = new BosonItemRenderer(c->item());
	}
 } else {
	itemRenderer = new BosonItemRenderer(c->item());
 }
 return itemRenderer;
}


