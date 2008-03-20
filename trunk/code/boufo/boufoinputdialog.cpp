/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufoinputdialog.h"
#include "boufoinputdialog.moc"

#include "boufolabel.h"
#include "boufopushbutton.h"
#include "boufolineedit.h"
#include "boufonuminput.h"
#include "boufomanager.h"

#include <bodebug.h>

#include <klocale.h>

#warning FIXME
// FIXME: libufo adds a menu with a "close" entry and a "close" button. however
// when that is used, the widget is only hidden. so if one of these is used, we
// have a memory leak, as it can't be made visible anymore
BoUfoInputDialog::BoUfoInputDialog(BoUfoManager* manager, const QString& label, const QString& title)
	: BoUfoInternalFrame(manager, title)
{
 init();
 mUfoManager = manager;
 setLabel(label);

 // TODO: position the dialog in the center of the root pane
}

BoUfoInputDialog::~BoUfoInputDialog()
{
 boDebug() << k_funcinfo << endl;
}

void BoUfoInputDialog::init()
{
 mUfoManager = 0;
 setLayoutClass(UVBoxLayout);
 mLabel = new BoUfoLabel();
 mContents = new BoUfoWidget();
 BoUfoHBox* buttons = new BoUfoHBox();
 mOk = new BoUfoPushButton(i18n("Ok"));
 mCancel = new BoUfoPushButton(i18n("Cancel"));
 mNumInput = 0;
 mLineEdit = 0;

 connect(mOk, SIGNAL(signalClicked()), this, SLOT(slotOkClicked()));
 connect(mCancel, SIGNAL(signalClicked()), this, SLOT(slotCancelled()));

 addWidget(mLabel);
 addWidget(mContents);
 buttons->addWidget(mOk);
 buttons->addWidget(mCancel);
 addWidget(buttons);
}

void BoUfoInputDialog::setLabel(const QString& label)
{
 mLabel->setText(label);
}

void BoUfoInputDialog::slotOkClicked()
{
 if (mNumInput) {
	emit signalValueEntered((int)mNumInput->value());
	emit signalValueEntered((float)mNumInput->value());
 }
 if (mLineEdit) {
	emit signalValueEntered(mLineEdit->text());
 }
 close();
}

void BoUfoInputDialog::slotCancelled()
{
 emit signalCancelled();
 close();
}

void BoUfoInputDialog::close()
{
 emit signalClosed();
 mUfoManager->removeFrame(this);
}

BoUfoNumInput* BoUfoInputDialog::numInput()
{
 if (!mNumInput) {
	mNumInput = new BoUfoNumInput();
	mContents->addWidget(mNumInput);
 }
 return mNumInput;
}

BoUfoLineEdit* BoUfoInputDialog::lineEdit()
{
 if (!mLineEdit) {
	mLineEdit = new BoUfoLineEdit();
	mContents->addWidget(mLineEdit);
 }
 return mLineEdit;
}

BoUfoInputDialog* BoUfoInputDialog::getIntegerWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title, int value, int min, int max, int step)
{
 BoUfoInputDialog* dialog = createNumDialog(manager, label, title, (float)value, (float)min, (float)max, (float)step);

 connect(dialog, SIGNAL(signalValueEntered(int)), receiver, slot);

 return dialog;
}

BoUfoInputDialog* BoUfoInputDialog::getFloatWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title, float value, float min, float max, float step)
{
 BoUfoInputDialog* dialog = createNumDialog(manager, label, title, value, min, max, step);

 connect(dialog, SIGNAL(signalValueEntered(float)), receiver, slot);

 return dialog;
}

BoUfoInputDialog* BoUfoInputDialog::getStringWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title, const QString& value)
{
 BoUfoInputDialog* dialog = new BoUfoInputDialog(manager, label, title);
// dialog->setResizable(true); // AB: not yet implemented in libufo

 BoUfoLineEdit* input = dialog->lineEdit();
 input->setText(value);

 dialog->adjustSize();

 connect(dialog, SIGNAL(signalValueEntered(const QString&)), receiver, slot);
 return dialog;
}


BoUfoInputDialog* BoUfoInputDialog::createNumDialog(BoUfoManager* manager, const QString& label, const QString& title, float value, float min, float max, float step)
{
 BoUfoInputDialog* dialog = new BoUfoInputDialog(manager, label, title);
// dialog->setResizable(true); // AB: not yet implemented in libufo

 BoUfoNumInput* input = dialog->numInput();
 input->setStepSize(step);
 input->setRange(min, max);
 input->setValue(value);

 dialog->adjustSize();

 return dialog;
}


