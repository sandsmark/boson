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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOPROPERTIESWIDGET_H
#define BOPROPERTIESWIDGET_H

#include "bodebugdcopiface.h"

#include <qwidget.h>
#include <qdom.h>

class QListViewItem;
class QListView;
class QLabel;

class BoPropertiesWidget : public QWidget
{
	Q_OBJECT
public:
	BoPropertiesWidget(QWidget* parent, const char* name = 0);
	~BoPropertiesWidget();

	void displayProperties(const QDomElement& root);

signals:
	void signalChanged(const QDomElement&);

protected:
	void setClassLabel(const QString& text);
	void createProperties(const QDomElement& root);

protected slots:
	void slotItemRenamed(QListViewItem* item, int col);

private:
	QLabel* mClassLabel;
	QListView* mListView;
	QDomElement mWidgetElement;
};

#endif

