/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOCONDITIONEDITORMAIN_H
#define BOCONDITIONEDITORMAIN_H

#include <qwidget.h>
#include <qmap.h>
#include <qvaluelist.h>

class QLabel;
class QPushButton;
class QListBox;
class KTar;
class KArchiveFile;
class KArchiveDirectory;
class QDomElement;
class QDomDocument;
class QListBoxItem;

class BoConditionEditorMain : public QWidget
{
	Q_OBJECT
public:
	BoConditionEditorMain();
	~BoConditionEditorMain();

public slots:
	void slotLoadFile(const QString&);
	void slotSaveFile(const QString&);

protected slots:
	void slotSelectFile();
	void slotSelectSaveFile();
	void slotEditConditions();

protected:
	void reset();
	bool loadXMLFile(const KArchiveFile*);
	bool parsePlayerIds(const KArchiveFile*);
	bool saveFile(KTar* save, const QString& path, const KArchiveDirectory* from);

private:
	QLabel* mFileName;
	QPushButton* mSelectFile;
	QPushButton* mSelectSaveFile;
	QPushButton* mEditConditions;
	QListBox* mConditions;

	KTar* mFile;
	QMap<QListBoxItem*, QDomElement> mItem2Element;
	QMap<QListBoxItem*, QWidget*> mItem2Widget;
	QMap<const KArchiveFile*, QDomDocument> mFile2XML;
	QMap<const KArchiveFile*, QListBoxItem*> mFile2Item;
	QValueList<unsigned long int> mPlayerIds;
};

#endif

