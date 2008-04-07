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

BoMeshRendererFactory::BoMeshRendererFactory(QObject* parent)
	: KLibFactory(parent)
{
}

BoMeshRendererFactory::~BoMeshRendererFactory()
{
}

QObject* BoMeshRendererFactory::create(const char* iface,
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
	// note: the _bomeshrendererplugin is NOT part of the string
	o = new BoPluginInformation_bomeshrendererplugin;
 } else if (keyWord == "BoMeshRendererSemiImmediate") {
	o = new BoMeshRendererSemiImmediate;
 } else if (keyWord == "BoMeshRendererImmediate") {
	o = new BoMeshRendererImmediate;
 } else if (keyWord == "BoMeshRendererVertexArray") {
	o = new BoMeshRendererVertexArray;
 } else if (keyWord == "BoMeshRendererVBO") {
	o = new BoMeshRendererVBO;
 } else {
	boError() << k_funcinfo << "no such class available: " << keyWord << endl;
	return 0;
 }
 emit objectCreated(o);
 return o;
}


QStringList BoPluginInformation_bomeshrendererplugin::plugins() const
{
 QStringList list;
 list.append("BoMeshRendererVBO");
 list.append("BoMeshRendererVertexArray");
 list.append("BoMeshRendererSemiImmediate");
 list.append("BoMeshRendererImmediate");
 return list;
}

BO_EXPORT_PLUGIN_FACTORY( bomeshrendererplugin, BoMeshRendererFactory )

