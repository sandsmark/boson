/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosongldriverworkarounds.h"

#include "bosonconfig.h"
#include "bogl.h"
#include "bodebug.h"

#include <qvaluelist.h>

BosonGLDriverWorkarounds::BosonGLDriverWorkarounds()
{
}

BosonGLDriverWorkarounds::~BosonGLDriverWorkarounds()
{
}

void BosonGLDriverWorkarounds::initWorkarounds()
{
 QString glVersion = BoGL::bogl()->OpenGLVersionString();
 if (glVersion.contains("Mesa ")) {
	QString mesaVersion = glVersion.right(glVersion.length() - glVersion.find("Mesa ") - QString("Mesa ").length());

	QValueList<int> versionParts;
	QString rest = mesaVersion;
	while (rest.contains(".")) {
		bool ok;
		int index = rest.find(".");
		QString v = rest.left(index);
		rest = rest.right(rest.length() - (index + 1));
		int number = v.toInt(&ok);
		if (!ok) {
			break;
		}
		versionParts.append(number);
	}
	if (versionParts.isEmpty()) {
		boError() << k_funcinfo << "mesa version string, but could not figure out mesa version in " << glVersion;
		return;
	}

	// AB: mesa <= 6.4.2 does not like vertex arrays: glPopClientAttrib()
	//     crashes
	//     -> actually it's that function (or glPushClientAttrib()) that's
	//        broken, but it will be used most likely with vertex arrays.
	//        and pretty much exclusively by them.
	//        so we disable vertex arrays.
	//
	//     atm (2006/09/23) the 6.5 (developer releases) line is broken,
	//     too, so up to at least 6.5.1 will be broken.
	//     the problem is fixed in more recent (i.e. cvs) versions.
	bool brokenVertexArrays = false;
	QValueList<int> maxBrokenVersion;
	maxBrokenVersion.append(6);
	maxBrokenVersion.append(5);
	maxBrokenVersion.append(1);

	// note that the following code will catch a 6.5.2 as fixed, but NOT a
	// 6.4.3
	// this is intended: I can't tell for sure whether the fix is included
	// in a 6.4.3 (assuming there will ever be one)
	for (unsigned int i = 0; i < QMIN(maxBrokenVersion.count(), versionParts.count()); i++) {
		if (versionParts[i] < maxBrokenVersion[i]) {
			brokenVertexArrays = true;
			break;
		} else if (versionParts[i] > maxBrokenVersion[i]) {
			brokenVertexArrays = false;
			break;
		}
	}

	if (brokenVertexArrays) {
		boWarning() << "detected mesa drivers with broken vertex arrays implementation. disable vertex arrays." << endl;
		boWarning() << "this may produce unexpected behaviour, such as certain objects not being rendered at all. consider upgrading your GL drivers." << endl;

		boConfig->setBoolValue("EnableMesaVertexArraysWorkarounds", true);
	}
 }

}

