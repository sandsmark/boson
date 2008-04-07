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
#ifndef BOSONGAMEVIEWPLUGINMANAGER_H
#define BOSONGAMEVIEWPLUGINMANAGER_H

#include "bopluginmanager.h"

class BosonGameViewPluginBase;

class BosonGameViewPluginManagerPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGameViewPluginManager : public BoPluginManager
{
public:
	~BosonGameViewPluginManager();

	static void initStatic();
	static void deleteStatic();

	static BosonGameViewPluginManager* manager();

protected:
	virtual QString configKey() const;
	virtual QString libname() const;
	virtual KPluginFactory* initWithoutLibrary();

	virtual void initializePlugin();
	virtual void deinitializePlugin();

private:
	BosonGameViewPluginManager();

private:
	BosonGameViewPluginManagerPrivate* d;
	static BosonGameViewPluginManager* mManager;
};

#endif
