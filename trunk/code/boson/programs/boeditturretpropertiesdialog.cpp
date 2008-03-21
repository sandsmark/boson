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
#include "boeditturretpropertiesdialog.h"
#include "boeditturretpropertiesdialog.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"

#include <klocale.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qstringlist.h>

#include <lib3ds/file.h>
#include <lib3ds/node.h>

void BoCheckListView::notifyChange()
{
 emit signalChanged();
}

class BoCheckListItem : public QCheckListItem
{
public:
	BoCheckListItem(QListView* parent, const QString& text)
		: QCheckListItem(parent, text, QCheckListItem::CheckBox)
	{
	}

	BoCheckListItem(QListViewItem* parent, const QString& text)
		: QCheckListItem(parent, text, QCheckListItem::CheckBox)
	{
	}

protected:
	virtual void stateChange(bool s)
	{
		QCheckListItem::stateChange(s);
		static bool recursive = false;
		bool wasRecursive = recursive;
		recursive = true;

		for (QListViewItem* n = firstChild(); n; n = n->nextSibling()) {
			if (n->rtti() != 1) {
				continue;
			}
			QCheckListItem* item = (QCheckListItem*)n;
			item->setOn(s);
		}

		if (!wasRecursive) {
			((BoCheckListView*)listView())->notifyChange();
		}

		recursive = false;
	}
};

class BoEditTurretPropertiesDialogPrivate
{
public:
	BoEditTurretPropertiesDialogPrivate()
	{
		mInitialZRotation = 0;
		mTurretMeshes = 0;
		mTurretMeshesListView = 0;
	}
	QLineEdit* mTurretMeshes;
	BoCheckListView* mTurretMeshesListView;
	KIntNumInput* mInitialZRotation;
};

BoEditTurretPropertiesDialog::BoEditTurretPropertiesDialog(QWidget* parent, bool modal)
		: KDialogBase(Plain, i18n("Edit Turrets"), Ok | Cancel | Apply,
		Ok, parent, "boeditturretpropertiesdialog", modal, true)
{
 d = new BoEditTurretPropertiesDialogPrivate();

 QVBoxLayout* layout = new QVBoxLayout(plainPage());
 d->mInitialZRotation = new KIntNumInput(plainPage());
 d->mInitialZRotation->setRange(0, 360, 1, true);
 d->mInitialZRotation->setLabel(i18n("Initial Z rotation"));
 layout->addWidget(d->mInitialZRotation);
 QLabel* meshesLabel = new QLabel(i18n("Turret Meshes:"), plainPage());
 d->mTurretMeshes = new QLineEdit(plainPage());
 connect(d->mTurretMeshes, SIGNAL(textChanged(const QString&)),
		this, SLOT(slotLineEditChanged()));
 layout->addWidget(meshesLabel);
 layout->addWidget(d->mTurretMeshes);

 d->mTurretMeshesListView = new BoCheckListView(plainPage());
 d->mTurretMeshesListView->setRootIsDecorated(true);
 d->mTurretMeshesListView->addColumn("foo1");
 d->mTurretMeshesListView->addColumn("foo2");
 connect(d->mTurretMeshesListView, SIGNAL(signalChanged()),
		this, SLOT(slotItemChanged()));
 layout->addWidget(d->mTurretMeshesListView);
}

BoEditTurretPropertiesDialog::~BoEditTurretPropertiesDialog()
{
 delete d;
}

void BoEditTurretPropertiesDialog::setModelFile(const QString& fileName)
{
 boDebug() << k_funcinfo << endl;
 d->mTurretMeshesListView->setEnabled(false);
 d->mTurretMeshesListView->hide();
 Lib3dsFile* file = lib3ds_file_load(fileName);
 if (!file) {
	boDebug() << k_funcinfo << "could not load model file " << fileName << endl;
	return;
 }
 for (Lib3dsNode* n = file->nodes; n; n = n->next) {
	addMesh(n, 0);
 }

 lib3ds_file_free(file);
 d->mTurretMeshesListView->setEnabled(true);
 d->mTurretMeshesListView->show();
 boDebug() << k_funcinfo << "done" << endl;
}

void BoEditTurretPropertiesDialog::setTurretMeshes(const QStringList& list)
{
 d->mTurretMeshes->setText(list.join(","));
}

QStringList BoEditTurretPropertiesDialog::turretMeshes() const
{
 return QStringList::split(',', d->mTurretMeshes->text());
}

float BoEditTurretPropertiesDialog::initialZRotation() const
{
 return (float)d->mInitialZRotation->value();
}

void BoEditTurretPropertiesDialog::addMesh(Lib3dsNode* node, QListViewItem* parent)
{
 BO_CHECK_NULL_RET(node);
 QCheckListItem* item = 0;
 if (parent) {
	item = new BoCheckListItem(parent, QString(node->name));
 } else {
	item = new BoCheckListItem(d->mTurretMeshesListView, QString(node->name));
 }
 item->setOn(false);
 item->setOpen(true);

 for (Lib3dsNode* n = node->childs; n; n = n->next) {
	addMesh(n, item);
 }
}

void BoEditTurretPropertiesDialog::updateListView()
{
 QStringList meshes = turretMeshes();
 QPtrList<QListViewItem> items;
 for (QListViewItem* item = d->mTurretMeshesListView->firstChild(); item; item = item->nextSibling()) {
	items.append(item);
 }
 d->mTurretMeshesListView->blockSignals(true);
 for (QPtrListIterator<QListViewItem> it(items); it.current(); ++it) {
	for (QListViewItem* item = it.current()->firstChild(); item; item = item->nextSibling()) {
		items.append(item);
	}

	if (it.current()->rtti() != 1) {
		continue;
	}
	QCheckListItem* item = (QCheckListItem*)it.current();

	if (meshes.contains(item->text())) {
		item->setOn(true);
	} else {
		item->setOn(false);
	}
 }
 d->mTurretMeshesListView->blockSignals(false);
}

void BoEditTurretPropertiesDialog::slotLineEditChanged()
{
 boDebug() << k_funcinfo << endl;
 updateListView();
}

void BoEditTurretPropertiesDialog::slotItemChanged()
{
 QStringList meshes;
 QPtrList<QListViewItem> items;
 for (QListViewItem* item = d->mTurretMeshesListView->firstChild(); item; item = item->nextSibling()) {
	items.append(item);
 }
 for (QPtrListIterator<QListViewItem> it(items); it.current(); ++it) {
	for (QListViewItem* item = it.current()->firstChild(); item; item = item->nextSibling()) {
		items.append(item);
	}

	if (it.current()->rtti() != 1) {
		continue;
	}
	QCheckListItem* item = (QCheckListItem*)it.current();
	if (item->isOn()) {
		meshes.append(item->text());
	}
 }

 // remove duplicates
 meshes.sort();
 QStringList list;
 QString previous;
 for (QStringList::iterator it = meshes.begin(); it != meshes.end(); ++it) {
	if ((*it) == previous) {
		continue;
	}
	list.append(*it);
	previous = *it;
 }

 d->mTurretMeshes->setText(list.join(","));
}

void BoEditTurretPropertiesDialog::slotApply()
{
 emit signalApply(this);
}



