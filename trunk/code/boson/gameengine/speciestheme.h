/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef SPECIESTHEME_H
#define SPECIESTHEME_H

#include "../defines.h"
#include "../global.h"
#include <qstring.h>
#include <qcolor.h>

class BosonTextureArray;
class BosonModel;
class UnitProperties;
class UnitBase;
class Unit;
class UpgradeProperties;
class BosonWeaponProperties;
class BoAction;

class QDomElement;
class QPixmap;
class QStringList;
class QColor;
template<class T> class QValueList;
template<class T> class QIntDict;

class SpeciesThemePrivate;
/**
 * Stores player's species - this includes units' textures and models, sounds,
 * properties and overviews as well as action pixmaps (attack, move and stop)
 * and player's teamcolor.
 *
 * This class provides methods for loading and teamcoloring pixmaps and for
 * retrieving them later.
 *
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class SpeciesTheme
{
public:
	/**
	 * Construct a theme. See @ref loadTheme.
	 **/
	SpeciesTheme();

	~SpeciesTheme();


	/**
	 * Load a theme. Look for index.unit files in the units directory,
	 * read and store these information - see @ref readUnitConfigs.
	 *
	 * Note that loadTheme is quite fast as it does <em>not</em> preload the
	 * pixmaps of the units. These are loaded as soon as a unit is accessed
	 * first time.
	 * @param species The theme name to be loaded. Must not be i18n'ed as it
	 * is used in the directoryname.
	 * @param teamColor The color of this team or QColor(0,0,0) for a default
	 * color
	 **/
	bool loadTheme(const QString& species, const QColor&);

	/**
	 * @return The color of the team of this player. See also @ref
	 * setTeamColor
	 **/
	const QColor& teamColor() const { return mTeamColor; }

	/**
	 * Change the team color. You can only call this function if no pixmaps
	 * have been loaded that use the teamcolor (like units)!
	 *
	 * So you cannot use this anymore as soon as you called @ref
	 * loadUnitImage
	 * @return True if the color could be changed, otherwise false.
	 **/
	bool setTeamColor(const QColor& color);

	/**
	 * @return A default color. This color differs after every call.
	 **/
	static QColor defaultColor();

	/**
	 * Load all available technologies. This must be done before starting game
	 **/
	bool loadTechnologies();

	/**
	 * @return Concatenation of all @ref UnitProperties::md5 sums in this
	 * theme, separated by "\n"s
	 **/
	QCString unitPropertiesMD5() const;

	/**
	 * Reads the default entries from the config file of the specified unit
	 * and applies them. If values are not available in the file the
	 * hardcoded defaults are used.
	 *
	 * Use this if you actually want to create a new unit (e.g. produce
	 * one), don't use it if values will differ from the defaults (although
	 * it wouldn't hurt if you would still use this)
	 **/
	void loadNewUnit(Unit* unit);

	const UnitProperties* unitProperties(unsigned long int unitType) const;

	/**
	 * See also @ref allUnitsNonConst
	 **/
	UnitProperties* nonConstUnitProperties(unsigned long int unitType) const;

	bool hasUnitProperties(unsigned long int unitType) const;

	/**
	 * @return The technology with @p id. Equivalent to
	 * upgrade("Technology", id)
	 **/
	const UpgradeProperties* technology(unsigned long int id) const;

	/**
	 * @return The specified upgrade of this SpeciesTheme
	 * @param type Describes the group of upgrades. Currently only
	 * "Technology" is provided. Other values might be "Moral" or
	 * "Experience" or ...
	 * @param id The ID of the desired upgrade. The ID is unique inside the
	 * groupd of upgrades of the same @p type in this theme. Different
	 * upgrades with different types may have the same @p id.
	 **/
	const UpgradeProperties* upgrade(const QString& type, unsigned long int id) const;

	/**
	 * @return the path to the species theme (ending with
	 * boson/themes/species/your_species/)
	 **/
	const QString& themePath() const { return mThemePath; }

	/**
	 * @return The typeIds of all facilities in this theme. See also @ref
	 * UnitProperties::typeId
	 **/
	QValueList<unsigned long int> allFacilities() const;

	/**
	 * @return The typeIds of all mobile units in this theme. See also @ref
	 * UnitProperties::typeId
	 **/
	QValueList<unsigned long int> allMobiles() const;

	/**
	 * @return A list of all unit properties in this theme
	 **/
	QValueList<const UnitProperties*> allUnits() const;

	const QIntDict<UnitProperties>* allUnitsNonConst() const;

	/**
	 * @return The names of all objects of this theme, as they could
	 * be provided to @ref objectModel.
	 * @param fileList if non-null here are the filenames returned, that
	 * belong to the names.
	 **/
	QStringList allObjects(QStringList* files = 0) const;

	/**
	 * @return A list of all units that have a @ref UnitProperties::producer
	 * specified in producerList.
	 **/
	QValueList<unsigned long int> productions(const QValueList<unsigned long int>& producerList) const;

	/**
	 * @return A list of all technologies that have a producer specified in
	 * producerList.
	 **/
	QValueList<unsigned long int> technologies(const QValueList<unsigned long int>& producerList) const;

	/**
	 * Reset this theme. Delete all pixmaps, unitProperties, ...
	 **/
	void reset();

	/**
	 * @return A list of all possible species. Note that the list contains
	 * the index.species files - so remove index.species from every entry to
	 * get the actual species directory.
	 **/
	static QStringList availableSpecies();

	/**
	 * @return The directory of the default species ("human")
	 **/
	static QString defaultSpecies();

	static QString speciesDirectory(const QString& identifier);

	/**
	 * @return The identifier of the current theme
	 **/
	QString identifier() const;

	static QValueList<QColor> defaultColors();

	/**
	 * Read the config files for all units available on this computer. The
	 * values are used by @ref loadNewUnit and @ref loadEntry - they don't
	 * have to load the config file theirselves.
	 **/
	bool readUnitConfigs(bool full = true);

	/**
	 * Save the game data to XML. The "game data" is all data that depends
	 * on how the game is being played. Therefore all UnitProperties are NOT
	 * saved (they depend on their index.unit files only), but the teamcolor
	 * or e.g. the upgrades of the UnitProperties are saved here.
	 **/
	bool saveGameDataAsXML(QDomElement& root) const;
	bool loadGameDataFromXML(const QDomElement& root);

protected:
	void insertUpgrade(UpgradeProperties* upgrade);

private:
	SpeciesThemePrivate* d;

	QString mThemePath;
	QColor mTeamColor;
};

#endif
