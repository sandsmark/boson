/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef __SPECIESTHEME_H__
#define __SPECIESTHEME_H__

#include <qstring.h>
#include <qcolor.h>
#include <qvaluelist.h>

class QPixmap;
class QCanvasPixmapArray;
class QStringList;
class UnitProperties;
class UnitBase;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class SpeciesTheme
{
public:
	/**
	 * Construct and load a theme. See @ref loadTheme.
	 **/
	SpeciesTheme(const QString& species, QRgb teamColor = qRgb(0,0,0));
	
	~SpeciesTheme();

	/**
	 * Load a theme.
	 * @param species The theme name to be loaded. Must not be i18n'ed as it
	 * is used in the directoryname.
	 * @param teamColor The color of this team or QRgb(0,0,0) for a default
	 * color
	 **/
	bool loadTheme(const QString& species, QRgb teamColor = qRgb(0,0,0));

	/**
	 * Load the unit unitType. This must be done before @ref pixmapArray,
	 * @ref bigOverview or @ref smallOverview can return something useful.
	 * These functions call this automatically so you usually don't need to
	 * bother about it. @ref unitProperties does not need to load the unit.
	 *
	 * You can use this to preload units.
	 **/
	bool loadUnit(int unitType);

	/***
	 * @return The pixmap array for unitType or NULL if none was found for
	 * this unitType. See also @ref UnitProperties::typeId
	 **/
	QCanvasPixmapArray* pixmapArray(int unitType);

	/**
	 * Make sure to call @ref loadShot before!
	 * @return The animation of a normal shot.
	 **/
	QCanvasPixmapArray* shot() const;
	
	/**
	 * Make sure to call @ref loadBigShot before!
	 **/
	QCanvasPixmapArray* bigShot(bool isFacility, unsigned int version) const;

	/***
	 * @return The big overview pixmap (the one that is displayed when the
	 * unit is selected) for unitType or NULL if none was found for
	 * this unitType. See also @ref UnitProperties::typeId
	 **/
	QPixmap* bigOverview(int unitType);

	/***
	 * @return The small pixmap (the one that is displayed on the order
	 * buttons in @ref BosonCommandFrame to construct this unit) for 
	 * unitType or NULL if none was found for this unitType. See also 
	 * @ref UnitProperties::typeId
	 **/
	QPixmap* smallOverview(int unitType);

	/**
	 * @return The color of the team of this player. Can only be changed
	 * on construction!
	 **/
	QRgb teamColor() const;

	/**
	 * @return A default color. This color differs after every call.
	 **/
	static QRgb defaultColor();

	/**
	 * Reads the default entries from the config file of the specified unit
	 * and applies them. If values are not available in the file the
	 * hardcoded defaults are used. 
	 *
	 * Use this if you actually want to create a new unit (e.g. produce
	 * one), don't use it if values will differ from the defaults (although
	 * it wouldn't hurt if you would still use this)
	 **/
	void loadNewUnit(UnitBase* unit);

	const UnitProperties* unitProperties(UnitBase* unit) const;
	const UnitProperties* unitProperties(int unitType) const;

	/**
	 * @return the path to the species theme (ending with
	 * boson/themes/species/your_species/)
	 **/
	const QString& themePath() const { return mThemePath; }

	/**
	 * @return The typeIds of all facilities in this theme. See also @ref
	 * UnitProperties::typeId
	 **/
	QValueList<int> allFacilities() const;

	/**
	 * @return The typeIds of all mobile units in this theme. See also @ref
	 * UnitProperties::typeId
	 **/
	QValueList<int> allMobiles() const;

	/**
	 * Reset this theme. Delete all pixmaps, unitProperties, ...
	 **/
	void reset();

	/**
	 * Load the shot "animation"
	 **/
	bool loadShot();
	bool loadBigShot(bool isFacility, unsigned int version);

	/**
	 * @return A list of all possible species. Note that the list contains
	 * the index.dektop files - so remove index.desktop from every entry to
	 * get the actual species directory.
	 **/
	static QStringList availableSpecies();

	/**
	 * @return The directory of the default species ("human")
	 **/
	static QString defaultSpecies();

	static QString speciesDirectory(const QString& identifier);
	

protected:
	/**
	 * Load a pixmap from path with mash (or not). This is used for all unit
	 * pixmaps: small/big overview and sprites. It is <em>not</em> yet used
	 * fot the shot sprites. Use @ref loadShotPixmap for these.
	 *
	 * @param fileName The path to load the pixmap from. Should be an abolute
	 * filename.
	 * @param pix The pixmap that is loaded. 
	 **/
	bool loadUnitPixmap(const QString& fileName, QPixmap &pix, bool withMask = true, bool withTeamColor = true);

	/**
	 * Used for the shot sprites by @ref loadShot.
	 **/
	bool loadShotPixmap(const QString& fileName, QPixmap& pix);

	/**
	 * Read the config files for all units available on this computer. The
	 * values are used by @ref loadNewUnit and @ref loadEntry - they don't
	 * have to load the config file theirselves.
	 **/
	void readUnitConfigs();

private:
	class SpeciesThemePrivate;
	SpeciesThemePrivate* d;

	QString mThemePath;
};

#endif
