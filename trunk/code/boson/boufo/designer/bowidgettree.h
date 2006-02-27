/*
    This file is part of the Boson game
    Copyright (C) 2004-2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOWIDGETTREE_H
#define BOWIDGETTREE_H

#include <qwidget.h>
#include <qmap.h>

class QListView;
class QListViewItem;
class QPushButton;
class QDomElement;

// displays the current form, as it is in the internal xml file
class BoWidgetTree : public QWidget
{
	Q_OBJECT
public:
	BoWidgetTree(QWidget* parent, const char* name = 0);
	~BoWidgetTree();

	void updateGUI(const QDomElement& root);

	void selectWidget(const QDomElement& widget);

protected:
	bool isContainer(QListViewItem* item) const;
	bool isContainer(const QDomElement&) const;

signals:
	void signalWidgetSelected(const QDomElement& widget);
	void signalRemoveWidget(const QDomElement& widget);
	void signalInsertWidget(const QDomElement& parent);
	void signalHierarchyChanged();

protected:
	void updateGUI(const QDomElement& root, QListViewItem* item);
	void moveElement(QListViewItem* widget, QListViewItem* parent, QListViewItem* before);

protected slots:
	void slotSelectionChanged(QListViewItem*);
	void slotInsert();
	void slotRemove();
	void slotMoveUp();
	void slotMoveDown();

private:
	QListView* mListView;
	QPushButton* mInsertWidget;
	QPushButton* mRemoveWidget;
	QPushButton* mMoveUp;
	QPushButton* mMoveDown;

	QMap<QListViewItem*, QDomElement> mItem2Element;
};

#endif

