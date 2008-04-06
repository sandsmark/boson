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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosonsearchpathswidget.h"
#include "bosonsearchpathswidget.moc"

#include "bofiledialog.h"
#include "bodebug.h"

#include <klocale.h>
#include <kdebug.h>

#include <qdir.h>
#include <qlineedit.h>
#include <q3listbox.h>

BosonSearchPathsWidget::BosonSearchPathsWidget(QWidget* parent)
	: BosonSearchPathsWidgetBase(parent)
{
 init();
}

void BosonSearchPathsWidget::slotAppendPath(QString path)
{
 mCurrentPaths->insertItem(path);
}

void BosonSearchPathsWidget::slotSetPaths( QStringList paths )
{
 mCurrentPaths->clear();
 // Maybe use slotAppendPath() which checks for errors?
 mCurrentPaths->insertStringList(paths);
}

void BosonSearchPathsWidget::slotRemovePath()
{
 mCurrentPaths->removeItem(mCurrentPaths->currentItem());
}

void BosonSearchPathsWidget::slotAddPath()
{
 if(mNewPath->text().isEmpty()) {
	return;
 }
 QDir d(mNewPath->text());
 if(!d.exists()) {
	// Directory doesn't exist. Display error?
	return;
 }
 mCurrentPaths->insertItem(d.absPath(), 0);
 mNewPath->setText("");
}

void BosonSearchPathsWidget::slotBrowse()
{
 mNewPath->setText(BoFileDialog::getExistingDirectory());
}

QStringList BosonSearchPathsWidget::currentPaths() const
{
 QStringList paths;
 for(unsigned int i = 0; i < mCurrentPaths->count(); i++) {
	paths.append(mCurrentPaths->item(i)->text());
 }
 return paths;
}

void BosonSearchPathsWidget::slotPathSelected( int index )
{
	boDebug() << k_funcinfo << index << endl;
 mCurrentPath = index;
 if(mCurrentPath >= 0) {
	Q3ListBoxItem* item = mCurrentPaths->item(index);
	BO_CHECK_NULL_RET(item);
	mNewPath->setText(item->text());
 }
}

void BosonSearchPathsWidget::slotCurrentPathChanged(const QString& path)
{
	boDebug() << k_funcinfo << path << endl;
 if(mCurrentPath == -1) {
	return;
 }
// mCurrentPaths->item(mCurrentPath)->setText(path);
 // QListBoxItem::setText() is protected so we have to use this workaround here
 mCurrentPaths->blockSignals(true);
 mCurrentPaths->changeItem(path, mCurrentPath);
 mCurrentPaths->blockSignals(false);
}


void BosonSearchPathsWidget::init()
{
 mCurrentPath = -1;
}
