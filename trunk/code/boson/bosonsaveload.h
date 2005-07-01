/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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


#define BOSON_SAVEGAME_END_COOKIE 1718


#include <qstring.h>
#include <qobject.h>

class QDomDocument;
class QDomElement;
class QDataStream;
class KPlayer;

class Boson;
class BosonCanvas;
class BosonPlayField;
class Player;

template <class T, class T2> class QMap;

class SaveLoadError
{
public:
	enum ErrorType {
		Unknown = 0,
		General = 1,
		LoadBSGFileError = 2,
		LoadInvalidXML = 3,
		LoadPlayersError = 4,
		LoadInvalidVersion = 5,
		InvalidFileFormat = 6
	};

	/**
	 * @param text An additional text describing the error. There are
	 * predefined messages available in this class, depending on the @p
	 * type. When you provide a @p text it will be displayed as additional
	 * information.
	 **/
	SaveLoadError(ErrorType type, const QString& text = QString::null, const QString& caption = QString::null);

	const QString& caption() const
	{
		return mCaption;
	}
	/**
	 * @return An error message. This message depends on the @ref type, but
	 * you can provide additional information in the constructor. Note that
	 * this message can be several lines long.
	 **/
	QString message() const;

	ErrorType type() const
	{
		return mType;
	}

private:
	ErrorType mType;
	QString mCaption;
	QString mText;
};

class LoadError : public SaveLoadError
{
public:
	/**
	 * @param text An additional text describing the error. There are
	 * predefined messages available in this class, depending on the @p
	 * type. When you provide a @p text it will be displayed as additional
	 * information.
	 * @param caption You will probably never need to touch this.
	 **/
	LoadError(ErrorType type, const QString& text = QString::null, const QString& caption = QString::null);
};

class SaveError : public SaveLoadError
{
public:
	/**
	 * @param text An additional text describing the error. There are
	 * predefined messages available in this class, depending on the @p
	 * type. When you provide a @p text it will be displayed as additional
	 * information.
	 * @param caption You will probably never need to touch this.
	 **/
	SaveError(ErrorType type, const QString& text = QString::null, const QString& caption = QString::null);
};

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
	 * Save the game to @p files.
	 **/
	bool saveToFiles(QMap<QString, QByteArray>& files);

	/**
	 * Just like @ref saveToFiles, but this takes care of converting the
	 * saved data into the correct format for a playfield file. This data
	 * can be used to start <em>new</em> games, as opposed to loading saved
	 * games.
	 **/
	bool savePlayFieldToFiles(QMap<QString, QByteArray>& files);

	/**
	 * Save the data from @p files to the @p file. Use @ref saveToFiles or
	 * @ref savePlayFieldToFiles to actually save the data.
	 **/
	static bool saveToFile(const QMap<QString, QByteArray>& files, const QString& file);

	/**
	 * Start a game from a @p files QMap. The @p files should come from a
	 * file that was saved using @ref saveToFile
	 *
	 * Note that the actual file and data loading all happens in @ref
	 * BosonStarting. This just starts the actual game, i.e. makes sure that
	 * items are added and so on. The playfield has been loaded already,
	 * too.
	 **/
	bool startFromFiles(const QMap<QString, QByteArray>& files);

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

protected:
	friend class BosonStarting;
	// TODO: move relevant stuff to BosonStarting.

	QCString saveKGameAsXML();
	QCString savePlayersAsXML();
	QCString saveCanvasAsXML();
	QCString saveExternalAsXML();
	bool saveEventListenerScripts(QMap<QString, QByteArray>* files);

	bool loadVersionFromXML(const QString&); // takes kgame.xml
	bool loadKGameFromXML(const QString&);
	bool loadPlayersFromXML(const QString&);
	bool loadCanvasFromXML(const QString&);
	bool loadExternalFromXML(const QString&);
	bool loadEventListenerScripts(const QMap<QString, QByteArray>& files);

	bool convertSaveGameToPlayField(QMap<QString, QByteArray>& files);

	/**
	 * Add an error message to the error queue.
	 **/
	void addError(SaveLoadError* error);
	void addSaveError(SaveLoadError::ErrorType, const QString& text = QString::null, const QString& caption = QString::null);
	void addLoadError(SaveLoadError::ErrorType, const QString& text = QString::null, const QString& caption = QString::null);

	/**
	 * This fixes player ids of filenames for saving.
	 *
	 * Other classes (e.g. the @ref BoEventManager) save their files to @p
	 * files and sometimes use the playerid in the filenames (e.g.
	 * ai-player_id.py). This method reads those filenames, removes them
	 * from @p files and re-adds them with the player id replaced by the
	 * index of the player in the game list (which is used in boson .bpf and
	 * .bsg files).
	 *
	 * Only files with a "-player_id" substring, where id is a number, are
	 * touched. That substring is replaced by "-player_n", where n is the
	 * index of the player.
	 *
	 * @return FALSE on error, otherwise TRUE.
	 **/
	bool convertPlayerIdsToIndices(QMap<QString, QByteArray>& files) const;

private:
	void initBoson();

private:
	BosonSaveLoadPrivate* d;
};

#endif
