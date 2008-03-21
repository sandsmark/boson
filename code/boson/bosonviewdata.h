/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONVIEWDATA_H
#define BOSONVIEWDATA_H

#include <qobject.h>

template <class T1, class T2> class QMap;
template <class T> class QPtrList;
class BosonItem;
class BosonItemEffects;
class BosonItemRenderer;
class SpeciesTheme;
class SpeciesData;
class BosonViewData;
class BosonEffect;
class BosonItemEffects;
class BosonItemContainer;
class BosonGroundTheme;
class BosonGroundThemeData;


/**
 * A pointer to the global @ref BosonViewData object.
 *
 * Please read the @ref BosonViewData docs on where NOT to use this. As a rule
 * of thumb: do NOT use the view data outsde of the gameview!
 **/
#define boViewData BosonViewData::globalViewData()


/**
 * @short Container for all (GUI-) data of a @ref Bosonitem
 *
 * This container should be created by the game view once a @ref BosonItem
 * object is added (by the game engine). The container groups all data that
 * logically belong to the item, but does not belong to the game engine,
 * especially all visible data (effects, item renderer, ...)
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonItemContainer
{
public:
	BosonItemContainer(BosonItem* item);
	~BosonItemContainer();

	void setEffects(BosonItemEffects* e)
	{
		mEffects = e;
	}
	void setItemRenderer(BosonItemRenderer* r)
	{
		mItemRenderer = r;
	}

	BosonItem* item() const
	{
		return mItem;
	}
	BosonItemRenderer* itemRenderer() const
	{
		return mItemRenderer;
	}
	BosonItemEffects* effects() const
	{
		return mEffects;
	}

private:
	BosonItem* mItem;
	BosonItemRenderer* mItemRenderer;
	BosonItemEffects* mEffects;
};



class BosonViewDataPrivate;
/**
 * @short Collection of data that belongs to the (game) view.
 *
 * The intention of this class is to map an object from the game engine (e.g.
 * a @ref BosonItem or a @ref SpeciesTheme) to an object containing the relevant
 * view data. Such data could be textures, models, sound files, ..., i.e.
 * everything that does not influence the game logic (and therefore should not
 * reside in the game engine).
 *
 * Note that an object (there should be only one per application) of this class
 * should NOT be used outside of the gameview. This object provides access to
 * gameview relevant data only, so using it outside of the gameview is breaking
 * the design:
 * <code>
 * + application framework
 * ++ game engine
 * ++ gameview
 * +++ viewdata
 * </code>
 * neither the application framework (mainwidget, ...) nor the gameengine should
 * ever touch the viewdata.
 *
 * Note that to this rule there are two exceptions where using an object of this
 * class is allowed (these are the reason why this class is not part of the
 * gameview!):
 * @li @ref BosonStarting: This class sets up the gameview and loads the data.
 *                         It is possible to remove viewdata access from this
 *                         class, but not intended. @ref BosonStarting is
 *                         allowed to access the viewdata.
 * @li borender: The borender application requires access to at least the @ref
 *               SpeciesData objects. This is for convenience only (we can do
 *               the same without allowing access to the viewdata or the
 *               SpeciesData objects), but it definitely makes sense.
 *               Note however, that "borender" is per definition a view by
 *               itself, so it does not really break the rule (the view is just
 *               not named "gameview", but "borender").
 *
 * AB: note that this description of allowed use is based on my opinion as of
 * today (05/08/09) and may change in the future. I hope not though.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonViewData : public QObject
{
	Q_OBJECT
public:
	BosonViewData(QObject* parent);
	~BosonViewData();

	static void setGlobalViewData(BosonViewData* viewData)
	{
		mGlobalViewData = viewData;
	}
	static BosonViewData* globalViewData()
	{
		return mGlobalViewData;
	}

	void addSpeciesTheme(const SpeciesTheme* theme);
	void removeSpeciesTheme(const SpeciesTheme* theme);
	SpeciesData* speciesData(const SpeciesTheme* theme) const;

	BosonItemContainer* itemContainer(BosonItem* item);
	const QPtrList<BosonItemContainer>& allItemContainers() const;

	void addGroundTheme(const BosonGroundTheme* theme);
	void removeGroundTheme(const BosonGroundTheme* theme);
	BosonGroundThemeData* groundThemeData(const BosonGroundTheme* theme) const;

public slots:
	void slotAddItemContainerFor(BosonItem* item);
	void slotRemoveItemContainerFor(BosonItem* item);

signals:
	/**
	 * Emitted when a new item container was added. See @ref
	 * slotAddItemContainerFor.
	 **/
	void signalItemContainerAdded(BosonItemContainer* container);

	/**
	 * Emitted before an item container is removed. See @ref
	 * slotRemoveItemContainerFor.
	 *
	 * Note that at this time the pointer is still valid. It will be deleted
	 * right after this signal was emitted.
	 **/
	void signalItemContainerAboutToBeRemoved(BosonItemContainer* container);

private:
	BosonViewDataPrivate* d;
	static BosonViewData* mGlobalViewData;
};

#endif

