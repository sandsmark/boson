/*
    This file is part of the Boson game
    Copyright (C) 2002-2008 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonchat.h"
#include "bosonchat.moc"

#include "../bomemory/bodummymemory.h"
#include "bosonconfig.h"
#include "bodebug.h"

#include <klocale.h>
#include <kgame/kgame.h>
#include <kgame/kplayer.h>
#include <kgame/kgamechat.h>
#include <kgame/kgameproperty.h>

#include <qstringlist.h>
#include <QTimer>
#include <qmap.h>
#include <Q3ValueList>
#include <Q3PtrList>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

class BosonChatPrivate
{
public:
	BosonChatPrivate()
	{
		mLabel = 0;
		mSendBox = 0;
		mEdit = 0;
		mSendTo = 0;

		mGame = 0;
		mFromPlayer = 0;
	}

	QStringList mMessages;
	Q3PtrList<unsigned int> mTimes; // how long the messages in mMessages are here already
	QTimer mRemoveTimer;

	QLabel* mLabel;
	QWidget* mSendBox;
	QLineEdit* mEdit;
	QComboBox* mSendTo;

	KGame* mGame;
	int mMessageId;
	KPlayer* mFromPlayer;

	Q3ValueList<int> mSendingEntryIds;

	// specialized IDs
	int mSendToAllId;
	int mSendToFromPlayerGroupId;
	QMap<int, int> mSendingEntryId2PlayerId;
};


BosonChat::BosonChat(QWidget* parent) : QWidget(parent)
{
 d = new BosonChatPrivate;
 d->mMessageId = -1;
 d->mSendToAllId = -1;
 d->mSendToFromPlayerGroupId = -1;
 d->mTimes.setAutoDelete(true);
 connect(&d->mRemoveTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
 d->mRemoveTimer.start(1000);

 QVBoxLayout* layout = new QVBoxLayout(this);
 d->mLabel = new QLabel(this);
 d->mLabel->setObjectName("ChatLabel");
 layout->addWidget(d->mLabel);

 d->mSendBox = new QWidget(this);
 d->mSendBox->setObjectName("ChatSendBox");
 layout->addWidget(d->mSendBox);

 QHBoxLayout* sendBoxLayout = new QHBoxLayout(d->mSendBox);

 d->mEdit = new QLineEdit(d->mSendBox);
 d->mEdit->setObjectName("ChatLineEdit");
 connect(d->mEdit, SIGNAL(signalActivated(const QString&)),
		this, SLOT(slotSendText(const QString&)));
 sendBoxLayout->addWidget(d->mEdit);

 d->mSendTo = new QComboBox(d->mSendBox);
 d->mSendTo->setObjectName("ChatSendToBox");
 sendBoxLayout->addWidget(d->mSendTo);

 d->mSendToAllId = insertSendToAllSendingEntry();
}

BosonChat::~BosonChat()
{
 d->mLabel = 0;
 d->mEdit = 0;
 clear();
 delete d;
}

void BosonChat::setFromPlayer(KPlayer* player)
{
 if (fromPlayer()) {
	removeSendingEntry(d->mSendToFromPlayerGroupId);
	d->mSendToFromPlayerGroupId = -1;
 }
 d->mFromPlayer = player;
 if (fromPlayer()) {
	d->mSendToFromPlayerGroupId = insertSendingEntry(i18n("Send to my group (\"%1\")", fromPlayer()->group()));
 }
}

KPlayer* BosonChat::fromPlayer() const
{
 return d->mFromPlayer;
}

void BosonChat::setMessageId(int msgid)
{
 d->mMessageId = msgid;
}

int BosonChat::messageId() const
{
 return d->mMessageId;
}

void BosonChat::setKGame(KGame* g, int msgid)
{
 if (msgid >= 0) {
	setMessageId(msgid);
 }
 if (g == game()) {
	return;
 }
 if (game()) {
	slotUnsetKGame();
 }
 d->mGame = g;
 if (!game()) {
	return;
 }
 connect(game(), SIGNAL(destroyed()), this, SLOT(slotUnsetKGame()));
 connect(game(), SIGNAL(signalNetworkData(int, const QByteArray&, quint32, quint32)),
		this, SLOT(slotReceiveMessage(int, const QByteArray&, quint32, quint32)));
 connect(game(), SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotAddPlayer(KPlayer*)));
 connect(game(), SIGNAL(signalPlayerLeftGame(KPlayer*)),
		this, SLOT(slotRemovePlayer(KPlayer*)));
 foreach (KPlayer* p, *game()->playerList()) {
	slotAddPlayer(p);
 }
}

KGame* BosonChat::game() const
{
 return d->mGame;
}

void BosonChat::slotUnsetKGame()
{
 if (!game()) {
	return;
 }
 disconnect(game(), 0, this, 0);
 d->mGame = 0;
}

void BosonChat::slotReceivePrivateMessage(int msgid, const QByteArray& buffer, quint32 sender, KPlayer* me)
{
 if (!me || me != fromPlayer()) {
	return;
 }
 slotReceiveMessage(msgid, buffer, me->kgameId(), sender);
}

void BosonChat::slotReceiveMessage(int msgid, const QByteArray& buffer, quint32, quint32 sender)
{
 if (!game()) {
	boError() << k_funcinfo << "Set a KGame first" << endl;
	return;
 }
 if (msgid != messageId()) {
	return;
 }
 QDataStream msg(buffer);
 QString text;
 msg >> text;

 addMessage(sender, text);
}

void BosonChat::addMessage(unsigned int p, const QString& text)
{
 if (!game()) {
	boError() << k_funcinfo << "Set a KGame first" << endl;
	return;
 }
 addMessage(game()->findPlayerByKGameId(p), text);
}

void BosonChat::addMessage(KPlayer* p, const QString& text)
{
 if (!p) {
	boError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 addMessage(i18n("%1: %2", p->name(), text));
}

void BosonChat::addMessage(const QString& text)
{
 if (boConfig->intValue("ChatScreenMaxItems") == 0) {
	// No messages allowed
	return;
 }
 if (boConfig->intValue("ChatScreenMaxItems") > 0 &&
		d->mMessages.count() + 1 > boConfig->intValue("ChatScreenMaxItems")) {
	removeFirstMessage();
 }
 d->mMessages.append(text);
 unsigned int* time = new unsigned int;
 *time = 0;
 d->mTimes.append(time);
 updateChat();
}

void BosonChat::removeFirstMessage()
{
 d->mMessages.remove(d->mMessages.begin());
 d->mTimes.removeFirst();
 updateChat();
}

void BosonChat::clear()
{
 d->mMessages.clear();
 d->mTimes.clear();
 d->mRemoveTimer.stop();
 updateChat();
}

const QStringList& BosonChat::messages() const
{
 return d->mMessages;
}

void BosonChat::slotTimeout()
{
 if (boConfig->uintValue("ChatScreenRemoveTime") > 0) {
	Q3PtrListIterator<unsigned int> it(d->mTimes);
	for (; it.current(); ++it) {
		(*it.current())++;
	}

	while (d->mTimes.count() > 0 && *d->mTimes.first() > boConfig->uintValue("ChatScreenRemoveTime")) {
		removeFirstMessage();
	}
 }
}

void BosonChat::updateChat()
{
 if (!d->mLabel) {
	return;
 }
 if (d->mMessages.count() == 0) {
	d->mLabel->setText("");
	return;
 }
 QString text;
 QStringList::Iterator it = d->mMessages.begin();
 for (; it != d->mMessages.end(); ++it) {
	text += *it + QString::fromLatin1("\n");
 }
 d->mLabel->setText(text);
}


int BosonChat::insertSendingEntry(const QString& text, int id, int index)
{
 if (id >= 0) {
	if (d->mSendingEntryIds.findIndex(id) >= 0) {
		boError() << k_funcinfo << "id " << id << " already in use" << endl;
		return -1;
	}
 }
 if (id < 0) {
	// AB: automatically assigned IDs are guaranteed to be >= 1024
	id = 1024 + d->mSendingEntryIds.count();
	while (d->mSendingEntryIds.findIndex(id) >= 0) {
		id++;
	}
 }
 if (index < 0) {
	index = d->mSendTo->count();
 }

 d->mSendTo->insertItem(text, index);
 if (d->mSendTo->currentItem() < 0) {
	d->mSendTo->setCurrentItem(index);
 }
 if (index < (int)d->mSendingEntryIds.count()) {
	d->mSendingEntryIds.insert(d->mSendingEntryIds.at(index), id);
 } else {
	d->mSendingEntryIds.append(id);
 }
 if (d->mSendTo->count() != d->mSendingEntryIds.count()) {
	boError() << k_funcinfo << "internal error!" << endl;
 }
 return id;
}

int BosonChat::insertSendToAllSendingEntry(int id, int index)
{
 return insertSendingEntry(i18n("Send to all Players"), id, index);
}

int BosonChat::insertPlayerSendingEntry(const QString& name, int id, int index)
{
 return insertSendingEntry(i18n("Send to %1", name), id, index);
}

void BosonChat::removeSendingEntry(int id)
{
 int index = d->mSendingEntryIds.findIndex(id);
 if (index < 0) {
	boError() << k_funcinfo << "don't know ID " << id << endl;
	return;
 }
 d->mSendingEntryIds.remove(id);
 d->mSendTo->removeItem(index);
}

int BosonChat::sendingEntry() const
{
 int index = d->mSendTo->currentItem();
 if (index < 0 || index >= d->mSendingEntryIds.count()) {
	return -1;
 }
 return d->mSendingEntryIds[index];
}

bool BosonChat::isToMyGroupMessage(int id) const
{
 if (id == d->mSendToFromPlayerGroupId) {
	return true;
 }
 return false;
}

bool BosonChat::isToPlayerMessage(int id) const
{
 if (d->mSendingEntryId2PlayerId.contains(id)) {
	return true;
 }
 return false;
}

bool BosonChat::isSendToAllMessage(int id) const
{
 if (id == d->mSendToAllId) {
	return true;
 }
 return false;
}

void BosonChat::slotSendText(const QString& text)
{
 BO_CHECK_NULL_RET(fromPlayer());
 BO_CHECK_NULL_RET(game());

 boDebug() << k_funcinfo << "from: " << fromPlayer()->kgameId() << "==" << fromPlayer()->name() << endl;

 int id = sendingEntry();

 int sender = fromPlayer()->kgameId();
 if (isToMyGroupMessage(id)) {
	// note: there is currently no support for other groups than the players
	// group! It might be useful to send to other groups, too
	QString group = fromPlayer()->group();
	boDebug() << k_funcinfo << "send to group " << group << endl;
	game()->sendGroupMessage(text, messageId(), sender, group);

	//TODO
	//AB: this message is never received!! we need to connect to
	//KPlayer::networkData!!!
	//TODO

 } else {
	int toPlayer = 0;
	if (!isSendToAllMessage(id) && isToPlayerMessage(id)) {
		toPlayer = d->mSendingEntryId2PlayerId[id];
		if (toPlayer == -1) {
			boError() << k_funcinfo << "don't know that player (" << id << ")"
					<< " - internal ERROR" << endl;
		}
	}
	int receiver = toPlayer;
	game()->sendMessage(text, messageId(), receiver, sender);
 }

 // Clear the lineedit widget
 d->mEdit->setText("");
}

void BosonChat::slotAddPlayer(KPlayer* p)
{
 BO_CHECK_NULL_RET(p);
 if (d->mSendingEntryId2PlayerId.values().findIndex(p->kgameId()) >= 0) {
	boError() << k_funcinfo << "player already added" << endl;
	return;
 }

 connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPropertyChanged(KGamePropertyBase*, KPlayer*)));
 connect(p, SIGNAL(signalNetworkData(int, const QByteArray&, quint32, KPlayer*)),
		this, SLOT(slotReceivePrivateMessage(int, const QByteArray&, quint32, KPlayer*)));

 int id = insertPlayerSendingEntry(p->name(), -1, -1);
 d->mSendingEntryId2PlayerId.insert(id, p->kgameId());
}

void BosonChat::slotRemovePlayer(KPlayer* p)
{
 BO_CHECK_NULL_RET(p);

 disconnect(p, 0, this, 0);

 int id = -1;
 for (QMap<int, int>::iterator it = d->mSendingEntryId2PlayerId.begin(); it != d->mSendingEntryId2PlayerId.end() && id < 0; ++it) {
	if (p->kgameId() == (unsigned int)it.data()) {
		id = it.key();
	}
 }
 if (id < 0) {
	boError() << k_funcinfo << "player not there" << endl;
	return;
 }

 removeSendingEntry(id);
 d->mSendingEntryId2PlayerId.remove(id);

 if (fromPlayer() == p) {
	boDebug() << k_funcinfo << "player is the fromplayer - resetting fromplayer" << endl;
	setFromPlayer(0);
 }
}

void BosonChat::slotPropertyChanged(KGamePropertyBase* prop, KPlayer* p)
{
 BO_CHECK_NULL_RET(prop);
 BO_CHECK_NULL_RET(p);
 if (prop->id() == KGamePropertyBase::IdName) {
	int id =  -1;
	for (QMap<int, int>::iterator it = d->mSendingEntryId2PlayerId.begin(); it != d->mSendingEntryId2PlayerId.end() && id < 0; ++it) {
		if ((unsigned int)it.data() == p->kgameId()) {
			id = it.key();
		}
	}
	if (id < 0) {
		boError() << k_funcinfo << "don't know player " << p->kgameId() << endl;
		return;
	}
	changeSendingEntry(p->name(), id);
 } else if (prop->id() == KGamePropertyBase::IdGroup) {
	if (p == fromPlayer()) {
		changeSendingEntry(p->group(), d->mSendToFromPlayerGroupId);
	}
 }
}

void BosonChat::changeSendingEntry(const QString& text, int id)
{
 int index = d->mSendingEntryIds.findIndex(id);
 if (index < 0) {
	boError() << k_funcinfo << "invalid ID " << id << endl;
	return;
 }
 d->mSendTo->setItemText(index, text);
}

void BosonChat::setSendBoxVisible(bool v)
{
 d->mSendBox->setVisible(v);
}

