/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "pluginproperties.h"
#include "../bo3dtools.h"

#include <qstring.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qptrlist.h>

class UnitPropertiesPrivate
{
public:
	UnitPropertiesPrivate()
	{
	}

	QCString mMD5;
	QString mName;
	QString mDescription;
	QString mUnitPath; // the path to the unit files
	QValueList<unsigned long int> mRequirements;

	QPtrList<PluginProperties> mPlugins;

	QMap<QString, QString> mTextureNames;
	QMap<int, QString> mSounds;

	QMap<int, QString> mActionStrings;

	QValueList<unsigned long int> mDestroyedEffectIds;
	QValueList<unsigned long int> mConstructedEffectIds;
	QValueList<unsigned long int> mExplodingFragmentFlyEffectIds;
	QValueList<unsigned long int> mExplodingFragmentHitEffectIds;
	BoVector3Fixed mHitPoint;  // FIXME: better name

	BoUpgradesCollection mUpgradesCollection;
};


