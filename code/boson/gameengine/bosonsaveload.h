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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONSAVELOAD_H
#define BOSONSAVELOAD_H


#define BOSON_SAVEGAME_END_COOKIE 1718


#include <qstring.h>
#include <qobject.h>
//Added by qt3to4:
#include <Q3CString>

class QDomDocument;
class QDomElement;
class QDataStream;
class KPlayer;

class Boson;
class BosonCanvas;
class BosonPlayField;
class Player;

template <class T, class T2> class QMap;

class BosonSaveLoadPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonSaveLoad : public QObject
{
	Q_OBJECT
public:
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


	bool loadKGameFromXML(const QMap<QString, QByteArray>& files);
	bool loadPlayersFromXML(const QMap<QString, QByteArray>& files);
	bool loadCanvasFromXML(const QMap<QString, QByteArray>& files);
	bool loadExternalFromXML(const QMap<QString, QByteArray>& files);
	bool loadEventListenerScripts(const QMap<QString, QByteArray>& files);
	bool loadEventListenersXML(const QMap<QString, QByteArray>& files);

signals:
	void signalLoadExternalStuffFromXML(const QDomElement& root);
	void signalSaveExternalStuffAsXML(QDomElement& root);

protected:
	friend class BosonStarting;
	// TODO: move relevant stuff to BosonStarting.

	Q3CString saveKGameAsXML();
	Q3CString savePlayersAsXML();
	Q3CString saveCanvasAsXML();
	Q3CString saveExternalAsXML();
	bool saveEventListenerScripts(QMap<QString, QByteArray>* files);
	bool saveEventListenersXML(QMap<QString, QByteArray>* files);

	bool loadVersionFromXML(const QString&); // takes kgame.xml

	bool convertSaveGameToPlayField(QMap<QString, QByteArray>& files);

private:
	void initBoson();

private:
	BosonSaveLoadPrivate* d;
};

#endif
