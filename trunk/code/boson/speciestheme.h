/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef SPECIESTHEME_H
#define SPECIESTHEME_H

#include "defines.h"
#include "global.h"
#include <qstring.h>
#include <qcolor.h>
#include <qvaluelist.h>

#include <GL/gl.h>

class BosonTextureArray;
class BosonModel;
class UnitProperties;
class UnitBase;

class QPixmap;
class QStringList;
class QColor;

/**
 * Stores player's species - this includes units' sprites (if you're using
 * QCanvas) or textures and models (if you're using OpenGL), sounds, properties
 * and overviews as well as action pixmaps (attack, move and stop) and player's
 * teamcolor.
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
	 * Construct and load a theme. See @ref loadTheme.
	 *
	 * Constructing is quite fast, as @ref loadTheme does no preloading of
	 * unit pixmaps.
	 **/
	SpeciesTheme(const QString& species, const QColor& color);
	
	~SpeciesTheme();

	/**
	 * Load a theme. Look for index.desktop files in the units directory,
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
	 * Load the unit unitType. This must be done before @ref pixmapArray,
	 * @ref bigOverview or @ref smallOverview can return something useful.
	 * These functions call this automatically so you usually don't need to
	 * bother about it. @ref unitProperties does not need to load the unit.
	 *
	 * You can use this to preload units.
	 **/
	bool loadUnit(unsigned long int unitType);

	/**
	 * Load pixmaps of available actions (attack, move ...). This must be
	 * done before @ref actionPixmap can return anything useful
	 **/
	bool loadActionGraphics();

	/**
	 * @return Pixmap for the specified action
	 **/
	QPixmap* actionPixmap(UnitAction action);

	int unitWidth(unsigned long int unitType);
	int unitHeight(unsigned long int unitType);

	BosonTextureArray* textureArray(unsigned long int unitType);
	GLuint textureNumber(unsigned long int unitType, int direction);
	// TODO an OpenGL implementation for shot()

	GLuint displayList(unsigned long int unitType);
	BosonModel* unitModel(unsigned long int unitType);

	/**
	 * @return The big overview pixmap (the one that is displayed when the
	 * unit is selected) for unitType or NULL if none was found for
	 * this unitType. See also @ref UnitProperties::typeId
	 **/
	QPixmap* bigOverview(unsigned long int unitType);

	/**
	 * @return The small pixmap (the one that is displayed on the order
	 * buttons in @ref BosonCommandFrame to construct this unit) for 
	 * unitType or NULL if none was found for this unitType. See also 
	 * @ref UnitProperties::typeId
	 **/
	QPixmap* smallOverview(unsigned long int unitType);

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
	const UnitProperties* unitProperties(unsigned long int unitType) const;

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
	 * @return A list of all units that have a @ref UnitProperties::producer
	 * specified in producerList.
	 **/
	QValueList<unsigned long int> productions(QValueList<int> producerList) const;

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

	/**
	 * @return The identifier of the current theme
	 **/
	QString identifier() const;
	
	static QValueList<QColor> defaultColors();

protected:
	bool loadUnitGraphics(const UnitProperties* prop);

	/**
	 * Load a pixmap from path with mash (or not). This is used for all unit
	 * pixmaps: small/big overview and sprites. It is <em>not</em> yet used
	 * fot the shot sprites. Use @ref loadShotPixmap for these.
	 *
	 * @param fileName The path to load the pixmap from. Should be an abolute
	 * filename.
	 * @param pix The pixmap that is loaded. 
	 **/
	bool loadUnitImage(const QString& fileName, QImage &image, bool withMask = true, bool withTeamColor = true);
//	bool loadUnitPixmap(const QString& fileName, QPixmap &pix, bool withMask = true, bool withTeamColor = true);

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

	void loadUnitTextures(unsigned long int type, QValueList<QImage> list);
	void loadUnitModel(const UnitProperties* prop);
	GLuint createDisplayList(unsigned long int typeId);

private:
	class SpeciesThemePrivate;
	SpeciesThemePrivate* d;

	QString mThemePath;
	QColor mTeamColor;
};

#endif
