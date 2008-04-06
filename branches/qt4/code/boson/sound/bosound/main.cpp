/*
    This file is part of the Boson game
    Copyright (C) 2003-2004 Andreas Beckermann (b_mann@gmx.de)

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

#include <qfile.h>
#include <qsocketnotifier.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "boaudiothread.h"
#include "boaudiocommand.h"
#include "../boson/boversion.h"

#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <krandom.h>


static const char* description = "Boson Audio Process";

static KCmdLineOptions options[] =
{
	{ "commands", I18N_NOOP("list valid commands"), 0},
	{ 0, 0, 0 }
};

static void listCommands();

int main(int argc, char **argv)
{
 // we have to use KApplication, as we use KRandom::random() in BosonSound!
 KAboutData about("bosonaudioprocess",
		"Boson Audio Process",
		BOSON_VERSION_STRING,
		description,
		KAboutData::License_GPL,
		"(C) 2003-2004 The Boson team",
		0,
		"http://boson.eu.org");
 about.addAuthor("Andreas Beckermann", I18N_NOOP("Coding"), "b_mann@gmx.de");
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 KApplication app(false, false);

 KGlobal::dirs()->addPrefix(BOSON_PREFIX);

 KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
 if (args->isSet("commands")) {
	listCommands();
	return 0;
 }


// this is the actual sound part:
 BoAudioThread* t = new BoAudioThread();

 // and now the communication with the other process.
 QFile readFile;
 readFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered, stdin);
 // we may want to write to stdout to let boson know about errors!
 QFile writeFile;
 writeFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered, stdout);

 QSocketNotifier notifier(readFile.handle(), QSocketNotifier::Read);
 QObject::connect(&notifier, SIGNAL(activated(int)), t, SLOT(slotReceiveStdin(int)));

 int ret = app.exec();

 readFile.close();
 writeFile.close();
 delete t;

 return ret;
}

static void listCommands()
{
 printf("Valid commands:\n");
 printf("(usual format: <type> <is_sound> [<species>] <data_int> <data_string> <data_string2>)\n");
 printf("(note: <data_int> is 0 when unused. the <data_string>s can be empty, but the spaces between them are mandatory!)\n");
 printf("The quotation marks \"\" do not belong to the command. they emphasize the number of spaces only.\n");
 printf("\n");

 printf("General:\n");
 printf("\"%d 0 0  \" - Create the music object\n", BoAudioCommand::CreateMusicObject);
 printf("\"%d 1 <species> 0  \" - Create a sound object for the species\n", BoAudioCommand::CreateSoundObject);
 printf("\"%d 0 0  \" - Disable music\n", BoAudioCommand::EnableMusic);
 printf("\"%d 0 1  \" - Enable music\n", BoAudioCommand::EnableMusic);
 printf("\"%d 0 0  \" - Disable sound\n", BoAudioCommand::EnableSound);
 printf("\"%d 0 1  \" - Enable sound\n", BoAudioCommand::EnableSound);
 printf("\2\n");

 printf("Music:\n");
 printf("\"%d 0 0  \"       - Start playing music\n", BoAudioCommand::PlayMusic);
 printf("\"%d 0 0  \"       - Stop playing music\n", BoAudioCommand::StopMusic);
 printf("\"%d 0 0  \"       - Clear music list (playlist)\n", BoAudioCommand::ClearMusicList);
 printf("\"%d 0 0 <file> \" - Add file to music list\n", BoAudioCommand::AddToMusicList);
 printf("\"%d 0 0  \"       - Start looping list\n", BoAudioCommand::StartMusicLoop);
 printf("\n");

 printf("Sound:\n");
 printf("\"%d 1 <species> 0 <name> \" - Play the (unit) sound\n", BoAudioCommand::PlaySound);
 printf("\"%d 1 <species> <id>  \" - Play the (general) sound\n", BoAudioCommand::PlaySound);
 printf("\"%d 1 <species> 0 <name> <file>\" - Add a unit sound\n", BoAudioCommand::AddUnitSound);
 printf("\"%d 1 <species> <id_number> <file> \" - Add a general sound\n", BoAudioCommand::AddGeneralSound);
 printf("\n");
}

