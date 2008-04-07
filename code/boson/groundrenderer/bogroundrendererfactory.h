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
#ifndef BOGROUNDRENDERERFACTORY_H
#define BOGROUNDRENDERERFACTORY_H

#include <klibloader.h>
#include "../bogroundrenderer.h"
#include "../bopluginmanager.h"

#include <qmap.h>

class BoGroundRenderer;

class BoGroundRendererFactory : public KLibFactory
{
	Q_OBJECT
public:
	BoGroundRendererFactory(QObject* parent = 0);
	~BoGroundRendererFactory();

protected:
	virtual QObject* create(const char* iface, QWidget* parentWidget, QObject* parent, const QVariantList& args, const QString& keyWord);

	bool rendererUsable(BoGroundRenderer* r) const;
};

class BoPluginInformation_bogroundrendererplugin : public BoPluginInformation
{
	Q_OBJECT
	Q_INTERFACES(BoPluginInformation)
public:
	BoPluginInformation_bogroundrendererplugin() : BoPluginInformation()
	{
	}
	~BoPluginInformation_bogroundrendererplugin()
	{
	}

	virtual QStringList plugins() const;

	bool rendererUsable(const QString& className) const;

	QMap<QString, bool> mRenderers;
};

#endif
