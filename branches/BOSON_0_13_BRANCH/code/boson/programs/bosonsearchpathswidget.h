/*
    This file is part of the Boson game
    Copyright (C) 2002 Rivo Laks (rivolaks@hot.ee)

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

#ifndef BOSONSEARCHPATHSWIDGET_H
#define BOSONSEARCHPATHSWIDGET_H

#include "bosonsearchpathswidgetbase.h"

#include "bofiledialog.h"

#include <klocale.h>
#include <kdebug.h>

#include <qdir.h>

class BosonSearchPathsWidget : public BosonSearchPathsWidgetBase
{
	Q_OBJECT
public:
	BosonSearchPathsWidget(QWidget* parent);

public slots:
	void slotAppendPath(QString path);
	void slotSetPaths(QStringList paths);
	void slotRemovePath();
	void slotAddPath();
	void slotBrowse();
	void slotPathSelected(int index);
	void slotCurrentPathChanged(const QString& path);

public:
	QStringList currentPaths() const;

private:
	void init();

private:
	int mCurrentPath;
};

#endif

