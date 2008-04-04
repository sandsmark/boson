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
#include "bocheckinstallation.h"

#include "defines.h"
#include "../bomemory/bodummymemory.h"
#include "gameengine/bosonplayfield.h"
#include "bosongroundtheme.h"
#include "bodebug.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include <qpixmap.h>

QString BoCheckInstallation::checkInstallation()
{
 QStringList requiredFiles;
 requiredFiles.append(locate("data", "boson/pics/boson-startup-bg.png"));
 requiredFiles.append(locate("data", "boson/pics/boson-startup-logo.png"));
 for (QStringList::iterator it = requiredFiles.begin(); it != requiredFiles.end(); ++it) {
	if (!KGlobal::dirs()->exists(*it)) {
		return i18n("You seem not to have Boson data files installed!\n Please install data package of Boson and restart Boson.");
	}
 }

 if (BosonGroundTheme::groundThemeFiles().count() == 0) {
	return i18n("No ground themes found. Check your installation!");
 }
 if (BosonPlayField::findAvailablePlayFields().count() == 0) {
	return i18n("No playfields found. Check your installation!");
 }

 if (KGlobal::dirs()->findResource("exe", "bobmfconverter").isNull()) {
	if (KGlobal::dirs()->findExe("bobmfconverter").isNull()) {
		return i18n("You seem not to have the \"bobmfconverer\" binary installed!\nPlease check your installation.");
	}

	// TODO: check version
 }

 return QString::null;
}

