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

#ifndef BOGAMECHAT_H
#define BOGAMECHAT_H

#include <kgame/kgamechat.h>
#include <qwidget.h>

class KGameChat;
class BoGameChatWidgetPrivate;
class BoGameChat;

/**
 * This is a widget that creates a @ref BoGameChat widget in normal game mode
 * and does nothing else. But in Qt designer mode it creates a dummy @ref QLabel
 * only. @ref KGameChat (which @ref BoGameChat is derived from) uses a @ref
 * KLineEdit which has often heavy problems with Qt designer, this workaround
 * avoids crashes.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGameChatWidget : public QWidget
{
	Q_OBJECT
public:
	/**
	 * @param game Forwarded to @ref BoGameChat
	 * @param msgid Forwarded to @ref BoGameChat
	 **/
	BoGameChatWidget(QWidget* parent = 0, const char* name = 0, KGame* game = 0, int msgid = 0);
	~BoGameChatWidget();

	BoGameChat* chatWidget() const { return mKGameChat; }

	/**
	 * Initialize the chat widget. This should be called automatically on
	 * construction when you are not in Qt designer (don't call this if you
	 * are in Qt designer).
	 *
	 * You should not need to call this.
	 * @param msgid The message id that is used for chat messages. Use 0 for
	 * the default @ref BosonMessage::IdChat.
	 **/
	void initWidget(KGame* game = 0, int msgid = 0);

private:
	BoGameChat* mKGameChat;
	BoGameChatWidgetPrivate* d;
};

/**
 * Simply a @ref KGameChat widget with an additional sending entry for the
 * script enging. See also @ref KChatBase::addSendingEntry
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGameChat : public KGameChat
{
	Q_OBJECT
public:
	/**
	 * Default c'tor that uses default values for message id and NULL for
	 * the @ref KGame object.
	 **/
	BoGameChat(QWidget* parent = 0, const char* name = 0);
	BoGameChat(KGame* game, int msgid, QWidget* parent = 0, const char* name = 0);
	~BoGameChat();

signals:
	/**
	 * The "message" was a command to the script engine.
	 **/
	void signalScriptCommand(const QString& text);

protected:
	virtual void returnPressed(const QString& text);

	/**
	 * @return TRUE if @p sendingEntryId (see @ref KChatBase::sendingEntry)
	 * is the script engine. Otherwise FALSE.
	 **/
	bool isScriptEngine(int sendingEntryId) const;

private:
	void init();

private:
	int mScriptEngine;
};

#endif
