/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BPFDESCRIPTION_H
#define BPFDESCRIPTION_H

#include <qdom.h>

class BoPlayFieldFile;
class KArchiveDirectory;
class KTar;
class KArchiveFile;
class QDomDocument;
class QDomElement;
class QDataStream;

/**
 * A BosonPlayField description. This class represents the description.xml file
 * from a .bpf file. Currently there are mainly the @ref name and a @ref comment
 * for the map in this file.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BPFDescription
{
public:
	/**
	 * Create a new description.xml file
	 **/
	BPFDescription();
	BPFDescription(const BPFDescription& data);

	/**
	 * Load a description.xml file
	 **/
	explicit BPFDescription(const QString& data);

	~BPFDescription();

	BPFDescription& operator=(const BPFDescription& data);

	QString name() const
	{
		return textOfElement(QString::fromLatin1("Name"));
	}
	QString comment() const
	{
		return textOfElement(QString::fromLatin1("Comment"));
	}

	void setName(const QString& name)
	{
		setTextForElement(QString::fromLatin1("Name"), name);
	}
	void setComment(const QString& comment)
	{
		setTextForElement(QString::fromLatin1("Comment"), comment);
	}

	QString toString() const { return mFile.toString(); }

protected:
	QDomElement element(const QString& name) const;
	QDomElement topElement() const
	{
		return mFile.documentElement();
	}
	/**
	 * A <em>very</em> simple convenience function. This simply takes the
	 * element from @ref topElement that fits the specified name (an warning
	 * occurs if there are multiple elements with this name!) and assignes
	 * the specified text. The element is created if not yet existant.
	 *
	 * Be careful when you are using this - it will revert everything that
	 * doesn't look like expected (e.g. multiple child nodes where only 1 is
	 * expected)
	 **/
	void setTextForElement(const QString& tagName, const QString& text);
	QString textOfElement(const QString& tagName) const;

private:
	QDomDocument mFile;
};



#endif
