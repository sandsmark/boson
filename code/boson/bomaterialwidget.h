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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef BOMATERIALWIDGET_H
#define BOMATERIALWIDGET_H

#include <qwidget.h>

class BoMaterial;

class BoMaterialWidgetPrivate;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMaterialWidget : public QWidget
{
	Q_OBJECT
public:
	BoMaterialWidget(QWidget* parent = 0, const char* name = 0);
	~BoMaterialWidget();

	void addMaterial(BoMaterial* mat);
	void clearMaterials();

protected slots:
	void slotActiveMaterialChanged(int);
	void slotUpdateMaterial();

private:
	BoMaterialWidgetPrivate* d;
};

#endif

