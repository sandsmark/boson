/*
    This file is part of the Boson game
    Copyright (C) 2005 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bocheckinstallation.h"

#include "defines.h"
#include "bodebug.h"

#include <klocale.h>
#include <kstandarddirs.h>

#include <qpixmap.h>

QString BoCheckInstallation::checkInstallation()
{
 boDebug() << k_funcinfo << endl;

 // TODO just use KStandarddirs::exist() ?
 QPixmap p1(locate("data", "boson/pics/boson-startup-bg.png"));
 QPixmap p2(locate("data", "boson/pics/boson-startup-logo.png"));
  if (p1.isNull() || p2.isNull()) {
	return i18n("You seem not to have Boson data files installed!\n Please install data package of Boson and restart Boson.");
}

#warning TODO: check without preloading data
#if 0
 if (!BosonGroundTheme::createGroundThemeList()) {
	return i18n("Unable to load groundThemes. Check your installation!");
 }
 if (!BosonPlayField::preLoadAllPlayFields()) {
	return i18n("Unable to preload playFields. Check your installation!");
 }
#endif

 boDebug() << k_funcinfo << "successful" << endl;
 return QString::null;
}

