/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONDATA_H
#define BOSONDATA_H

#include <qstring.h>

#define boData BosonData::bosonData()

class QStringList;
class BosonGroundTheme;
class BosonPlayField;
class BosonCampaign;
template<class T> class BoGlobalObject;

class BosonDataObject
{
public:
	BosonDataObject(const QString& file)
	{
		mFile = file;
	}
	virtual ~BosonDataObject()
	{
	}

	virtual const QString& file() const
	{
		return mFile;
	}

	virtual QString idString() const = 0;

	/**
	 * @return A pointer to the object we are operating on. You should cast
	 * the returned pointer to the desired type.
	 **/
	virtual void* pointer() const = 0;

	virtual bool load() = 0;

private:
	QString mFile;
};

/**
 * A @ref BosonDataObject implementation that operates on void* pointers.
 **/
class BosonGenericDataObject
{
public:
	BosonGenericDataObject(const QString& file, const QString& id, void* pointer)
	{
		mFile = file;
		mId = id;
		mPointer = pointer;
	}
	virtual ~BosonGenericDataObject()
	{
	}

	virtual QString idString() const
	{
		return mId;
	}

	virtual void* pointer() const
	{
		return mPointer;
	}

	virtual bool load()
	{
		return true;
	}

private:
	QString mFile;
	QString mId;
	void* mPointer;
};

class BosonDataPrivate;
class BosonData
{
public:
	// note that we don't have a public c'tor
	~BosonData();

	/**
	 * @return BoGlobal::boGlobal->bosonData();
	 **/
	static BosonData* bosonData();

	void clearData();

	bool insertGroundTheme(BosonDataObject* object);
	bool loadGroundTheme(const QString& id);
	BosonGroundTheme* groundTheme(const QString& id) const;
	QStringList availableGroundThemes() const;

	bool insertPlayField(BosonDataObject* object);
	bool loadPlayField(const QString& id);
	BosonPlayField* playField(const QString& id) const;
	QStringList availablePlayFields() const;

	bool insertCampaign(BosonDataObject* object);
	BosonCampaign* campaign(const QString& id) const;
	QStringList availableCampaigns() const;

	/**
	 * @return A list of available files that match @p searchPattern. See
	 * @ref KStandardDirs::findAllResources for complete usage. The files
	 * have an absolute path.
	 *
	 * Note that the search always takes place in the group "data" and will
	 * prepend "boson/" to @p searchPattern.
	 *
	 * @param recursive If TRUE then all "*" in @p searchPattern will be
	 * extended to subdirectories as well. I.e. "boson/ * /file.png" (remove
	 * spaces - necessary because of c++ comments.. any way around this?)
	 * will _also_ expand to "boson/foo/bar/file.png", whereas with
	 * recursive=FALSE it would expand to "boson/foo2/file.png" _only_
	match
	 **/
	static QStringList availableFiles(const QString& searchPattern, bool recursive = false);

	/**
	 * Replacmenet for the usual locate("data", fileName);
	 *
	 * @return The absolute path for @p fileName, or @ref QString::null, if
	 * @p fileName can't be found.
	 **/
	QString locateDataFile(const QString& fileName) const;

protected:
	// AB: I am a bit surprised, that g++ accepts this, as BoGlobalObject is
	// a forwarded template. is this valid c++ ?
	friend class BoGlobalObject<BosonData>;
	// protected, so that noone can make an object except for ourselves.
	BosonData();

private:
	static BosonData* mBosonData;

private:
	BosonDataPrivate* d;
};

#endif
