/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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

#include "bpfdescriptiondialog.h"
#include "bpfdescriptiondialog.moc"

#include "../bomemory/bodummymemory.h"
#include "gameengine/bpfdescription.h"
#include "bodebug.h"

#include <klocale.h>
#include <kcombobox.h>

#include <qlayout.h>
#include <qlineedit.h>
#include <q3vgroupbox.h>
#include <q3hbox.h>
#include <qlabel.h>
#include <q3textedit.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

class BPFDescriptionDialogPrivate
{
public:
	BPFDescriptionDialogPrivate()
	{
		mDescription = 0;
		mLanguage = 0;
		mName = 0;
		mComment = 0;
	}

	BPFDescription* mDescription;
	KComboBox* mLanguage;
	QLineEdit* mName;
	Q3TextEdit* mComment;
};

BPFDescriptionDialog::BPFDescriptionDialog(QWidget* parent, bool modal)
		: KDialogBase(Plain, i18n("Map Description"), Ok|Apply|Cancel, Ok, parent,
		"bpfdescriptiondialog", modal, true)
{
 init();
}

BPFDescriptionDialog::~BPFDescriptionDialog()
{
 delete d;
}

void BPFDescriptionDialog::init()
{
 d = new BPFDescriptionDialogPrivate;
 Q3VBoxLayout* topLayout = new Q3VBoxLayout(plainPage(), KDialog::marginHint(), KDialog::spacingHint(), "toplayout");

 Q3HBox* hbox = new Q3HBox(plainPage(), "hbox");
 (void)new QLabel(i18n("Language: "), hbox);
 d->mLanguage = new KComboBox(hbox, "language");
 d->mLanguage->insertItem(i18n("C"));
 topLayout->addWidget(hbox);

 Q3VGroupBox* groupBox = new Q3VGroupBox(i18n("Map Description"), plainPage(), "groupbox");
 topLayout->addWidget(groupBox);

 // i don't like QGroupBox's autolayout stuff and I'm too lazy for the clean
 // solution. this one is just as good.
 QWidget* w = new QWidget(groupBox);
 Q3VBoxLayout* layout = new Q3VBoxLayout(w);

 hbox = new Q3HBox(w);
 (void)new QLabel(i18n("Map name:"), hbox, "namelabel");
 d->mName = new QLineEdit(hbox, "namelineedit");
 layout->addWidget(hbox);

 d->mComment = new Q3TextEdit(w, "commentedit");
 d->mComment->setText("");
 layout->addWidget(d->mComment);
}

void BPFDescriptionDialog::setDescription(BPFDescription* description)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(description);
 d->mDescription = description;
 d->mName->setText(description->name());
 d->mComment->setText(description->comment());
}

void BPFDescriptionDialog::slotOk()
{
 slotApply();
 QDialog::accept();
}

void BPFDescriptionDialog::slotApply()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(d->mDescription);
 d->mDescription->setName(d->mName->text());
 d->mDescription->setComment(d->mComment->text());
}

