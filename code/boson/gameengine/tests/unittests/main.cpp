/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "../../bomemory/bodummymemory.h"
#include <config.h>
#include "boversion.h"
#include "bodebug.h"
#include "bodebugdcopiface.h"
#include "bosonconfig.h"
#include "boapplication.h"
#include "gameengine/boeventloop.h"
#include "boglobal.h"

#include "maptest.h"
#include "playfieldtest.h"
#include "canvastest.h"
#include "movetest.h"
#include "constructiontest.h"
#include "productiontest.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kinstance.h>

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
    { 0, 0, 0 }
};

static bool startTests();

int main(int argc, char **argv)
{
 BoDebug::disableAreas(); // dont load bodebug.areas
 KAboutData about("bosontests",
		I18N_NOOP("BosonTests"),
		version);
 about.addAuthor("Andreas Beckermann",
		I18N_NOOP("Coding & Current Maintainer"),
		"b_mann@gmx.de");

 QCString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
#if BOSON_LINK_STATIC
 KApplication::disableAutoDcopRegistration();
#endif

 BoGlobal::initStatic();
 BoGlobal::boGlobal()->initGlobalObjects();

 KInstance instance(&about);

 if (!startTests()) {
	boDebug() << "tests FAILED" << endl;
 } else {
	boDebug() << "tests completed successfully" << endl;
 }

 return 0;
}

static bool startTests()
{
#define ADD_TEST(x) \
	x test_##x; \
	if (!test_##x.test()) { \
		boError() << k_funcinfo << #x " failed" << endl; \
		return false; \
	}
 ADD_TEST(MapTest);
 ADD_TEST(PlayFieldTest);
 ADD_TEST(CanvasTest);
 ADD_TEST(MoveTest);
 ADD_TEST(ConstructionTest);
 ADD_TEST(ProductionTest);

 return true;
}

