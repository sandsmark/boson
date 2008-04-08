/*
    This file is part of the Boson game
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "../bomemory/bodummymemory.h"
#include "bosonmainwidget.h"
#include "bocheckinstallation.h"
#include "bosonconfig.h"
#include "boglobal.h"
#include "boapplication.h"
#include "boversion.h"
#include "bodebug.h"
#include "bo3dtools.h"
//#include "boeventloop.h"
#include "bosongameengine.h"
#include "bosongldriverworkarounds.h"
#include <config.h>
#include <bogl.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmessagebox.h>
//Added by qt3to4:
#include <Q3CString>

// sound is enabled by default atm
#define HARDCODE_NOSOUND 0


static const char *version = BOSON_VERSION_STRING;



void postBosonConfigInit();


int main(int argc, char **argv)
{
 KAboutData about("boson",
        QByteArray(),
        ki18n("Boson"),
        version,
        ki18n("A realtime strategy game for KDE"),
        KAboutData::License_GPL,
        ki18n("(C) 1999-2000,2001-2005 The Boson team"),
        KLocalizedString(),
        "http://boson.eu.org");
 about.addAuthor(ki18n("Thomas Capricelli"), ki18n("Initial Game Design & Coding"),
        "orzel@freehackers.org", "http://orzel.freehackers.org");
 about.addAuthor(ki18n("Benjamin Adler"), ki18n("Graphics & Homepage Design"),
        "benadler@bigfoot.de");
 about.addAuthor(ki18n("Andreas Beckermann"), ki18n("Coding & Current Maintainer"), "b_mann@gmx.de");
 about.addAuthor(ki18n("Rivo Laks"), ki18n("Coding & Homepage Redesign"), "rivolaks@hot.ee");
 about.addAuthor(ki18n("Felix Seeger"), ki18n("Documentation"), "felix.seeger@gmx.de");
 about.addAuthor(ki18n("Christopher J. Jepson (Xenthorious)"), ki18n("Modeling & texturing"), "xenthorious@yahoo.com");


 KCmdLineOptions options;
#if HARDCODE_NOSOUND
 options.add("sound", ki18n("Enable Sounds"));
#else
 options.add("nosound", ki18n("Disable Sounds"));
#endif
 options.add("new", ki18n("Skip Welcome Widget and display the New Game screen"));
 options.add("editor", ki18n("Skip Welcome Widget and display the Start Editor screen"));
 options.add("load", ki18n("Skip Welcome Widget and display the Load Game screen"));
 options.add("load-from-log <file>", ki18n("Load from emergency log, for debugging"));
 options.add("playfield <identifier>", ki18n("Playfield identifier for newgame/start editor widget"));
 options.add("computer <count>", ki18n("Add (currently dummy) computer player"));
 options.add("start", ki18n("Start the game"));
 options.add("aidelay <delay>", ki18n("Set AI delay (in seconds). The less it is, the faster AI will send it's units"));
 options.add("noai", ki18n("Disable AI"));
 options.add("indirect", ki18n("Use Indirect rendering (sloooow!! - debugging only)"));
 options.add("ati-depth-workaround", ki18n("Enable the ATI (proprietary) driver workaround for reading the depth buffer. Will use depth of 0.00390625. This workaround is deprecated and not required anymore!"));
 options.add("mesa-vertexarray-workaround", ki18n("Enable the MESA driver workaround for vertex arrays. This is required for mesa <= 6.5.1, which will probably crash otherwise."));
 options.add("ati-depth-workaround-depth <depth>", ki18n("Use with --ati-depth-workaround. Supply a depth value for your system (default=0.00390625)"));
 options.add("default-lodcount <count>", ki18n("Use <count> for default level of detail count"));
 options.add("nomodels", ki18n("Disable model loading for faster startup (you won't see the units)"));
 options.add("notexturecompression", ki18n("Disable texture compression for faster startup"));
 options.add("fast", ki18n("Fast Startup"));
 options.add("veryfast", ki18n("Very Fast Startup (debugging only!)"));



 // first tell BoGlobal that we need to do extra stuff after BosonConfig's
 // initialization
 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 Q3CString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);

 boDebug() << k_funcinfo << "Boson " << version << " is starting..." << endl;
 boDebug() << k_funcinfo << "resolving GL, GLX and GLU symbols" << endl;
 if (!boglResolveGLSymbols()) {
#warning TODO: messagebox
	// TODO: open a messagebox
	boError() << k_funcinfo << "Could not resolve all symbols!" << endl;
	return 1;
 }
 boDebug() << k_funcinfo << "GL, GLX and GLU symbols successfully resolved" << endl;

 //BoEventLoop eventLoop(0, "main event loop");
 BoApplication app(argv0);
 KGlobal::locale()->insertCatalog("libkdegames");

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

 // make sure the data files are installed at the correct location
 BoCheckInstallation checkInstallation;
 QString errorMessage = checkInstallation.checkInstallation();
 if (!errorMessage.isNull()) {
	boError() << k_funcinfo << errorMessage << endl;
	boError() << k_funcinfo << "check your installation!" << endl;
	KMessageBox::sorry(0, errorMessage, i18n("Check your installation"));
	return 1;
 }

 BosonGameEngine* gameEngine = new BosonGameEngine(0);

 bool forceWantDirect = boConfig->boolValue("ForceWantDirect");
 BosonMainWidget* top = new BosonMainWidget(0, forceWantDirect);

 bool directRendering = top->format().directRendering();
 if (!directRendering) {
	boWarning() << "using INDIRECT (software) rendering" << endl;
 }
#if 0
 QString glDriverBroken = top->glDriverBroken();
 if (!glDriverBroken.isEmpty()) {
	KMessageBox::information(0, i18n("Your OpenGL driver appears to be broken! The reported error is:\n%1"
			"\n\nGL library used: %2"
			"\nGLU library used: %3"
			"\n\nThis error will be ignored, but will probably cause problems later on!", 
			glDriverBroken, 
			BoGL::bogl()->OpenGLFile(), 
			BoGL::bogl()->GLUFile()));
 }
#else
#warning TODO: port BosonGLWidget::glDriverBroken()
#endif
 if (!directRendering) {
	KMessageBox::information(0, i18n("Direct rendering is NOT enabled! 3d acceleration is DISABLED.\nBoson will run very slowly (seconds per frame instead of frames per second).\n\nIf you are sure that your 3d drivers are installed correctly and support 3d acceleration, please let us know about this problem and help us fixing it: boson-devel@lists.sourceforge.net"));
 }

 BosonGLDriverWorkarounds::initWorkarounds();
 if (args->isSet("mesa-vertexarray-workaround")) {
	boConfig->setBoolValue("EnableMesaVertexArraysWorkarounds", true);
 }

 top->initUfoGUI();

 top->show();

 top->setGameEngine(gameEngine);

 if (!gameEngine->preloadData()) {
	boError() << k_funcinfo << "unable to preload some data" << endl;
	KMessageBox::sorry(0, i18n("Unable to preload data. Check your installation!"), i18n("Check your installation"));
	return 1;
 }

 // pretend an old game was over. here we actually start
 top->slotGameOver();

 if (boConfig->boolValue("EnableATIDepthWorkaround", false)) {
	double depth = boConfig->doubleValue("ATIDepthWorkaroundValue", 0.00390625);
	Bo3dTools::enableReadDepthBufferWorkaround((float)depth);
 }

 if (args->isSet("ati-depth-workaround") || args->isSet("ati-depth-workaround-depth")) {
	// this is the value that a call to
	// glReadPixels(x,y,1,1,GL_DEPTH_COMPONENT, GL_FLOAT, &depth); returns
	// when it should return 1.0 (i.e. is freshly cleared with 1.0)
	float depth = 0.00390625;
	if (args->isSet("ati-depth-workaround-depth")) {
		QString s = args->getOption("ati-depth-workaround-depth");
		bool ok = false;
		depth = s.toFloat(&ok);
		if (!ok) {
			boError() << "depth of " << s << " is not a valid floating point number!" << endl;
			return 1;
		}
	}
	Bo3dTools::enableReadDepthBufferWorkaround(depth);
 }
 if (args->isSet("default-lodcount")) {
	bool ok = false;
	unsigned int v = 0;
	QString s = args->getOption("default-lodcount");
	v = s.toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "default-lodcount was not a valid number" << endl;
		return 1;
	}
	boConfig->setIntValue("DefaultLodCount", v);
 }
 if (!args->isSet("models")) {
	boConfig->setBoolValue("ForceDisableModelLoading", true);
 }
 if (args->isSet("texturecompression")) {
	boConfig->setBoolValue("ForceDisableTextureCompression", false);
 } else {
	boConfig->setBoolValue("ForceDisableTextureCompression", true);
 }
 if (args->isSet("new")) {
	top->slotShowNewGamePage(args);
 } else if (args->isSet("editor")) {
	top->slotShowStartEditorPage(args);
 } else if (args->isSet("load")) {
	top->slotShowLoadGamePage(args);
 } else if (args->isSet("load-from-log")) {
	QString file = args->getOption("load-from-log");
	top->slotLoadFromLog(file);
 }
 args->clear();

 if (boConfig->boolValue("ForceDisableModelLoading")) {
	boWarning() << "model loading disabled - you will not see any units!" << endl;
 }
 int ret = app.exec();

 delete top;
 delete gameEngine;

 return ret;
}

void postBosonConfigInit()
{
 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 if (!args) {
	boError() << k_funcinfo << "NULL cmdline args" << endl;
	return;
 }
 if (!BoGlobal::boGlobal()) {
	boError() << k_funcinfo << "NULL BoGlobal object" << endl;
	return;
 }
 BosonConfig* conf = BoGlobal::boGlobal()->bosonConfig();
 if (!conf) {
	boError() << k_funcinfo << "NULL BosonConfig object" << endl;
	return;
 }
 if (args->isSet("sound")) {
	boConfig->setBoolValue("ForceDisableSound", false);
 }
 if (!args->isSet("ai")) {
	boDebug() << k_funcinfo << "ai arg is not set" << endl;
	boConfig->setDoubleValue("AIDelay", -1.0);
 } else if (args->isSet("aidelay")) {
	QString delay = args->getOption("aidelay");
	bool ok;
	float aidelay = delay.toFloat(&ok);
	if (ok) {
		boConfig->setDoubleValue("AIDelay", aidelay);
		boDebug() << k_funcinfo << "aidelay set to " << boConfig->doubleValue("AIDelay") << endl;
	} else {
		boError() << k_funcinfo << "aidelay is not a valid float!" << endl;
	}
 }
 if (args->isSet("indirect")) {
	boWarning() << k_funcinfo << "use indirect rendering (slow!)" << endl;
	boConfig->setBoolValue("ForceWantDirect", false);
 }

 if (args->isSet("fast")) {
	const BosonConfigScript* script = boConfig->configScript("FastStartup");
	if (!script) {
		BO_NULL_ERROR(script);
	} else {
		script->execute(boConfig);
	}
 }
 if (args->isSet("veryfast")) {
	const BosonConfigScript* script = boConfig->configScript("VeryFastStartup");
	if (!script) {
		BO_NULL_ERROR(script);
	} else {
		script->execute(boConfig);
	}
 }
}

