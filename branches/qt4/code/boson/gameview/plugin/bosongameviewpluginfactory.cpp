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

BosonGameViewPluginFactory::BosonGameViewPluginFactory(QObject* parent)
	: KLibFactory(parent)
{
}

BosonGameViewPluginFactory::~BosonGameViewPluginFactory()
{
}

QObject* BosonGameViewPluginFactory::create(const char* iface,
		QWidget* parentWidget,
		QObject* parent,
		const QVariantList& args,
		const QString& keyWord)
{
 Q_UNUSED(iface);
 Q_UNUSED(parentWidget);
 Q_UNUSED(parent);
 Q_UNUSED(args);
 QObject* o = 0;
 if (keyWord == "BoPluginInformation") {
	// note: the _bosongameviewplugin is NOT part of the string
	o = new BoPluginInformation_bosongameviewplugin;
 } else if (keyWord == "BosonGameViewPluginDefault") {

	// AB: note that the gamew view plugin is intended to consist of only a
	// single class!
	// -> the plugin exists only to make runtime-reloading of the plugin
	//    possible.
	// multiple classes are possible, but I don't see much use of this.

	o = new BosonGameViewPluginDefault;
 } else {
	boError() << k_funcinfo << "no such class available: " << keyWord << endl;
	return 0;
 }
 emit objectCreated(o);
 return o;
}


QStringList BoPluginInformation_bosongameviewplugin::plugins() const
{
 QStringList list;
 list.append("BosonGameViewPluginDefault");
 return list;
}

BO_EXPORT_PLUGIN_FACTORY( bosongameviewplugin, BosonGameViewPluginFactory )

