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

#include "bogroundrendererfactory.h"
#include "bogroundrendererfactory.moc"

#include "bodefaultgroundrenderer.h"
#include "bofastgroundrenderer.h"
#include "boveryfastgroundrenderer.h"
#include "boquickgroundrenderer.h"
#include "../boversion.h"

#include <bodebug.h>

KInstance* BoGroundRendererFactory::mInstance = 0;

BoGroundRendererFactory::BoGroundRendererFactory(QObject* parent, const char* name)
	: KLibFactory(parent, name)
{
 mInstance = new KInstance("BoGroundRendererFactory");
}

BoGroundRendererFactory::~BoGroundRendererFactory()
{
 delete mInstance;
 mInstance = 0;
}

QObject* BoGroundRendererFactory::createObject(QObject* parent, const char* name,
		const char* className, const QStringList &args)
{
 Q_UNUSED(name);
 Q_UNUSED(args);
 Q_UNUSED(parent);
 QObject* o = 0;
 if (qstrcmp(className, "BoPluginInformation") == 0) {
	// note: the _libbomeshrendererplugin is NOT part of the string
	o = new BoPluginInformation_libbogroundrendererplugin;
 } else if (qstrcmp(className, "BoDefaultGroundRenderer") == 0) {
	o = new BoDefaultGroundRenderer();
 } else if (qstrcmp(className, "BoFastGroundRenderer") == 0) {
	o = new BoFastGroundRenderer();
 } else if (qstrcmp(className, "BoVeryFastGroundRenderer") == 0) {
	o = new BoVeryFastGroundRenderer();
 } else if (qstrcmp(className, "BoQuickGroundRenderer") == 0) {
	o = new BoQuickGroundRenderer();
 } else {
	boError() << k_funcinfo << "no such class available: " << className << endl;
	return 0;
 }
 boDebug() << k_funcinfo << "created object of class " << o->className() << endl;
 emit objectCreated(o);
 return o;
}


QStringList BoPluginInformation_libbogroundrendererplugin::plugins() const
{
 QStringList list;
 list.append("BoFastGroundRenderer");
 list.append("BoVeryFastGroundRenderer");
 list.append("BoQuickGroundRenderer");
 list.append("BoDefaultGroundRenderer");
 return list;
}

BO_EXPORT_PLUGIN_FACTORY( libbogroundrendererplugin, BoGroundRendererFactory )

