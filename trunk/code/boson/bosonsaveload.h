/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONSAVELOAD_H
#define BOSONSAVELOAD_H

#define BOSON_MAKE_SAVEGAME_FORMAT_VERSION( a,b,c ) ( ((a) << 16) | ((b) << 8) | (c) )

// version from boson 0.8
#define BOSON_SAVEGAME_FORMAT_VERSION_0_8 \
	( BOSON_MAKE_SAVEGAME_FORMAT_VERSION (0x00, 0x01, 0x12) )

#define BOSON_SAVEGAME_END_COOKIE 1718


#include <qstring.h>
#include <qobject.h>

class QDomDocument;
class QDomElement;
class QDataStream;

class Boson;
class BosonCanvas;
class BosonPlayField;
class Player;
class KPlayer;

class BosonSaveLoadPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonSaveLoad : public QObject
{
	Q_OBJECT
public:
	/**
	 * Describes current status when loading saved game
	 * This is mostly used for error checking
	 *
	 * Possible values:
	 * @li NotLoaded - Game is not yet loaded (loading method isn't called yet)
	 * @li LoadingInProgress - Loading is in progress
	 * @li LoadingCompleted - Loading is completed (successfully)
	 * @li BSGFileError - Error in BSGFile. Most likely it wasn't Boson savegame
	 * @li InvalidXML - Error in one of XML files.
	 * @li InvalidFileFormat - File format was invalid (error)
	 * @li InvalidCookie - Cookie in the file header was invalid (error)
	 * @li InvalidVersion - SaveGame version was invalid. Probably the game was saved with too old version of Boson (error)
	 * @li KGameError - Error while loading KGame stuff
	 **/
	enum LoadingStatus {
		NotLoaded = 1,
		LoadingInProgress,
		LoadingCompleted,
		BSGFileError,
		InvalidXML,
		InvalidFileFormat,
		InvalidCookie,
		InvalidVersion,
		KGameError
	};

	BosonSaveLoad(Boson* parent);
	~BosonSaveLoad();

	void setCanvas(BosonCanvas*);
	void setPlayField(BosonPlayField*);

	/**
	 * You are meant to use @ref Boson::loadFromFile instead, so that @ref
	 * Boson::loadingStatus will be valid.
	 **/
	bool loadFromFile(const QString& file);

	/**
	 * Use Boson::saveToFile instead.
	 **/
	bool saveToFile(Player* localPlayer, const QString& file);

	LoadingStatus loadingStatus() const;

	/**
	 * Load the XML file in @p xml into @p doc and display an error message
	 * if an error occured.
	 * @return TRUE on success
	 **/
	bool loadXMLDoc(QDomDocument* doc, const QString& xml);

	/**
	 * @param kgameXML The kgame.xml file.
	 * @return The file format version of @p kgameXML or 0 if this is not a
	 * valid file.
	 **/
	unsigned long int savegameFormatVersion(const QString& kgameXML);

	/**
	 * @return Latest Boson savegame format version
	 **/
	static unsigned long int latestSavegameVersion();

signals:
	void signalLoadExternalStuff(QDataStream& stream);
	void signalSaveExternalStuff(QDataStream& stream);
	void signalLoadExternalStuffFromXML(const QDomElement& root);
	void signalSaveExternalStuffAsXML(QDomElement& root);

	void signalLoadingPlayersCount(int count);
	void signalLoadingPlayer(int current);
	void signalLoadingType(int type);

	/**
	 * Request for loading the data files of @p player
	 **/
	void signalLoadPlayerData(Player* player);

	void signalInitMap(const QByteArray& map);

protected:
	QString saveKGameAsXML();
	QString savePlayersAsXML(Player* localPlayer);
	QString saveCanvasAsXML();
	QString saveExternalAsXML();

	bool loadKGameFromXML(const QString&);
	bool loadPlayersFromXML(const QString&);
	bool loadCanvasFromXML(const QString&);
	bool loadExternalFromXML(const QString&);

private:
	void initBoson();

	/**
	 * KGame::systemAddPlayer is protected, this is used as a workaround.
	 * Not nice, but working.
	 **/
	void systemAddPlayer(KPlayer*);

private:
	BosonSaveLoadPrivate* d;
};

#endif
