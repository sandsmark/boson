/*
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

// note the copyright above: this is LGPL!

#include "bogamechat.h"
#include "bogamechat.moc"

#include "../bosonmessage.h"

#include <kgame/kgamechat.h>
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>

#include <qlayout.h>
#include <qlabel.h>

class BoGameChatWidgetPrivate
{
public:
	BoGameChatWidgetPrivate()
	{
		mLayout = 0;
		mDesignerLabel = 0;
	}
	bool mInitialized;
	QVBoxLayout* mLayout;
	QLabel* mDesignerLabel;
};

BoGameChatWidget::BoGameChatWidget(QWidget* parent, const char* name, KGame* game, int msgid)
	: QWidget(parent, name)
{
 d = new BoGameChatWidgetPrivate;
 d->mInitialized = false;
 if (!kapp) {
	// we are in Qt designer. create some dummy widgets, to show that this
	// is actually a chat widget.
	// do NOT create a KGameChat widget here, as it uses a KLineEdit which
	// uses kapp and that is not allowed in Qt designer. Sometimes it works,
	// but KDE breaks compatibility in that class very often (e.g. when it
	// comes to completion settings).
	d->mDesignerLabel = new QLabel("This is a (dummy) BoGameChatWidget", this);
	d->mDesignerLabel->resize(d->mDesignerLabel->sizeHint());
 } else {
	initWidget(game, msgid);
 }
}

BoGameChatWidget::~BoGameChatWidget()
{
 delete d;
}

void BoGameChatWidget::initWidget(KGame* game, int msgid)
{
 if (d->mInitialized) {
	return;
 }
 if (msgid == 0) {
	msgid = BosonMessage::IdChat;
 }
 delete d->mDesignerLabel;
 d->mDesignerLabel = 0;
 d->mLayout = new QVBoxLayout(this);
 mKGameChat = new BoGameChat(game, msgid, this);
 d->mLayout->addWidget(mKGameChat);

 d->mInitialized = true;
}

BoGameChat::BoGameChat(QWidget* parent, const char*)
	: KGameChat(0, BosonMessage::IdChat, parent)
{
 init();
}


BoGameChat::BoGameChat(KGame* game, int msgid, QWidget* parent, const char*)
	: KGameChat(game, msgid, parent)
{
 kdDebug() << k_funcinfo << endl;
 init();
}

BoGameChat::~BoGameChat()
{
}

void BoGameChat::init()
{
 // AB: we may need a list / dict / whatever here in the future. we may need to
 // be able to send commands to different scripts.
 mScriptEngine = nextId();
 addSendingEntry(i18n("Send to script engine"), mScriptEngine);
}

void BoGameChat::returnPressed(const QString& text)
{
 int id = sendingEntry();
 if (isScriptEngine(id)) {
	emit signalScriptCommand(text);
 } else {
	KGameChat::returnPressed(text);
 }
}

bool BoGameChat::isScriptEngine(int sendingEntryId) const
{
 if (sendingEntryId == mScriptEngine) {
	return true;
 }
 return false;
}
