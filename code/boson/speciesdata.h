/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef SPECIESDATA_H
#define SPECIESDATA_H

#include "defines.h"
#include "global.h"
#include <qstring.h>
#include <qcolor.h>

class BosonModel;
class BosonSound;
class UnitProperties;
class UnitBase;
class BosonParticleSystemProperties;
class BosonWeaponProperties;

class QPixmap;
class QStringList;
class QColor;
template<class T> class QDict;

/**
 * Here we store all un-modifyable data, such as images or unit models for a
 * @ref SpeciesTheme. The @ref SpeciesTheme constructor requests a SpeciesData
 * instance using @ref speciesData. If there is already an object available
 * (from another player with the same species) this object will be used and
 * therefore no data wasted. Otherwise a new instance is created.
 *
 * You'll need to call @ref addTeamColor before you can do much with this class.
 * Much data can't be shared between different players, as they depend on the
 * current teamcolor. Anyway we might find a solution for this one day and so
 * the basic structure is already here.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class SpeciesData
{
public:
	SpeciesData(const QString& speciesPath);
	~SpeciesData();

	/**
	 * Initialize the static object. Needs to be called at least once
	 **/
	static void initStatic();

	/**
	 * @param species The path to the directory of the species.
	 * @return The SpeciesData instance for the specified species. Will
	 * create the instance if not yet existing.
	 **/
	static SpeciesData* speciesData(const QString& speciesPath);

	/**
	 * Add the specified @p color to the SpeciesData instance. You need to
	 * call this before using one of the color-dependant functions.
	 **/
	void addTeamColor(const QColor& color);

	/**
	 * Remove @p color from the instance. You can leave this out in order to
	 * use the same data for the next game (i.e. you won't have to reload
	 * anything).
	 *
	 * Note that re-using the data is currently not supported, as we delete
	 * the OpenGL context. Anyway - non-color dependand data won't be
	 * deleted here.
	 **/
	void removeTeamColor(const QColor& color);

	/**
	 * Load the action (move, mine, attack, ...) pixmaps.
	 * @return TRUE if successful, otherwise FALSE
	 **/
	bool loadActionPixmaps();

	/**
	 * Load the model for the specified unit @prop and the color @p
	 * teamColor.
	 **/
	void loadUnitModel(const UnitProperties* prop, const QColor& teamColor);

	/**
	 * Load the overview pixmaps for the unit @p prop in the color @p
	 * teamColor
	 * @return TRUE if successfull, otherwise FALSE
	 **/
	bool loadUnitOverview(const UnitProperties* prop, const QColor& teamColor);

	/**
	 * Load the @ref BosonParticleSystemProperties for all particles
	 * speciefied in the particles.boson file of this theme.
	 **/
	void loadParticleSystemProperties();

	/**
	 * @return The unit model for @p unitType. Will <em>not</em> load the
	 * model - see @ref loadUnitModel
	 **/
	BosonModel* unitModel(unsigned long int unitType, const QColor& teamColor) const;

	/**
	 * @return The specified object model from @p file. Loads the model if
	 * necessary.
	 **/
	BosonModel* objectModel(const QString& file, const QColor& teamColor);

	QPixmap* actionPixmap(UnitAction action) const;
	QPixmap* bigOverview(unsigned long int unitType, const QColor& teamColor) const;
	QPixmap* smallOverview(unsigned long int unitType, const QColor& teamColor) const;
	const BosonParticleSystemProperties* particleSystemProperties(unsigned long int id) const;
	QPixmap* upgradePixmapByName(const QString& name);

	/**
	 * @return The absolute path to the species directory
	 * (boson/themes/species/your_species/)
	 **/
	QString themePath() const;

	/**
	 * @return The model filename. This is the relative filename, i.e.
	 * currently just "unit.3ds"
	 **/
	static QString unitModelFile();

protected:

	/**
	 * AB: doc might be obsolete, since the method moved to another class
	 * Load a pixmap from path with mash (or not). This is used for all unit
	 * pixmaps: small/big overview and sprites. It is <em>not</em> yet used
	 * fot the shot sprites. Use @ref loadShotPixmap for these.
	 *
	 * @param fileName The path to load the pixmap from. Should be an abolute
	 * filename.
	 * @param pix The pixmap that is loaded.
	 **/
	bool loadUnitImage(const QColor& color, const QString& fileName, QImage &image);

private:
	class TeamColorData;

	/**
	 * @return The teamcolor object of @p color or NULL if it hasn't been
	 * added yet. See @ref addTeamColor
	 **/
	TeamColorData* teamColorData(const QColor& color) const;

private:
	static QDict<SpeciesData>* mSpeciesData;

private:
	class SpeciesDataPrivate;
	SpeciesDataPrivate* d;
};

#endif

