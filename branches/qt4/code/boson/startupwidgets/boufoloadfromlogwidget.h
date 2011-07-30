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

#ifndef BOUFOLOADFROMLOGWIDGET_H
#define BOUFOLOADFROMLOGWIDGET_H

#include "boufoloadfromlogwidgetbase.h"

class BosonStartupNetwork;
class BosonPlayField;
class Player;
class KPlayer;
class KGamePropertyBase;
class BosonCampaign;

class BoUfoLoadFromLogWidgetPrivate;
class BoUfoLoadFromLogWidget : public BoUfoLoadFromLogWidgetBase
{
	Q_OBJECT
public:
	BoUfoLoadFromLogWidget(BosonStartupNetwork* interface);
	~BoUfoLoadFromLogWidget();

	bool loadFromLog(const QString& file);

signals:
	void signalCancelled();

protected slots:
	void slotCancel();
	void slotStartGame();

	void slotNetStart();
	void slotNetStartLoadingFromLog();

protected:
	BosonStartupNetwork* networkInterface() const { return mNetworkInterface; }

private:
	BoUfoLoadFromLogWidgetPrivate* d;

	BosonStartupNetwork* mNetworkInterface;
};

#endif
