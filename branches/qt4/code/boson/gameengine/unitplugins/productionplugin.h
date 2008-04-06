/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2006 Rivo Laks (rivolaks@hot.ee)

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
#ifndef PRODUCTIONPLUGIN_H
#define PRODUCTIONPLUGIN_H

#include "unitplugin.h"
//Added by qt3to4:
#include <Q3ValueList>

template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;

class QDomElement;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class ProductionPlugin : public UnitPlugin
{
public:
	ProductionPlugin(Unit* unit);
	~ProductionPlugin();

	virtual int pluginType() const { return Production; }

	/**
	 * @return Whether there are any productions pending for this unit.
	 * Always FALSE if unitProperties()->canProduce() is FALSE.
	 **/
	inline bool hasProduction() const
	{
		return !mProductions.isEmpty();
	}

	/**
	 * @return The type ID (see @ref UnitProperties::typeId) of the
	 * completed production, if any. 0 If no such production available.
	 **/
	unsigned long int completedProductionId() const;
	ProductionType completedProductionType() const;

	/**
	 * @return The type ID of the current production. 0 if there is no
	 * production.
	 **/
	inline unsigned long int currentProductionId() const
	{
		if (!hasProduction()) {
			return 0;
		}
		return mProductions.first().second;
	}

	inline ProductionType currentProductionType() const
	{
		if (!hasProduction()) {
			return ProduceNothing;
		}
		return mProductions.first().first;
	}

	/**
	 * This is called when the current production has been placed onto the
	 * map. The current production is removed from the internal list and the
	 * next production is started.
	 *
	 * The result of calling this while the current production is not yet
	 * finished is undefined.
	 **/
	void productionPlaced(Unit* produced);

	/**
	 * Add production of type and with id (see @ref UnitProprties::typeId) to the
	 * construction list and start the production.
	 **/
	void addProduction(ProductionType type, unsigned long int id);

	void pauseProduction();
	void unpauseProduction();
	void abortProduction(ProductionType type, unsigned long int id);

	Q3ValueList<QPair<ProductionType, unsigned long int> > productionList() const { return mProductions; }
	bool contains(ProductionType type, unsigned long int id); // { return productionList().contains(typeId);}

	/**
	 * @return A list with all unittypes that this plugin could prodcue if
	 * all production requirements were fullfilled.
	 *
	 * @param producible If non-NULL, this returns alls unittypes that this
	 * plugin can currently produce.
	 * @param impossible This list returns (if non-NULL) all
	 * unittypes that cannot be produced currently, but could be, if the
	 * necessary requirements were met.
	 **/
	Q3ValueList<unsigned long int> allUnitProductions(Q3ValueList<unsigned long int>* producible, Q3ValueList<unsigned long int>* notYetProducible) const;

	/**
	 * This behaves like @ref allUnitProductions with one exception:
	 *
	 * All technologies that already have been researched and thus cannot be
	 * researched anymore, are not returned.
	 **/
	Q3ValueList<unsigned long int> allTechnologyProductions(Q3ValueList<unsigned long int>* producible, Q3ValueList<unsigned long int>* notYetProducible) const;



	/**
	 * See @ref canCurrentlyProduceUnit and @ref canCurrentlyProduceTechnology
	 **/
	bool canCurrentlyProduce(ProductionType p, unsigned long int type) const;

	/**
	 * @return TRUE if this plugin can produce the unit @p type. This is the
	 * case if this plugin is a producer of @p type and all requirements are
	 * fullfilled.
	 **/
	bool canCurrentlyProduceUnit(unsigned long int type) const;

	/**
	 * @return TRUE if this plugin can produce (i.e. research) technology @p
	 * type. This is the case if this plugin is a producer of @p type and
	 * all requirements are fullfilled and if @p type has not yet been
	 * researched.
	 **/
	bool canCurrentlyProduceTechnology(unsigned long int type) const;

	/**
	 * @return The percentage of the production progress. 0 means the
	 * production just started, 100 means the production is completed.
	 **/
	double productionProgress() const;

	virtual void advance(unsigned int);

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*) { }

protected:
	/**
	 * Remove first occurance of type ID id in the production list. Does not
	 * remove anything if id is not in the list.
	 *
	 * This will re-fund any minerals/oil paid for this production so far.
	 **/
	bool removeProduction(ProductionType type, unsigned long int id);

	/**
	 * @overload
	 * Remove the first item from the production list.
	 **/
	bool removeProduction();

	/**
	 * Like @ref removeProduction but won't re-fund the minerals/oil used
	 * for this production.
	 **/
	void removeCompletedProduction();

private:
	/**
	 * Helper method for @ref advance. This is called when the production is
	 * completed and will e.g. place the production (or whatever is
	 * applicable)
	 **/
	void productionCompleted();

private:
	Q3ValueList<QPair<ProductionType, unsigned long int> > mProductions;
	KGameProperty<unsigned int> mProductionState;
	KGameProperty<unsigned long int> mMineralsPaid;
	KGameProperty<unsigned long int> mOilPaid;
};

#endif
