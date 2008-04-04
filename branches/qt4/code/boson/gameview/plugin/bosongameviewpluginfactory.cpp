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

#include "bosongameviewpluginfactory.h"
#include "bosongameviewpluginfactory.moc"

#include "../../../bomemory/bodummymemory.h"
#include "../../boversion.h"
#include "bosongameviewplugindefault.h"

#include <bodebug.h>

KInstance* BosonGameViewPluginFactory::mInstance = 0;

BosonGameViewPluginFactory::BosonGameViewPluginFactory(QObject* parent, const char* name)
	: KLibFactory(parent, name)
{
 mInstance = new KInstance("BosonGameViewPluginFactory");
}

BosonGameViewPluginFactory::~BosonGameViewPluginFactory()
{
 delete mInstance;
 mInstance = 0;
}

QObject* BosonGameViewPluginFactory::createObject(QObject* parent, const char* name,
		const char* className, const QStringList &args)
{
 Q_UNUSED(name);
 Q_UNUSED(args);
 Q_UNUSED(parent);
 QObject* o = 0;
 if (qstrcmp(className, "BoPluginInformation") == 0) {
	// note: the _libbosongameviewplugin is NOT part of the string
	o = new BoPluginInformation_libbosongameviewplugin;
 } else if (qstrcmp(className, "BosonGameViewPluginDefault") == 0) {

	// AB: note that the gamew view plugin is intended to consist of only a
	// single class!
	// -> the plugin exists only to make runtime-reloading of the plugin
	//    possible.
	// multiple classes are possible, but I don't see much use of this.

	o = new BosonGameViewPluginDefault;
 } else {
	boError() << k_funcinfo << "no such class available: " << className << endl;
	return 0;
 }
 emit objectCreated(o);
 return o;
}


QStringList BoPluginInformation_libbosongameviewplugin::plugins() const
{
 QStringList list;
 list.append("BosonGameViewPluginDefault");
 return list;
}

BO_EXPORT_PLUGIN_FACTORY( libbosongameviewplugin, BosonGameViewPluginFactory )

