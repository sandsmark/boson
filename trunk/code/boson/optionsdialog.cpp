/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "optionsdialog.h"

#include "defines.h"

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include <qlayout.h>

#include "optionsdialog.moc"

class OptionsDialog::OptionsDialogPrivate
{
public:
	OptionsDialogPrivate()
	{
		mArrowSpeed = 0;
		mGameSpeed = 0;
	}

	KIntNumInput* mArrowSpeed;
	KIntNumInput* mGameSpeed;
};

OptionsDialog::OptionsDialog(QWidget* parent, bool modal)
		: KDialogBase(Plain, i18n("Boson Options"), Ok|Cancel|Default,
		Cancel, parent, "bosonoptionsdialog", modal, true)
{
 d = new OptionsDialogPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(plainPage(), marginHint(), spacingHint());
 
 d->mArrowSpeed = new KIntNumInput(ARROW_KEY_STEP, plainPage());
 d->mArrowSpeed->setRange(1, 200);
 connect(d->mArrowSpeed, SIGNAL(valueChanged(int)), 
		this, SIGNAL(signalArrowScrollChanged(int)));
 topLayout->addWidget(d->mArrowSpeed);

 d->mGameSpeed = new KIntNumInput(10, plainPage());
 d->mGameSpeed->setRange(MAX_GAME_SPEED, MIN_GAME_SPEED);
 connect(d->mGameSpeed, SIGNAL(valueChanged(int)), 
		this, SLOT(slotSpeedChanged(int)));
 topLayout->addWidget(d->mGameSpeed);
}


OptionsDialog::~OptionsDialog()
{
 delete d;
}

void OptionsDialog::slotSpeedChanged(int value)
{
// value is not actually the new speed but the value supplied by the input
// (1=very slow).
// the actual speed is the value for the QTimer, where 1 would be 1ms and
// therefore *veery* fast. So lets change the value:
 if (value == 0) {
	kdError() << "cannot use speed=0" << endl;
	return;
 }

 int newSpeed = (MIN_GAME_SPEED + MAX_GAME_SPEED) - value;
 emit signalSpeedChanged(newSpeed);
}

void OptionsDialog::setGameSpeed(int ms)
{
 int value = (MIN_GAME_SPEED + MAX_GAME_SPEED) - ms;
 d->mGameSpeed->setValue(value);
}

void OptionsDialog::setArrowScrollSpeed(int value)
{
 d->mArrowSpeed->setValue(value);
}
