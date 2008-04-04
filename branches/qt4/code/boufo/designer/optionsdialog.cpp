/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "optionsdialog.h"
#include "optionsdialog.moc"

#include <bodebug.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qdir.h>
#include <qsettings.h>


class OptionsDialogPrivate
{
public:
	OptionsDialogPrivate()
	{
		mDataDir = 0;
	}
	QLineEdit* mDataDir;
};

OptionsDialog::OptionsDialog(QWidget* parent)
	: QDialog(parent, false)
{
 d = new OptionsDialogPrivate;
 QVBoxLayout* layout = new QVBoxLayout(this);

 QVBoxLayout* vbox;
 QHBoxLayout* hbox;
 QLabel* label;

 vbox = new QVBoxLayout(layout);
 label = new QLabel(tr("libufo data_dir - all file paths (images/pixmaps, ...) are relative to this directory."), this);
 vbox->addWidget(label);
 hbox = new QHBoxLayout(vbox);
 d->mDataDir = new QLineEdit(this);
 QPushButton* browseDataDir = new QPushButton(tr("..."), this);
 connect(browseDataDir, SIGNAL(clicked()),
		this, SLOT(slotBrowseDataDir()));
 hbox->addWidget(d->mDataDir);
 hbox->addWidget(browseDataDir);


 QPushButton* ok = new QPushButton(tr("&Ok"), this);
 QPushButton* apply = new QPushButton(tr("&Apply"), this);
 QPushButton* cancel = new QPushButton(tr("&Cancel"), this);
 connect(ok, SIGNAL(clicked()),
		this, SLOT(slotOk()));
 connect(apply, SIGNAL(clicked()),
		this, SLOT(slotApply()));
 connect(cancel, SIGNAL(clicked()),
		this, SLOT(reject()));
 QHBoxLayout* buttonsLayout = new QHBoxLayout(layout);
 buttonsLayout->addStretch(1);
 buttonsLayout->addWidget(ok);
 buttonsLayout->addWidget(apply);
 buttonsLayout->addWidget(cancel);

 QSettings settings;
 settings.setPath("boson.eu.org", "boufodesigner");
 d->mDataDir->setText(settings.readEntry("/boufodesigner/data_dir"));
}

OptionsDialog::~OptionsDialog()
{
 delete d;
}

void OptionsDialog::slotOk()
{
 slotApply();
 accept();
}

void OptionsDialog::slotApply()
{
 QSettings settings;
 settings.setPath("boson.eu.org", "boufodesigner");
 settings.writeEntry("/boufodesigner/data_dir", d->mDataDir->text());

 emit signalApplyOptions();
}

void OptionsDialog::slotBrowseDataDir()
{
 QString defaultDir;
 {
	QDir dir(d->mDataDir->text());
	if (dir.exists()) {
		if (dir.cdUp()) {
			defaultDir = dir.absPath();
		}
	}
 }
 QString dir = QFileDialog::getExistingDirectory(defaultDir, this);
 if (!dir.isEmpty()) {
	d->mDataDir->setText(dir);
 }
}

