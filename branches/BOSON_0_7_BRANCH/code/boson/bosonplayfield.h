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
#ifndef BOSONPLAYFIELD_H
#define BOSONPLAYFIELD_H

#include <qobject.h>

class BosonMap;
class BosonScenario;
class Boson;

class QDomDocument;
class QDomElement;
class QDataStream;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonPlayField : public QObject
{
	Q_OBJECT
public:
	BosonPlayField(QObject* parent = 0);
	~BosonPlayField();

	bool loadPlayField(const QString& fileName);

	/**
	 * Remember to call @ref applyScenario first! The @ref
	 * BosonMap is updated automatically whenever a cell is added, but the
	 * unit list is not.
	 **/
	bool savePlayField(const QString& fileName);

	void applyScenario(Boson* boson);

	static QString defaultPlayField();
	static QStringList availablePlayFields();

	/**
	 * Load a @ref BosonMap from stream. Create a new BosonMap object if it
	 * does not yet exist, otherwise load from stream but also compare the
	 * stream with our locally present @ref BosonMap. Note that there must
	 * be no difference as we should have been the one who sent the stream!!
	 *
	 * But if there is a difference by any reason the network stream will be
	 * used instead of our locally present one.
	 *
	 * @return FALSE if the format was invalid. Note that the map will be
	 * broken then!
	 **/
	bool loadMap(QDataStream& stream);

	/**
	 * This simply deletes the map. This may be useful for game starting, as
	 * the ADMIN already has the map, but all other players don't. So the
	 * ADMIN might send the map over network and delete his local copy, so
	 * that it is ensured the entire loading code is the same on all
	 * clients, i.e. without a map until it is received.
	 **/
	void deleteMap();
	
	void saveMap(QDataStream& stream);

	BosonMap* map() const { return mMap; }
	BosonScenario* scenario() const { return mScenario; }

	void quit();

	static QString playFieldFileName(const QString& identifier);

	bool modified() const;

	/**
	 * Make @p map the new map of this playfield. Called by the editor
	 * only.
	 **/
	void changeMap(BosonMap* map);

	/**
	 * Make @p s the new scenario of this playfield. Called by the editor
	 * only.
	 **/
	void changeScenario(BosonScenario* s);

signals:
	/**
	 * Emitted when the map changes. Note that this can even be 0!
	 **/
	void signalNewMap(BosonMap*);

protected:
	bool loadMap(QDomElement& node);
	bool loadScenario(QDomElement& node);

private:
	BosonMap* mMap;
	BosonScenario* mScenario;

	class BosonPlayFieldPrivate;
	BosonPlayFieldPrivate* d;
};

#endif
