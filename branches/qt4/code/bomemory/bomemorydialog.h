/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOMEMORYDIALOG_H
#define BOMEMORYDIALOG_H

#include <kdialogbase.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>

class Q3ListViewItem;
class MyMemNode;
template<class T> class Q3ValueList;

class BoMemoryDialogPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMemoryDialog : public KDialogBase
{
	Q_OBJECT
public:
	BoMemoryDialog(QWidget* parent, bool modal = false);
	~BoMemoryDialog();

public slots:
	void slotUpdate();

protected:
	Q3ListViewItem* createFileItem(const QString& file) const;
	Q3ListViewItem* createFunctionItem(Q3ListViewItem* parent, const QString& function) const;
	Q3ListViewItem* createMemoryItem(Q3ListViewItem* parent, const MyMemNode* node) const;

	/**
	 * Create the QListViewItem list of @p file, for all items in @p list.
	 * @param list A pointer to the list of @ref MyMemNode objects that are in
	 * @p file.
	 * @return The number of bytes that are in this file
	 **/
	unsigned long int createFileList(Q3ListViewItem* file, const Q3PtrList<MyMemNode>* list);
	unsigned long int createFunctionList(Q3ListViewItem* line, const Q3PtrList<MyMemNode>* list);

	void setSize(Q3ListViewItem* item, unsigned long int bytes) const;

private:
	BoMemoryDialogPrivate* d;
};

#endif
