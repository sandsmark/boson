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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bomeshrendererfactory.h"
#include "bomeshrendererfactory.moc"

#include "../../../bomemory/bodummymemory.h"
#include "bomeshrendererimmediate.h"
#include "bomeshrenderersemiimmediate.h"
#include "bomeshrenderervertexarray.h"
#include "bomeshrenderervbo.h"
#include "../../boversion.h"

#include <bodebug.h>

KInstance* BoMeshRendererFactory::mInstance = 0;

BoMeshRendererFactory::BoMeshRendererFactory(QObject* parent, const char* name)
	: KLibFactory(parent, name)
{
 mInstance = new KInstance("BoMeshRendererFactory");
}

BoMeshRendererFactory::~BoMeshRendererFactory()
{
 delete mInstance;
 mInstance = 0;
}

QObject* BoMeshRendererFactory::createObject(QObject* parent, const char* name,
		const char* className, const QStringList &args)
{
 Q_UNUSED(name);
 Q_UNUSED(args);
 Q_UNUSED(parent);
 QObject* o = 0;
 if (qstrcmp(className, "BoPluginInformation") == 0) {
	// note: the _libbomeshrendererplugin is NOT part of the string
	o = new BoPluginInformation_libbomeshrendererplugin;
 } else if (qstrcmp(className, "BoMeshRendererSemiImmediate") == 0) {
	o = new BoMeshRendererSemiImmediate;
 } else if (qstrcmp(className, "BoMeshRendererImmediate") == 0) {
	o = new BoMeshRendererImmediate;
 } else if (qstrcmp(className, "BoMeshRendererVertexArray") == 0) {
	o = new BoMeshRendererVertexArray;
 } else if (qstrcmp(className, "BoMeshRendererVBO") == 0) {
	o = new BoMeshRendererVBO;
 } else {
	boError() << k_funcinfo << "no such class available: " << className << endl;
	return 0;
 }
 emit objectCreated(o);
 return o;
}


QStringList BoPluginInformation_libbomeshrendererplugin::plugins() const
{
 QStringList list;
 list.append("BoMeshRendererVBO");
 list.append("BoMeshRendererVertexArray");
 list.append("BoMeshRendererSemiImmediate");
 list.append("BoMeshRendererImmediate");
 return list;
}

BO_EXPORT_PLUGIN_FACTORY( libbomeshrendererplugin, BoMeshRendererFactory )

