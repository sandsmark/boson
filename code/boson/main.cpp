/*
    This file is part of the Boson game
    Copyright (C) 2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "top.h"

#include "bosonconfig.h"
#include "boglobal.h"
#include "boapplication.h"
#include "boversion.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include "boeventloop.h"
#include <config.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmessagebox.h>

// sound is enabled by default atm
#define HARDCODE_NOSOUND 0

static const char *description =
    I18N_NOOP("A realtime strategy game for KDE");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
#if HARDCODE_NOSOUND
    { "sound", I18N_NOOP("Enable Sounds"), 0 },
#else
    { "nosound", I18N_NOOP("Disable Sounds"), 0 },
#endif
    { "new", I18N_NOOP("Skip Welcome Widget and display the New Game screen"), 0 },
    { "editor", I18N_NOOP("Skip Welcome Widget and display the Start Editor screen"), 0 },
    { "load", I18N_NOOP("Skip Welcome Widget and display the Load Game screen"), 0 },
    { "load-from-log <file>", I18N_NOOP("Load from emergency log, for debugging"), 0 },
    { "playfield <identifier>", I18N_NOOP("Playfield identifier for newgame/start editor widget"), 0 },
    { "computer <count>", I18N_NOOP("Add (currently dummy) computer player"), 0 },
    { "start", I18N_NOOP("Start the game"), 0},
    { "aidelay <delay>", I18N_NOOP("Set AI delay (in seconds). The less it is, the faster AI will send it's units"), 0 },
    { "noai", I18N_NOOP("Disable AI"), 0 },
    { "indirect", I18N_NOOP("Use Indirect rendering (sloooow!! - debugging only)"), 0 },
    { "ati-depth-workaround", I18N_NOOP("Enable the ATI (proprietary) driver workaround for reading the depth buffer. Will use depth of 0.00390625"), 0 },
    { "ati-depth-workaround-depth <depth>", I18N_NOOP("Use with --ati-depth-workaround. Supply a depth value for your system (default=0.00390625)"), 0 },
    { "default-lodcount <count>", I18N_NOOP("Use <count> for default level of detail count"), 0 },
    { "nomodels", I18N_NOOP("Disable model loading for faster startup (you won't see the units)"), 0 },
    { 0, 0, 0 }
};


void postBosonConfigInit();


int main(int argc, char **argv)
{
 KAboutData about("boson",
        I18N_NOOP("Boson"),
        version,
        description,
        KAboutData::License_GPL,
        "(C) 1999-2000,2001-2004 The Boson team",
        0,
        "http://boson.eu.org");
 about.addAuthor("Thomas Capricelli", I18N_NOOP("Initial Game Design & Coding"),
        "orzel@freehackers.org", "http://orzel.freehackers.org");
 about.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"),
        "benadler@bigfoot.de");
 about.addAuthor("Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de");
 about.addAuthor("Rivo Laks", I18N_NOOP("Coding & Homepage Redesign"), "rivolaks@hot.ee");
 about.addAuthor("Felix Seeger", I18N_NOOP("Documentation"), "felix.seeger@gmx.de");

 // first tell BoGlobal that we need to do extra stuff after BosonConfig's
 // initialization
 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
#ifdef BOSON_COMPILE_STATIC
 KApplication::disableAutoDcopRegistration();
#endif

 BoEventLoop eventLoop(0, "main event loop");
 BoApplication app;
 KGlobal::locale()->insertCatalogue("libkdegames");

    // register ourselves as a dcop client
//    app.dcopClient()->registerAs(app.name(), false);

 // make sure the data files are installed at the correct location
 QString errorMessage = TopWidget::checkInstallation();
 if (!errorMessage.isNull()) {
    boError() << k_funcinfo << errorMessage << endl;
    boError() << k_funcinfo << "check your installation!" << endl;
    KMessageBox::sorry(0, errorMessage, i18n("Check your installation"));
    return 1;
 }

 TopWidget *top = new TopWidget;
 app.setMainWidget(top);

 top->show();

 // pretend an old game was over. here we actually start
 top->slotGameOver();

 if (boConfig->boolValue("EnableATIDepthWorkaround", false)) {
    double depth = boConfig->doubleValue("ATIDepthWorkaroundValue", 0.00390625);
    Bo3dTools::enableReadDepthBufferWorkaround((float)depth);
 }

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
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
 if (args->isSet("models")) {
    boConfig->setBoolValue("ForceDisableModelLoading", false);
 } else {
    boWarning() << "model loading disabled - you will not see any units!" << endl;
    boConfig->setBoolValue("ForceDisableModelLoading", true);
 }
 if (args->isSet("new")) {
    top->slotNewGame(args);
 } else if (args->isSet("editor")) {
    top->slotStartEditor(args);
 } else if (args->isSet("load")) {
    top->slotLoadGame(args);
 } else if (args->isSet("load-from-log")) {
    QString file = args->getOption("load-from-log");
    top->slotLoadFromLog(file);
 }
 args->clear();
 return app.exec();
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
    boConfig->setDoubleValue("AiDelay", 0.0);
 } else if (args->isSet("aidelay")) {
    QString delay = args->getOption("aidelay");
    bool ok;
    boConfig->setDoubleValue("AiDelay", delay.toFloat(&ok));
    boDebug() << k_funcinfo << "aidelay set to " << boConfig->doubleValue("AIDelay") << endl;
    if (!ok) {
        boError() << k_funcinfo << "aidelay is not a valid float!" << endl;
        // Fall back to default
        boConfig->setDoubleValue("AiDelay", 3.0);
    }
 }
 if (args->isSet("indirect")) {
    boWarning() << k_funcinfo << "use indirect rendering (slow!)" << endl;
    boConfig->setBoolValue("ForceWantDirect", false);
 }
}

