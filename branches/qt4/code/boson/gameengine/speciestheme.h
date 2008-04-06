/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2008 Andreas Beckermann (b_mann@gmx.de)
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
//Added by qt3to4:
#include <QPixmap>
#include <Q3CString>
#include <Q3ValueList>

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
template<class T> class Q3ValueList;
template<class T> class Q3IntDict;

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
	 * Construct a theme.
	 *
	 * The theme must be set up (see e.g. @ref setThemePath and @ref
	 * setTeamColor) and loaded (see e.g. @ref loadTechnologies, @ref
	 * readUnitConfigs) before it can be used.
	 **/
	SpeciesTheme();

	~SpeciesTheme();


	/**
	 * Set the directory where the theme searches for additional files,
	 * such as unit configs (see @ref readUnitConfigs), pixmaps,
	 * technologies (see @ref loadTechnologies) and so on.
	 *
	 * This should be called exactly once after construction of the theme.
	 **/
	void setThemePath(const QString& dir);

	/**
	 * @return the path to the species theme, see @ref setThemePath. The
	 * themepath is usually ending with boson/themes/species/your_species/
	 * and is guaranteed to end with a "/".
	 **/
	const QString& themePath() const { return mThemePath; }

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
	 *
	 * @param color The desired color, or an invalid color (see @ref
	 * QColor::isValid) to use a default color (see @ref defaultColor).
	 *
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
	 * Read the config files for all units available on this computer. The
	 * values are used by @ref loadNewUnit and @ref loadEntry - they don't
	 * have to load the config file theirselves.
	 **/
	bool readUnitConfigs();

	/**
	 * @return Concatenation of all @ref UnitProperties::md5 sums in this
	 * theme, separated by "\n"s
	 **/
	Q3CString unitPropertiesMD5() const;

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
	 * @return The typeIds of all facilities in this theme. See also @ref
	 * UnitProperties::typeId
	 **/
	Q3ValueList<unsigned long int> allFacilities() const;

	/**
	 * @return The typeIds of all mobile units in this theme. See also @ref
	 * UnitProperties::typeId
	 **/
	Q3ValueList<unsigned long int> allMobiles() const;

	/**
	 * @return A list of all unit properties in this theme
	 **/
	Q3ValueList<const UnitProperties*> allUnits() const;

	const Q3IntDict<UnitProperties>* allUnitsNonConst() const;

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
	Q3ValueList<unsigned long int> productions(const Q3ValueList<unsigned long int>& producerList) const;

	/**
	 * @return A list of all technologies that have a producer specified in
	 * producerList.
	 **/
	Q3ValueList<unsigned long int> technologies(const Q3ValueList<unsigned long int>& producerList) const;

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

	static Q3ValueList<QColor> defaultColors();

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
