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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOEDITTURRETPROPERTIESDIALOG_H
#define BOEDITTURRETPROPERTIESDIALOG_H

#include <kdialogbase.h>
#include <qlistview.h>

#include <lib3ds/types.h>

class QStringList;
class QListViewItem;

class BoEditTurretPropertiesDialogPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoEditTurretPropertiesDialog : public KDialogBase
{
	Q_OBJECT
public:
	BoEditTurretPropertiesDialog(QWidget* parent, bool modal = false);
	~BoEditTurretPropertiesDialog();

	void setModelFile(const QString& file);
	void setTurretMeshes(const QStringList& list);
	QStringList turretMeshes() const;
	float initialZRotation() const;

signals:
	void signalApply(BoEditTurretPropertiesDialog*);

protected slots:
	void slotLineEditChanged();
	void slotItemChanged();
	virtual void slotApply();

protected:
	void addMesh(Lib3dsNode* node, QListViewItem* parent);
	void updateListView();

protected slots:

private:
	BoEditTurretPropertiesDialogPrivate* d;
};

class BoCheckListView : public QListView
{
	Q_OBJECT
public:
	BoCheckListView(QWidget* parent)
		: QListView(parent)
	{
	}

	void notifyChange();

signals:
	void signalChanged();
};

#endif
