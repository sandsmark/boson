/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOMESHRENDERERFACTORY_H
#define BOMESHRENDERERFACTORY_H

#include <klibloader.h>
#include "../bomeshrenderer.h"

// AB: we should move this macro to a more generic place when we start to use
// several kinds of plugins (atm this is the only dynamically loaded plugin
// in boson)

/**
 * Every boson plugin must provide at least two plain C functions, which are the
 * interface to the plugin.
 * <li>init_libname() where "libname" is the name of the library, without the
 * .la (but including the "lib"). This function must return a pointer to the
 * factory of the plugin, which must be derived from @ref KLibFactory.
 * <li>version_libname() where "libname" is equal to the one above. This returns
 * simply BOSON_VERSION from boversion.h. This function is used by the plugin
 * loader to find out whether the plugin can be used.
 *
 * Both of these functions are automatically added by this macro. Make sure that
 * you #include <boversion.h> ! (or tell me how to do that in a macro)
 **/
// AB: probably add a #include <boversion.h> to the file defining this macro
#define BO_EXPORT_PLUGIN_FACTORY( libname, factory ) \
	extern "C" { \
		void* init_##libname() { return new factory; } \
		int version_##libname() { return BOSON_VERSION; } \
	}


class BoMeshRendererFactory : public KLibFactory
{
	Q_OBJECT
public:
	BoMeshRendererFactory(QObject* parent = 0, const char* name = 0);
	~BoMeshRendererFactory();

protected:
	virtual QObject* createObject(QObject* parent = 0, const char* name = 0,
			const char* className = "QObject",
			const QStringList &args = QStringList());

private:
	static KInstance* mInstance;
};

class BoMeshRendererInformation_libbomeshrendererplugin : public BoMeshRendererInformation
{
	Q_OBJECT
public:
	BoMeshRendererInformation_libbomeshrendererplugin() : BoMeshRendererInformation()
	{
	}
	~BoMeshRendererInformation_libbomeshrendererplugin()
	{
	}

	virtual QStringList meshRenderers() const;

};

#endif
