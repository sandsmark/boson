/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "../boufo.h"
#include <bogl.h>

#include "boufodesignermain.h"
#include <bodebug.h>

#include <qapplication.h>
#include <qsettings.h>

#include <stdlib.h>
#include <iostream>

int main(int argc, char **argv)
{
 std::cout << "resolving GL, GLX and GLU symbols" << std::endl;
 if (!boglResolveGLSymbols()) {
#warning TODO: messagebox
	// TODO: open a messagebox
	std::cerr << "Could not resolve all symbols!" << std::endl;
    return 1;
 }
 std::cout << "GL, GLX and GLU symbols successfully resolved" << std::endl;


 QApplication app(argc, argv);
 QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

 BoUfoDesignerMain* main = new BoUfoDesignerMain();
 app.setMainWidget(main);
 main->show();

 QString file;
 for (int i = 0; i < app.argc(); i++) {
	QString a = app.argv()[i];
	if (a == "--file") {
		if (i + 1 >= app.argc()) {
			boError() << "--file expects a filename" << endl;
			return 1;
		}
		file = app.argv()[i + 1];
	}
 }

 if (file.isEmpty()) {
	QSettings settings;
	settings.setPath("boson.eu.org", "boufodesigner");
	file = settings.readEntry("/boufodesigner/MostRecentFile");
 }

 if (file.isEmpty()) {
	if (!main->slotCreateNew()) {
		boError() << k_funcinfo << "could not create new document" << endl;
	}
 } else {
	if (!main->slotLoadFromFile(file)) {
		boError() << k_funcinfo << "could not load " << file << endl;
	}
 }

 return app.exec();
}

