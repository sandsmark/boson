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

#include <qwidget.h>

class KGameChat;
class BoGameChatWidgetPrivate;

class BoGameChatWidget : public QWidget
{
	Q_OBJECT
public:
	BoGameChatWidget(QWidget* parent, const char* name);
	~BoGameChatWidget();

	KGameChat* chatWidget() const { return mKGameChat; }

	/**
	 * Initialize the chat widget. This should be called automatically on
	 * construction when you are not in Qt designer (don't call this if you
	 * are in Qt designer).
	 *
	 * You should not need to call this.
	 **/
	void initWidget();

private:
	KGameChat* mKGameChat;
	BoGameChatWidgetPrivate* d;
};

#endif
