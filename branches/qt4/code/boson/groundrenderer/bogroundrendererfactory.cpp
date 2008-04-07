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

#include "bogroundrendererfactory.h"
#include "bogroundrendererfactory.moc"

#include "../../bomemory/bodummymemory.h"
#include "bodefaultgroundrenderer.h"
#include "bofastgroundrenderer.h"
#include "boveryfastgroundrenderer.h"
#include "boquickgroundrenderer.h"
#include "../boversion.h"

#include <bodebug.h>

BoGroundRendererFactory::BoGroundRendererFactory(QObject* parent)
	: KLibFactory(parent)
{
}

BoGroundRendererFactory::~BoGroundRendererFactory()
{
}

QObject* BoGroundRendererFactory::create(const char* iface,
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
 bool initrenderer = true;

 if (keyWord == "BoPluginInformation") {
	// note: the _bogroundrendererplugin is NOT part of the string
	o = new BoPluginInformation_bogroundrendererplugin;
	BoPluginInformation_bogroundrendererplugin* info = (BoPluginInformation_bogroundrendererplugin*)o;
	// Check which renderers are usable
	bool usable;
	usable = rendererUsable(new BoFastGroundRenderer);
	info->mRenderers["BoFastGroundRenderer"] = rendererUsable(new BoFastGroundRenderer);
	info->mRenderers["BoVeryFastGroundRenderer"] = rendererUsable(new BoVeryFastGroundRenderer);
	info->mRenderers["BoQuickGroundRenderer"] = rendererUsable(new BoQuickGroundRenderer);
	info->mRenderers["BoDefaultGroundRenderer"] = rendererUsable(new BoDefaultGroundRenderer);
	initrenderer = false;
 } else if (keyWord == "BoDefaultGroundRenderer") {
	o = new BoDefaultGroundRenderer();
 } else if (keyWord == "BoFastGroundRenderer") {
	o = new BoFastGroundRenderer();
 } else if (keyWord == "BoVeryFastGroundRenderer") {
	o = new BoVeryFastGroundRenderer();
 } else if (keyWord == "BoQuickGroundRenderer") {
	o = new BoQuickGroundRenderer();
 } else {
	boError() << k_funcinfo << "no such class available: " << keyWord << endl;
	return 0;
 }

 if (initrenderer) {
	((BoGroundRenderer*)o)->initGroundRenderer();
 }
 emit objectCreated(o);
 return o;
}

bool BoGroundRendererFactory::rendererUsable(BoGroundRenderer* r) const
{
 bool u = r->usable();
 delete r;
 return u;
}


QStringList BoPluginInformation_bogroundrendererplugin::plugins() const
{
 QStringList list;
 QMap<QString, bool>::const_iterator it;
 for (it = mRenderers.begin(); it != mRenderers.end(); ++it) {
	if (it.value()) {
		list.append(it.key());
	}
 }
 return list;
}

bool BoPluginInformation_bogroundrendererplugin::rendererUsable(const QString& className) const
{
 QMap<QString, bool>::const_iterator it = mRenderers.find(className);
 if (it == mRenderers.end()) {
	return false;
 } else {
	return it.value();
 }
}

BO_EXPORT_PLUGIN_FACTORY( bogroundrendererplugin, BoGroundRendererFactory )

