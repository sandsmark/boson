/*
    This file is part of the Boson game
    Copyright (C) 2004-2008 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOADVANCECONTROL_H
#define BOADVANCECONTROL_H

#include <QObject>

class BoAdvanceControlPrivate;

/**
 * The BoAdvanceControl class controls at what time an "advance call" is made.
 *
 * Every "advance message" in boson causes n advance calls, with n depending on
 * the game speed. The calls are supposed to be made linearly over the advance
 * message interval - e.g. if 10 calls should be made per advance message and
 * every 250ms there is one advance message, then one call should be made every
 * 25ms.
 *
 * This class takes care that these time constraints are met as good as
 * possible. If there are already advance messages waiting before all advance
 * calls of the previous message were made, the remaining calls are made as soon
 * as possible (possibly with limited blocking).
 *
 * In addition, this class may decide that there is "extra time" available until
 * the next advance call needs to be made and that this time could be spent
 * rendering another frame. In that case this class will emit @ref
 * signalUpdateGL.
 *
 *
 * The class should be used in the following way:
 * @li Create one (possibly global) object of this class.
 * @li Initialize this object, in particular call @ref setAdvanceObject. See
 *     @ref setAdvanceObject for details on this object.
 * @li Whenever an advance message is received, calls @ref
 *     receivedAdvanceMessage. A message is "received" when it is emitted by @ref
 *     KGame::signalNetworkData(). Note that when it arrives at @ref
 *     Boson::netorkTransmission(), it may @em not get received yet (i.e. it may
 *     get delayed).
 * @li Whenever advance messages get delayed, call @ref
 *     setAdvanceMessagesWaiting. An advance message should be delayed when an advance
 *     message arrives from the network, i.e. at KGame::netorkTransmission(),
 *     but not all advance calls from the previous message have been made.
 * @li Start the internal BoAdvanceControl loop by calling @ref slotProcess().
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoAdvanceControl : public QObject
{
	Q_OBJECT
public:
	BoAdvanceControl(QObject* parent = 0);
	~BoAdvanceControl();


	/**
	 * Set the advance object. This object should know how to process advance calls.
	 *
	 * The advance object will
	 * receive a @ref QEvent with type @ref QEvent::User + QtEventAdvanceCall
	 * whenever such an event should be made.
	 *
	 * In addition, the advance object will
	 * receive a @ref QEvent with type @ref QEvent::User +
	 * QtEventAdvanceMessageCompleted
	 * when all advance calls that belong to an advance message have been
	 * made.
	 **/
	void setAdvanceObject(QObject* object);
	void setAdvanceMessageInterval(int interval);
	void receivedAdvanceMessage(int gameSpeed);
	void setAdvanceMessagesWaiting(int count);

public slots:
	/**
	 * Check whether an advance call needs to be made, and call @ref
	 * postAdvanceCallEvent if we need to make one.
	 *
	 * This method needs to be called once only (e.g. using
	 * QTimer::singleShot). After that, it takes care of calling itself
	 * (asynchronously using @ref QTimer::singleShot) when necessary.
	 **/
	void slotProcess();

protected:
	/**
	 * Post an QtEventAdvanceCall event to the advance object (see @ref
	 * setAdvanceObject).
	 **/
	void postAdvanceCallEvent();

signals:
	void signalUpdateGL();

private:
	QObject* mAdvanceObject;
	BoAdvanceControlPrivate* d;
};

#endif

