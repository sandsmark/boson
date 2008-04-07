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

#include "boeditplayerinputswidget.h"
#include "boeditplayerinputswidget.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "../gameengine/player.h"
#include "bosonlocalplayerinput.h"
#include "bosonmenuinput.h"
#include "../gameengine/bosoncomputerio.h"

#include <qlayout.h>
#include <q3listbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <Q3ValueList>

#include <kgameio.h>

#include <klocale.h>

class BoEditPlayerInputsWidgetPrivate
{
public:
	BoEditPlayerInputsWidgetPrivate()
	{
		mPlayer = 0;

		mCurrentPlayerIOs = 0;
		mNewPlayerIOs = 0;
	}

	Player* mPlayer;

	Q3ListBox* mCurrentPlayerIOs;
	Q3ListBox* mNewPlayerIOs;

	Q3ValueList<int> mNewRTTIs;
};

BoEditPlayerInputsWidget::BoEditPlayerInputsWidget(QWidget* parent)
	: QWidget(parent)
{
 d = new BoEditPlayerInputsWidgetPrivate;

 Q3HBoxLayout* topLayout = new Q3HBoxLayout(this);
 Q3VBoxLayout* vbox = new Q3VBoxLayout(topLayout);
 QLabel* currentLabel = new QLabel(i18n("Current IOs"), this);
 vbox->addWidget(currentLabel);
 d->mCurrentPlayerIOs = new Q3ListBox(this);
 vbox->addWidget(d->mCurrentPlayerIOs, 1);
 QPushButton* remove = new QPushButton(i18n("&Remove"), this);
 connect(remove, SIGNAL(clicked()),
		this, SLOT(slotRemoveIO()));
 vbox->addWidget(remove);

 vbox = new Q3VBoxLayout(topLayout);
 QLabel* newLabel = new QLabel(i18n("Available IOs"), this);
 vbox->addWidget(newLabel);
 d->mNewPlayerIOs = new Q3ListBox(this);
 vbox->addWidget(d->mNewPlayerIOs, 1);
 QPushButton* add = new QPushButton(i18n("&Add"), this);
 connect(add, SIGNAL(clicked()),
		this, SLOT(slotAddIO()));
 vbox->addWidget(add);

}

BoEditPlayerInputsWidget::~BoEditPlayerInputsWidget()
{
 delete d;
}

void BoEditPlayerInputsWidget::setPlayer(Player* p)
{
 BO_CHECK_NULL_RET(p);
 d->mPlayer = p;
 d->mCurrentPlayerIOs->clear();
 d->mNewPlayerIOs->clear();
 d->mNewRTTIs.clear();

 foreach (KGameIO* io, *p->ioList()) {
	new Q3ListBoxText(d->mCurrentPlayerIOs, rttiString(io->rtti()));
 }


 if (!d->mPlayer->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI)) {
	d->mNewRTTIs.append(BosonLocalPlayerInput::LocalPlayerInputRTTI);
 }
 if (!d->mPlayer->findRttiIO(BosonMenuInput::RTTI)) {
	d->mNewRTTIs.append(BosonMenuInput::RTTI);
 }
 if (!d->mPlayer->findRttiIO(KGameIO::ComputerIO)) {
	d->mNewRTTIs.append(KGameIO::ComputerIO);
 }
 for (Q3ValueList<int>::iterator it = d->mNewRTTIs.begin(); it != d->mNewRTTIs.end(); ++it) {
	new Q3ListBoxText(d->mNewPlayerIOs, rttiString(*it));
 }
}

void BoEditPlayerInputsWidget::slotAddIO()
{
 BO_CHECK_NULL_RET(d->mPlayer);
 int current = d->mNewPlayerIOs->currentItem();
 if (current < 0) {
	boDebug() << k_funcinfo << "invalid index " << current << endl;
	return;
 }
 if (current >= d->mNewRTTIs.count()) {
	boDebug() << k_funcinfo << "index " << current << " out of bounds" << endl;
	return;
 }
 Q3ValueList<int>::const_iterator it = d->mNewRTTIs.at(current);
 int rtti = *it;

 switch (rtti) {
	case KGameIO::ComputerIO:
	{
		BosonComputerIO* io = new BosonComputerIO();
		d->mPlayer->addGameIO(io);
		if (!io->initializeIO()) {
			boError() << k_funcinfo << "could not initialize computer IO" << endl;
			d->mPlayer->removeGameIO(io, true);
		}
		break;
	}
	case BosonLocalPlayerInput::LocalPlayerInputRTTI:
	{
		BosonLocalPlayerInput* io = new BosonLocalPlayerInput();
		d->mPlayer->addGameIO(io);
		if (!io->initializeIO()) {
			boError() << k_funcinfo << "could not initialize localplayer IO" << endl;
			d->mPlayer->removeGameIO(io, true);
		}
		emit signalAddedLocalPlayerInput();
		break;
	}
	case BosonMenuInput::RTTI:
		emit signalAddMenuInput();
		break;
	default:
		boError() << k_funcinfo << "unhandled RTTI " << rtti << endl;
		return;
 }
 setPlayer(d->mPlayer);
}

void BoEditPlayerInputsWidget::slotRemoveIO()
{
 BO_CHECK_NULL_RET(d->mPlayer);
 int current = d->mCurrentPlayerIOs->currentItem();
 if (current < 0) {
	boDebug() << k_funcinfo << "invalid index " << current << endl;
	return;
 }
 if (current >= d->mPlayer->ioList()->count()) {
	boDebug() << k_funcinfo << "index " << current << " out of bounds" << endl;
	return;
 }
 KGameIO* io = d->mPlayer->ioList()->at(current);
 BO_CHECK_NULL_RET(io);
 d->mPlayer->removeGameIO(io);

 setPlayer(d->mPlayer);
}

QString BoEditPlayerInputsWidget::rttiString(int rtti) const
{
 QString name;
 switch (rtti) {
	case KGameIO::ComputerIO:
		name = i18n("ComputerIO");
		break;
	case BosonLocalPlayerInput::LocalPlayerInputRTTI:
		name = i18n("LocalPlayerInput");
		break;
	case BosonMenuInput::RTTI:
		name = i18n("MenuInput");
		break;
	default:
		name = i18n("Unknown");
		break;
 }
 return i18n("%1 (RTTI=%2)").arg(name).arg(rtti);
}

